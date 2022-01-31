#include "runner.hpp"

#include "libaudiointerfaces/audio_interface_factory.hpp"

namespace bear {
namespace demo {

  Runner::Runner(const std::string &interface_name,
                 const std::string &audio_config_str,
                 const bear::Config &bear_config_)
      : bear_config(bear_config_), listener_queue(10), objects_queue(10), direct_speakers_queue(10)
  {
    visr::audiointerfaces::AudioInterface::Configuration audio_config(
        bear_config.get_num_objects_channels() + bear_config.get_num_direct_speakers_channels() +
            bear_config.get_num_hoa_channels(),
        2,
        bear_config.get_sample_rate(),
        bear_config.get_period_size());

    audio_interface =
        visr::audiointerfaces::AudioInterfaceFactory::create(interface_name, audio_config, audio_config_str);

    renderer = Renderer(bear_config);

    audio_interface->registerCallback(&Runner::audio_callback_static, this);
  }

  void Runner::start() { audio_interface->start(); }
  void Runner::stop() { audio_interface->stop(); }

  void Runner::audio_callback_static(void *self,
                                     const float *const *capture_samples,
                                     float *const *playback_samples,
                                     bool &error)
  {
    Runner *runner = (Runner *)self;
    error = runner->audio_callback(capture_samples, playback_samples);
  }

  bool Runner::add_objects_block(size_t channel, ObjectsInput metadata)
  {
    std::pair<size_t, ObjectsInput> oi(channel, std::move(metadata));
    return objects_queue.push(oi);
  }

  bool Runner::add_direct_speakers_block(size_t channel, DirectSpeakersInput metadata)
  {
    std::pair<size_t, DirectSpeakersInput> dsi(channel, std::move(metadata));
    return direct_speakers_queue.push(dsi);
  }

  void Runner::set_listener(const Listener &l, const boost::optional<Time> &interpolation_time)
  {
    std::pair<Listener, boost::optional<Time>> listener_p(l, interpolation_time);
    listener_queue.push(listener_p);
  }

  bool Runner::audio_callback(const float *const *capture_samples, float *const *playback_samples)
  {
    // TODO: this ignores return value of add_objects_block; it should be saved
    // and re-tried in the next round if it returns false
    std::pair<size_t, ObjectsInput> oi;
    while (objects_queue.pop(oi)) {
      renderer.add_objects_block(oi.first, oi.second);
    }

    std::pair<size_t, DirectSpeakersInput> dsi;
    while (direct_speakers_queue.pop(dsi)) {
      renderer.add_direct_speakers_block(dsi.first, dsi.second);
    }

    std::pair<Listener, boost::optional<Time>> listener_p;
    while (listener_queue.pop(listener_p)) {
      renderer.set_listener(listener_p.first, listener_p.second);
    }

    renderer.process(capture_samples,
                     capture_samples + bear_config.get_num_objects_channels(),
                     capture_samples + (bear_config.get_num_objects_channels() +
                                        bear_config.get_num_direct_speakers_channels()),
                     playback_samples);
    return false;
  }
}  // namespace demo
}  // namespace bear
