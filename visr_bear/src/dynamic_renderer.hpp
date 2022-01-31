#pragma once
#include "bear/api.hpp"

namespace bear {

class DynamicRendererImpl;

/// renderer which supports dynamic configuration changes
///
/// After construction, all methods can be used, but process will output
/// silence until set_config has been called and the renderer setup has
/// finished (or set_config_blocking has been called successfully).
///
/// This is not currently thread-safe, in that methods must not be called
/// concurrently.
class DynamicRenderer {
 public:
  /// period_size: number of samples in each block
  /// max_size: maximum expected channels in each type. This is a soft limit --
  ///     if you go over this then there will be some reallocation. There's not
  ///     much data per channel so set this high.
  DynamicRenderer(size_t period_size, size_t max_size);

  DynamicRenderer(DynamicRenderer &&r);
  DynamicRenderer();
  ~DynamicRenderer();

  DynamicRenderer &operator=(DynamicRenderer &&r);

  /// Set the configuration; this does not block, configuration changes happen on another thread.
  ///
  /// It may take some time for this change to be visible in the output of
  /// process, but other methods behave as if this change has already happened
  /// (so metadata can be pushed straight away).
  ///
  /// After calling this, either is_running() will become true after some time
  /// (with process calls), or get_error will return non-null once.
  void set_config(const Config &config);

  /// Set the configuration; this blocks, and throws exceptions if something
  /// goes wrong
  ///
  /// This bypasses fading; the assumption is that this will be used before
  /// starting the renderer (either the first time, or after pausing
  /// processing).
  void set_config_blocking(const Config &config);

  /// get and clear an error from constructing the renderer (started by set_config)
  std::exception_ptr get_error();
  /// is the renderer running; returns true immediately after
  /// set_config_blocking, or some time after set_config, providing there is no
  /// error (then get_error will return non-null once)
  bool is_running();

  /// Process one block of samples.
  /// num_*_channels refers to the number of channels in the corresponding
  /// *_input buffer. Channels beyond those that are configured are ignored,
  /// and missing channels are filled with zeros.
  void process(size_t num_objects_channels,
               const Sample *const *objects_input,
               size_t num_direct_speakers_channels,
               const Sample *const *direct_speakers_input,
               size_t num_hoa_channels,
               const Sample *const *hoa_input,
               Sample *const *output);

  // below methods are identical to the Renderer API

  bool add_objects_block(size_t channel, ObjectsInput metadata);
  bool add_direct_speakers_block(size_t channel, DirectSpeakersInput metadata);
  bool add_hoa_block(size_t stream, HOAInput metadata);
  Time get_block_start_time() const;
  void set_block_start_time(const Time &time);
  Time get_delay() const;
  void set_listener(const Listener &l, const boost::optional<Time> &interpolation_time = {});

 private:
  std::unique_ptr<DynamicRendererImpl> impl;
};

}  // namespace bear
