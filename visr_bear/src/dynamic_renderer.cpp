#include "dynamic_renderer.hpp"

#include <boost/container/small_vector.hpp>
#include <libefl/basic_vector.hpp>
#include <libefl/initialise_library.hpp>
#include <libefl/vector_functions.hpp>
#include <vector>

#include "bear/api.hpp"
#include "config_impl.hpp"
#include "constructor_thread.hpp"
#include "utils.hpp"

namespace bear {

template <typename T, int N>
class MetadataBuffer {
 public:
  MetadataBuffer(size_t max_size) { data.reserve(max_size); }

  void push(size_t channel, const T &channel_data)
  {
    if (channel >= data.size()) data.resize(channel + 1);

    if (data[channel].size() == N) data[channel].erase(data[channel].begin());

    data[channel].push_back(channel_data);
  }

  std::vector<boost::container::small_vector<T, N>> data;
};

/// controller for DynamicRendererImpl, implementing fade up, fade down, waiting and preroll logic
class StateMachine {
 public:
  StateMachine(size_t period_size)
      : num_preroll_frames(static_cast<int>(std::ceil((double)num_preroll_samples / period_size)))
  {
  }

  // state transitions

  // call when reconfiguration is started
  void start_config()
  {
    if (state == State::RUNNING || state == State::FADE_DOWN)
      set_state(State::FADE_DOWN);
    else
      set_state(State::WAIT);
  }

  // call when reconfiguration is finished (after start_config) and we have a
  // renderer again
  void config_finished()
  {
    bear_assert(state != State::START, "unexpected state");
    bear_assert(state != State::RUNNING, "unexpected state");

    if (state == State::WAIT) {
      set_state(State::PREROLL);
      preroll_frames_left = num_preroll_frames;
    }
  }

  void set_config_blocking() { set_state(State::RUNNING); }

  // call at the end of each process call
  void process_done()
  {
    if (state == State::FADE_DOWN)
      set_state(State::WAIT);
    else if (state == State::PREROLL) {
      preroll_frames_left--;
      if (preroll_frames_left == 0) set_state(State::FADE_UP);
    } else if (state == State::FADE_UP)
      set_state(State::RUNNING);
  }

  // externally-visible state

  // should the output be faded down?
  bool should_fade_down() { return state == State::FADE_DOWN; }

  // should the output be faded up?
  bool should_fade_up() { return state == State::FADE_UP; }

  // should the output be cleared?
  bool should_clear_output()
  {
    // preroll, or when !should_render
    return state == State::START || state == State::WAIT || state == State::PREROLL;
  }

  // should the real renderer process function be called?
  bool should_render()
  {
    return state == State::RUNNING || state == State::FADE_DOWN || state == State::PREROLL ||
           state == State::FADE_UP;
  }

  // should the renderer be swapped with the configuration result?
  bool should_swap() { return state == State::START || state == State::WAIT; }

  bool is_running() { return state == State::RUNNING; }

 private:
  // generally, refers to what should be done in the next frame
  enum class State { START, FADE_DOWN, WAIT, PREROLL, FADE_UP, RUNNING };

  State state = State::START;
  int preroll_frames_left = 0;

  void set_state(State s)
  {
    state = s;
    // const char *names[] = {"START", "FADE_DOWN", "WAIT", "PREROLL", "FADE_UP", "RUNNING"};
    // std::cout << "state: " << names[(int)state] << "\n";
  }

  static constexpr int num_preroll_samples = 2976   // brir length
                                             + 128  // decorrelation filters
                                             + 38   // max delays
                                             + 3;   // delay implementation
  const int num_preroll_frames;
};

class TypeAdapter {};

struct ObjectsTypeAdapter {
  static bool input_fits(const ConfigImpl &config, size_t channel, const ObjectsInput &)
  {
    return channel < config.num_objects_channels;
  }

  static bool add_block(Renderer &renderer, size_t channel, ObjectsInput metadata)
  {
    return renderer.add_objects_block(channel, std::move(metadata));
  }

  using InputType = ObjectsInput;
};

struct DirectSpeakersTypeAdapter {
  static bool input_fits(const ConfigImpl &config, size_t channel, const DirectSpeakersInput &)
  {
    return channel < config.num_direct_speakers_channels;
  }

  static bool add_block(Renderer &renderer, size_t channel, DirectSpeakersInput metadata)
  {
    return renderer.add_direct_speakers_block(channel, std::move(metadata));
  }

  using InputType = DirectSpeakersInput;
};

struct HOATypeAdapter {
  static bool input_fits(const ConfigImpl &config, size_t stream, const HOAInput &metadata)
  {
    (void)stream;

    for (size_t channel : metadata.channels)
      if (channel >= config.num_hoa_channels) return false;
    return true;
  }

  static bool add_block(Renderer &renderer, size_t channel, HOAInput metadata)
  {
    return renderer.add_hoa_block(channel, std::move(metadata));
  }

  using InputType = HOAInput;
};

class DynamicRendererImpl {
 public:
  DynamicRendererImpl(size_t _block_size, size_t max_size)
      : block_size(_block_size),
        objects_buffer(max_size),
        direct_speakers_buffer(max_size),
        hoa_buffer(max_size),
        state(block_size),
        ramp_up(block_size, 0),
        ramp_down(block_size, 0),
        zeros(block_size, 0)
  {
    visr::efl::vectorRamp<Sample>(ramp_up.data(), block_size, 0.0, 1.0, true, false);
    visr::efl::vectorRamp<Sample>(ramp_down.data(), block_size, 1.0, 0.0, true, false);
    visr::efl::vectorZero(zeros.data(), block_size);

    next_render_config.num_objects_channels = 0;
    next_render_config.num_direct_speakers_channels = 0;
    next_render_config.num_hoa_channels = 0;
  }

  void set_config_common(const Config &config)
  {
    sample_rate = config.get_sample_rate();
    bear_assert(config.get_period_size() == block_size, "config has incorrect period size");

    next_render_config = config.get_impl();
  }

  void set_config(const Config &config)
  {
    set_config_common(config);

    if (!constructor_thread.start(config)) next_config = config.get_impl();

    state.start_config();
  }

  void set_config_blocking(const Config &config)
  {
    renderer = Renderer(config);

    set_config_common(config);
    current_render_config = next_render_config;
    initialise_renderer();
    state.set_config_blocking();
  }

  std::exception_ptr get_error() { return constructor_thread.get_construction_error(); }
  bool is_running() { return state.is_running(); }

  bool add_objects_block(size_t channel, ObjectsInput metadata)
  {
    return add_block<ObjectsTypeAdapter>(objects_buffer, channel, metadata);
  }

  bool add_direct_speakers_block(size_t channel, DirectSpeakersInput metadata)
  {
    return add_block<DirectSpeakersTypeAdapter>(direct_speakers_buffer, channel, metadata);
  }

  bool add_hoa_block(size_t channel, HOAInput metadata)
  {
    return add_block<HOATypeAdapter>(hoa_buffer, channel, metadata);
  }

  void process(size_t num_objects_channels,
               const Sample *const *objects_input,
               size_t num_direct_speakers_channels,
               const Sample *const *direct_speakers_input,
               size_t num_hoa_channels,
               const Sample *const *hoa_input,
               Sample *const *output)
  {
    if (next_config) {
      constructor_thread.get_result();
      if (constructor_thread.start(*next_config)) next_config = boost::none;
    }

    maybe_swap();

    if (state.should_render()) {
      bear_assert(renderer.has_value(), "expected renderer to be set");

      fill_temp_pointers(
          current_render_config.num_objects_channels, temp_objects, num_objects_channels, objects_input);
      fill_temp_pointers(current_render_config.num_direct_speakers_channels,
                         temp_direct_speakers,
                         num_direct_speakers_channels,
                         direct_speakers_input);
      fill_temp_pointers(current_render_config.num_hoa_channels, temp_hoa, num_hoa_channels, hoa_input);
      renderer->process(temp_objects.data(), temp_direct_speakers.data(), temp_hoa.data(), output);
    }

    if (state.should_fade_down())
      for (size_t i = 0; i < 2; i++)
        visr::efl::vectorMultiplyInplace(ramp_down.data(), output[i], block_size);
    if (state.should_fade_up())
      for (size_t i = 0; i < 2; i++) visr::efl::vectorMultiplyInplace(ramp_up.data(), output[i], block_size);
    if (state.should_clear_output())
      for (size_t i = 0; i < 2; i++) visr::efl::vectorZero(output[i], block_size);

    state.process_done();
    num_blocks_processed++;
  }

  void set_block_start_time(const Time &time)
  {
    // get_raw_time() - time_offset == time
    time_offset = get_raw_block_start_time() - time;
  }

  Time get_block_start_time() const { return get_raw_block_start_time() - time_offset; }

  void set_listener(const Listener &l, const boost::optional<Time> &interpolation_time)
  {
    if (renderer) renderer->set_listener(l, interpolation_time);

    last_listener = l;
    last_listener_interpolation_time = interpolation_time;
  }

 private:
  template <typename Adapter, typename Buffer>
  bool add_block(Buffer &buffer, size_t channel, typename Adapter::InputType metadata)
  {
    if (!Adapter::input_fits(next_render_config, channel, metadata))
      throw std::invalid_argument("not enough channels configured to add block");

    if (metadata.rtime) *metadata.rtime += time_offset;

    if (!metadata.rtime || *metadata.rtime < get_raw_next_block_start_time()) {
      // always buffer so that when the renderer reconfiguration is finished we
      // can feed it metadata for all channels
      buffer.push(channel, metadata);
      // only push metadata to the renderer when we will actually be calling
      // the process function, otherwise our notion of time and the renderer's
      // may get out of sync (plus it's wasted effort)
      if (state.should_render() && Adapter::input_fits(current_render_config, channel, metadata)) {
        bear_assert(renderer.has_value(), "expected renderer to be set");
        bear_assert(Adapter::add_block(*renderer, channel, metadata), "could not push");
      }
      return true;
    } else
      return false;
  }

  void maybe_swap()
  {
    if (state.should_swap()) {
      if (constructor_thread.try_swap_result(renderer)) {
        current_render_config = next_render_config;

        initialise_renderer();

        state.config_finished();
      }
    }
  }

  /// initialise the newly-created renderer: set up time, listener, and push
  /// metadata from queues
  void initialise_renderer()
  {
    renderer->set_block_start_time({block_size * num_blocks_processed, sample_rate});
    renderer->set_listener(last_listener, last_listener_interpolation_time);

    push_data_from_buffer<ObjectsTypeAdapter>(objects_buffer);
    push_data_from_buffer<DirectSpeakersTypeAdapter>(direct_speakers_buffer);
    push_data_from_buffer<HOATypeAdapter>(hoa_buffer);
  }

  template <typename Adapter, typename Buffer>
  void push_data_from_buffer(const Buffer &buffer)
  {
    for (size_t i = 0; i < buffer.data.size(); i++)
      for (size_t j = 0; j < buffer.data[i].size(); j++) {
        const auto &metadata = buffer.data[i][j];
        if (Adapter::input_fits(current_render_config, i, metadata))
          bear_assert(Adapter::add_block(*renderer, i, metadata), "could not push");
      }
  }

  void fill_temp_pointers(size_t num_out,
                          std::vector<const Sample *> &out,
                          size_t num_in,
                          const Sample *const *in)
  {
    out.resize(num_out);
    for (size_t i = 0; i < num_out; i++) {
      out[i] = (i < num_in) ? in[i] : zeros.data();
    }
  }

  Time get_raw_block_start_time() const { return {num_blocks_processed * block_size, sample_rate}; }

  Time get_raw_next_block_start_time() const
  {
    return {(num_blocks_processed + 1) * block_size, sample_rate};
  }

  size_t block_size;
  size_t num_blocks_processed = 0;
  size_t sample_rate = 0;

  /// metadata buffers for each input type; these are used when starting up a
  /// new renderer
  MetadataBuffer<ObjectsInput, 2> objects_buffer;
  MetadataBuffer<DirectSpeakersInput, 1> direct_speakers_buffer;
  MetadataBuffer<HOAInput, 1> hoa_buffer;

  boost::optional<Renderer> renderer;

  /// the next configuration to pass to the constructor thread, set if the
  /// thread is busy when set_config is called
  boost::optional<ConfigImpl> next_config;

  /// the configuration of renderer, if it exists
  ConfigImpl current_render_config;
  /// the externally-visible configuration, set by set_config, and starting
  /// with zero input channels
  ConfigImpl next_render_config;

  ConstructorThread constructor_thread;

  StateMachine state;

  visr::efl::BasicVector<Sample> ramp_up;
  visr::efl::BasicVector<Sample> ramp_down;
  visr::efl::BasicVector<Sample> zeros;

  std::vector<const Sample *> temp_objects;
  std::vector<const Sample *> temp_direct_speakers;
  std::vector<const Sample *> temp_hoa;

  Time time_offset;

  // non-optional is fine, as the renderer start condition is the same as the
  // default-constructed listener
  Listener last_listener;
  boost::optional<Time> last_listener_interpolation_time;
};

DynamicRenderer::DynamicRenderer() {}

DynamicRenderer::DynamicRenderer(size_t block_size, size_t max_size)
{
  visr::efl::initialiseLibrary();

  impl = std::make_unique<DynamicRendererImpl>(block_size, max_size);
}

DynamicRenderer::DynamicRenderer(DynamicRenderer &&r) = default;

DynamicRenderer &DynamicRenderer::operator=(DynamicRenderer &&r) = default;

void DynamicRenderer::set_config(const Config &config) { impl->set_config(config); }

void DynamicRenderer::set_config_blocking(const Config &config) { impl->set_config_blocking(config); }

std::exception_ptr DynamicRenderer::get_error() { return impl->get_error(); }
bool DynamicRenderer::is_running() { return impl->is_running(); }

void DynamicRenderer::process(size_t num_objects_channels,
                              const Sample *const *objects_input,
                              size_t num_direct_speakers_channels,
                              const Sample *const *direct_speakers_input,
                              size_t num_hoa_channels,
                              const Sample *const *hoa_input,
                              Sample *const *output)
{
  impl->process(num_objects_channels,
                objects_input,
                num_direct_speakers_channels,
                direct_speakers_input,
                num_hoa_channels,
                hoa_input,
                output);
}

bool DynamicRenderer::add_objects_block(size_t channel, ObjectsInput metadata)
{
  return impl->add_objects_block(channel, std::move(metadata));
}

bool DynamicRenderer::add_direct_speakers_block(size_t channel, DirectSpeakersInput metadata)
{
  return impl->add_direct_speakers_block(channel, std::move(metadata));
}

bool DynamicRenderer::add_hoa_block(size_t stream, HOAInput metadata)
{
  return impl->add_hoa_block(stream, std::move(metadata));
}

Time DynamicRenderer::get_block_start_time() const { return impl->get_block_start_time(); }

void DynamicRenderer::set_block_start_time(const Time &time) { impl->set_block_start_time(time); }

void DynamicRenderer::set_listener(const Listener &l, const boost::optional<Time> &interpolation_time)
{
  impl->set_listener(l, interpolation_time);
}

DynamicRenderer::~DynamicRenderer() = default;

}  // namespace bear
