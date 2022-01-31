#pragma once
#include <boost/lockfree/spsc_queue.hpp>

#include "bear/api.hpp"
#include "libaudiointerfaces/audio_interface.hpp"

namespace bear {
namespace demo {
  class Runner {
   public:
    Runner(const std::string &interface_name,
           const std::string &audio_config_str,
           const bear::Config &bear_config);

    void start();
    void stop();

    void set_listener(const Listener &l, const boost::optional<Time> &interpolation_time = {});
    bool add_objects_block(size_t channel, ObjectsInput metadata);
    bool add_direct_speakers_block(size_t channel, DirectSpeakersInput metadata);

   private:
    static void audio_callback_static(void *self,
                                      const float *const *capture_samples,
                                      float *const *playback_samples,
                                      bool &error);
    bool audio_callback(const float *const *capture_samples, float *const *playback_samples);

    bear::Config bear_config;
    std::unique_ptr<visr::audiointerfaces::AudioInterface> audio_interface;
    bear::Renderer renderer;

    boost::lockfree::spsc_queue<std::pair<Listener, boost::optional<Time>>> listener_queue;
    boost::lockfree::spsc_queue<std::pair<size_t, ObjectsInput>> objects_queue;
    boost::lockfree::spsc_queue<std::pair<size_t, DirectSpeakersInput>> direct_speakers_queue;
  };
}  // namespace demo
}  // namespace bear
