#include "bear/api.hpp"

#include <libefl/denormalised_number_handling.hpp>
#include <libefl/initialise_library.hpp>
#include <libpml/initialise_parameter_library.hpp>
#include <librrl/audio_signal_flow.hpp>
#include <libvisr/signal_flow_context.hpp>
#include <libvisr/time.hpp>

#include "config_impl.hpp"
#include "listener_impl.hpp"
#include "parameters.hpp"
#include "top.hpp"
#include "utils.hpp"

using namespace visr;

namespace {
template <typename T, size_t N>
std::array<T, N> to_array(const Eigen::Ref<const Eigen::Matrix<T, N, 1>> &m)
{
  std::array<T, N> out;
  Eigen::Matrix<double, N, 1>::Map(out.data()) = m;
  return out;
}
}  // namespace

namespace bear {

Config::Config() : impl(std::make_unique<ConfigImpl>()) {}
Config::Config(const Config &c) : impl(std::make_unique<ConfigImpl>(*c.impl)) {}
Config::~Config() = default;

Config &Config::operator=(const Config &c)
{
  *impl = *c.impl;
  return *this;
}

void Config::set_num_objects_channels(size_t num_channels) { impl->num_objects_channels = num_channels; }
size_t Config::get_num_objects_channels() const { return impl->num_objects_channels; }

void Config::set_num_direct_speakers_channels(size_t num_channels)
{
  impl->num_direct_speakers_channels = num_channels;
}
size_t Config::get_num_direct_speakers_channels() const { return impl->num_direct_speakers_channels; }

void Config::set_num_hoa_channels(size_t num_channels) { impl->num_hoa_channels = num_channels; }
size_t Config::get_num_hoa_channels() const { return impl->num_hoa_channels; }

void Config::set_period_size(size_t num_samples) { impl->period_size = num_samples; }
size_t Config::get_period_size() const { return impl->period_size; }

void Config::set_sample_rate(size_t sample_rate) { impl->sample_rate = sample_rate; }
size_t Config::get_sample_rate() const { return impl->sample_rate; }

void Config::set_data_path(const std::string &path) { impl->data_path = path; }
const std::string &Config::get_data_path() const { return impl->data_path; }

void Config::set_fft_implementation(const std::string &fft_implementation)
{
  impl->fft_implementation = fft_implementation;
}
const std::string &Config::get_fft_implementation() const { return impl->fft_implementation; }

void Config::validate() const
{
  if (impl->period_size == 0) throw std::invalid_argument("Config: period size must be set");
  if (impl->data_path.empty()) throw std::invalid_argument("Config: data path must be set");
}

ConfigImpl &Config::get_impl() { return *impl; }
const ConfigImpl &Config::get_impl() const { return *impl; }

Listener::Listener() : impl(std::make_unique<ListenerImpl>()) {}
Listener::~Listener() = default;
Listener::Listener(const Listener &other) : Listener()
{
  impl->position = other.impl->position;
  impl->orientation = other.impl->orientation;
}

void Listener::set_position_cart(std::array<double, 3> position)
{
  impl->position = {position[0], position[1], position[2]};
}

std::array<double, 3> Listener::get_position_cart() const { return to_array<double, 3>(impl->position); }

void Listener::set_orientation_quaternion(std::array<double, 4> orientation)
{
  impl->orientation = {orientation[0], orientation[1], orientation[2], orientation[3]};
}

std::array<double, 4> Listener::get_orientation_quaternion() const
{
  return {impl->orientation.w(), impl->orientation.x(), impl->orientation.y(), impl->orientation.z()};
}

std::array<double, 3> Listener::look() const { return to_array<double, 3>(impl->look()); }
std::array<double, 3> Listener::right() const { return to_array<double, 3>(impl->right()); }
std::array<double, 3> Listener::up() const { return to_array<double, 3>(impl->up()); }

Listener &Listener::operator=(const Listener &other)
{
  if (this != &other) {
    impl->position = other.impl->position;
    impl->orientation = other.impl->orientation;
  }
  return *this;
}

const ListenerImpl &Listener::get_impl() const { return *impl; }

class RendererImpl {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  RendererImpl(ConfigImpl config)
      : config(config),
        ctx(config.period_size, config.sample_rate),
        top(ctx, "top", nullptr, config),
        flow(top),
        objects_metadata_in(dynamic_cast<decltype(objects_metadata_in)>(
            flow.externalParameterReceivePort("objects_metadata_in"))),
        direct_speakers_metadata_in(dynamic_cast<decltype(direct_speakers_metadata_in)>(
            flow.externalParameterReceivePort("direct_speakers_metadata_in"))),
        hoa_metadata_in(
            dynamic_cast<decltype(hoa_metadata_in)>(flow.externalParameterReceivePort("hoa_metadata_in"))),
        listener_in(dynamic_cast<decltype(listener_in)>(flow.externalParameterReceivePort("listener_in"))),
        temp_input_channels(num_input_channels(config))
  {
    // default-initialised listeners in listener_in contain default values
    listener_in.swapBuffers();
  }

  void process(const Sample *const *objects_input,
               const Sample *const *direct_speakers_input,
               const Sample *const *hoa_input,
               Sample *const *output)
  {
    {
      size_t i = 0;
      for (size_t j = 0; j < config.num_objects_channels; j++, i++) temp_input_channels[i] = objects_input[j];
      for (size_t j = 0; j < config.num_direct_speakers_channels; j++, i++)
        temp_input_channels[i] = direct_speakers_input[j];
      for (size_t j = 0; j < config.num_hoa_channels; j++, i++) temp_input_channels[i] = hoa_input[j];
    }

    auto denorm_state = efl::DenormalisedNumbers::setDenormHandling();
    flow.process(temp_input_channels.data(), output);
    efl::DenormalisedNumbers::resetDenormHandling(denorm_state);
  }

  bool add_objects_block(size_t channel, ObjectsInput metadata)
  {
    if (channel >= config.num_objects_channels)
      throw std::invalid_argument("channel number out of range in add_objects_block");

    if (metadata.rtime) *metadata.rtime += time_offset;

    if (!metadata.rtime || metadata.rtime < get_raw_next_block_start_time()) {
      auto parameter = std::make_unique<ADMParameter<ObjectsInput>>(channel, std::move(metadata));
      objects_metadata_in.enqueue(std::move(parameter));
      return true;
    } else
      return false;
  }

  bool add_direct_speakers_block(size_t channel, DirectSpeakersInput metadata)
  {
    if (channel >= config.num_direct_speakers_channels)
      throw std::invalid_argument("channel number out of range in add_direct_speakers_block");

    if (metadata.rtime) *metadata.rtime += time_offset;

    if (!metadata.rtime || metadata.rtime < get_raw_next_block_start_time()) {
      auto parameter = std::make_unique<ADMParameter<DirectSpeakersInput>>(channel, std::move(metadata));
      direct_speakers_metadata_in.enqueue(std::move(parameter));
      return true;
    } else
      return false;
  }

  bool add_hoa_block(size_t stream, HOAInput metadata)
  {
    if (metadata.rtime) *metadata.rtime += time_offset;

    if (!metadata.rtime || metadata.rtime < get_raw_next_block_start_time()) {
      auto parameter = std::make_unique<ADMParameter<HOAInput>>(stream, std::move(metadata));
      hoa_metadata_in.enqueue(std::move(parameter));
      return true;
    } else
      return false;
  }

  Time get_raw_block_start_time() const { return {top.time().sampleCount(), config.sample_rate}; }
  Time get_raw_next_block_start_time() const
  {
    return {top.time().sampleCount() + config.period_size, config.sample_rate};
  }

  Time get_block_start_time() const { return get_raw_block_start_time() - time_offset; }
  Time get_next_block_start_time() const { return get_raw_next_block_start_time() - time_offset; }
  void set_block_start_time(const Time &time)
  {
    // get_raw_time() - time_offset == time
    time_offset = get_raw_block_start_time() - time;
  }

  void set_listener(const Listener &l, const boost::optional<Time> &interpolation_time)
  {
    auto &data = dynamic_cast<ListenerParameter &>(listener_in.data());
    data = l.get_impl();
    listener_in.swapBuffers();
  }

 private:
  ConfigImpl config;
  const SignalFlowContext ctx;
  Top top;
  rrl::AudioSignalFlow flow;
  pml::MessageQueueProtocol::OutputBase &objects_metadata_in;
  pml::MessageQueueProtocol::OutputBase &direct_speakers_metadata_in;
  pml::MessageQueueProtocol::OutputBase &hoa_metadata_in;
  pml::DoubleBufferingProtocol::OutputBase &listener_in;
  std::vector<const Sample *> temp_input_channels;
  Time time_offset;
};

Renderer::Renderer() {}

Renderer::Renderer(const Config &config)
{
  efl::initialiseLibrary();
  pml::initialiseParameterLibrary();
  init_parameters();

  config.validate();

  impl = std::make_unique<RendererImpl>(config.get_impl());
}

Renderer::Renderer(Renderer &&r) = default;

Renderer &Renderer::operator=(Renderer &&r) = default;

void Renderer::process(const Sample *const *objects_input,
                       const Sample *const *direct_speakers_input,
                       const Sample *const *hoa_input,
                       Sample *const *output)
{
  impl->process(objects_input, direct_speakers_input, hoa_input, output);
}

bool Renderer::add_objects_block(size_t channel, ObjectsInput metadata)
{
  return impl->add_objects_block(channel, std::move(metadata));
}

bool Renderer::add_direct_speakers_block(size_t channel, DirectSpeakersInput metadata)
{
  return impl->add_direct_speakers_block(channel, std::move(metadata));
}

bool Renderer::add_hoa_block(size_t stream, HOAInput metadata)
{
  return impl->add_hoa_block(stream, std::move(metadata));
}

Time Renderer::get_block_start_time() const { return impl->get_block_start_time(); }

void Renderer::set_block_start_time(const Time &time) { impl->set_block_start_time(time); }

void Renderer::set_listener(const Listener &l, const boost::optional<Time> &interpolation_time)
{
  impl->set_listener(l, interpolation_time);
}

Renderer::~Renderer() = default;
}  // namespace bear
