#pragma once
#include <boost/rational.hpp>
#include <cstddef>
#include <memory>
#include <string>

#include "ear/metadata.hpp"

namespace bear {

struct ConfigImpl;
struct ListenerImpl;
class RendererImpl;

/// configuration for the renderer
class Config {
 public:
  Config();
  Config(const Config &c);
  ~Config();

  Config &operator=(const Config &c);

  /// set the number of Objects channels (default 0)
  void set_num_objects_channels(size_t num_channels);
  size_t get_num_objects_channels() const;

  /// set the number of DirectSpeakers channels (default 0)
  void set_num_direct_speakers_channels(size_t num_channels);
  size_t get_num_direct_speakers_channels() const;

  /// set the number of HOA channels (default 0)
  void set_num_hoa_channels(size_t num_channels);
  size_t get_num_hoa_channels() const;

  /// set the number of samples in a period (no default, must be set)
  void set_period_size(size_t num_samples);
  size_t get_period_size() const;

  /// set the sample rate (default: 48kHz)
  void set_sample_rate(size_t sample_rate);
  size_t get_sample_rate() const;

  /// set the path to the data file (no default, must be set)
  void set_data_path(const std::string &path);
  const std::string &get_data_path() const;

  /// set the FFT implementation to use (default: a built-in implementation)
  void set_fft_implementation(const std::string &fft_implementation);
  const std::string &get_fft_implementation() const;

  /// check that the configuration is valid; raises exceptions for missing or
  /// incorrect values
  void validate() const;

  ConfigImpl &get_impl();
  const ConfigImpl &get_impl() const;

 private:
  std::unique_ptr<ConfigImpl> impl;
};

using Sample = float;
// time in seconds
using Time = boost::rational<int64_t>;

/// interface for specifying distance behaviour.
class DistanceBehaviour {
 public:
  virtual ~DistanceBehaviour() {}
  /// given the distance to an object and the absoluteDistance parameter, get a
  /// linear gain to be applied
  ///
  /// if absoluteDistance is set, distance is the distance parameter multiplied
  /// by the absoluteDistance in metres, accounting for the listener head
  /// position
  ///
  /// if it's not set, distance is just the distance parameter
  ///
  /// For standard behaviour, this should return 1.0 (or a constant value) if
  /// absoluteDistance is not provided, as BS.2076-2 says:
  ///
  ///   If absoluteDistance is negative or undefined, distance based binaural
  ///   rendering is not intended.
  virtual double get_gain(double distance, boost::optional<double> absoluteDistance) = 0;
};

struct AudioPackFormatData {
  boost::optional<double> absoluteDistance;
};

struct MetadataInput {
  boost::optional<Time> rtime;
  boost::optional<Time> duration;
  AudioPackFormatData audioPackFormat_data;
};

struct ObjectsInput : public MetadataInput {
  ear::ObjectsTypeMetadata type_metadata;
  boost::optional<Time> interpolationLength;
  /// distance behaviour; there is no distance-dependant gain if null
  std::shared_ptr<DistanceBehaviour> distance_behaviour = nullptr;
};

struct DirectSpeakersInput : public MetadataInput {
  ear::DirectSpeakersTypeMetadata type_metadata;
};

struct HOAInput : public MetadataInput {
  ear::HOATypeMetadata type_metadata;
  std::vector<size_t> channels;
};

/// Representation of the listener position and head orientation.
///
/// The ADM Cartesian coordinate system is used, as defined in section 8 of
/// BS.2076-2.
class Listener {
 public:
  Listener();
  ~Listener();
  Listener(const Listener &);

  /// ADM-format Cartesian position of the listener relative to the origin in
  /// metres
  void set_position_cart(std::array<double, 3>);
  std::array<double, 3> get_position_cart() const;

  /// orientation of the listener; this translates from world coordinates to
  /// listener coordinates, so:
  ///     position_rel_listener = orientation * position
  ///     position = orientation' * position_rel_listener
  /// quaternion in order {w, x, y, z}, with the coordinate axes from the ADM
  /// Cartesian system
  void set_orientation_quaternion(std::array<double, 4>);
  std::array<double, 4> get_orientation_quaternion() const;

  Listener &operator=(const Listener &other);

  /// methods intended for testing rotation mappings

  /// the ADM-format Cartesian vector that the listener is looking along
  std::array<double, 3> look() const;
  /// the ADM-format Cartesian vector to the right of the listener
  std::array<double, 3> right() const;
  /// the ADM-format Cartesian vector pointing up from the listener
  std::array<double, 3> up() const;

  const ListenerImpl &get_impl() const;

 private:
  std::unique_ptr<ListenerImpl> impl;
};

class Renderer {
 public:
  Renderer(const Config &config);
  Renderer(Renderer &&r);
  Renderer();
  ~Renderer();

  Renderer &operator=(Renderer &&r);

  /// process period_size samples
  ///
  /// @param objects_input num_objects_channels pointers to period_size samples
  ///     of objects input audio
  /// @param direct_speakers_input num_direct_speakers_channels pointers to
  ///     period_size samples of direct speakers input audio
  /// @param hoa_input num_hoa_channels pointers to period_size samples of HOA
  ///     input audio
  /// @param output 2 pointers to period_size samples to store the output audio
  void process(const Sample *const *objects_input,
               const Sample *const *direct_speakers_input,
               const Sample *const *hoa_input,
               Sample *const *output);

  /// Add some objects metadata to be rendered. This returns true if the
  /// metadata was stored, or false if it was added too early and should be
  /// pushed after processing more samples.
  ///
  /// timing behaviour:
  /// - If metadata contains an rtime equal to the rtime+duration of the
  ///   previous block, there will be interpolation from the previous block
  ///   over the interpolationLength (or the full block if that is not
  ///   specified), followed by constant rendering for the rest of the block.
  /// - If metadata contains an rtime greater than to the rtime+duration of the
  ///   previous block (or it is the first block), the rendering will be
  ///   constant over the length of the block. Between the end of the previous
  ///   block (or the start of the audio processing) and the start of this
  ///   block, there will be silence.
  /// - If the rtime is less than the rtime+duration of the previous block, and
  ///   the rtime is greater than or equal to block_start_time, then the
  ///   samples during the previous block will not be affected, but the
  ///   interpolation during the current block will be smooth. this means that
  ///   this sequence:
  ///
  ///       add(a, 0..100)
  ///       add(b, 100..200)
  ///       add(c, 150..200)
  ///
  ///   results in these interpolation points:
  ///
  ///       time   value
  ///       0      a
  ///       100    a
  ///       150    (a + b) / 2
  ///       200    c
  ///
  ///   If the rtime is less than block_start_time, the result is not
  ///   specified, except that the interpolation at the end of the block uses
  ///   the specified metadata; this can be used to reset the renderer to a
  ///   known state, for example to start rendering half way through a block
  ///   after seeking or when rendering a block which has been changed.
  /// @param channel channel in objects_input that this metadata corresponds to
  bool add_objects_block(size_t channel, ObjectsInput metadata);

  /// Add direct speakers metadata; the semantics are the same as
  /// add_objects_block, except that no interpolation occurs.
  ///
  /// @param channel channel in direct_speakers_input that this metadata
  ///     applies to
  bool add_direct_speakers_block(size_t channel, DirectSpeakersInput metadata);

  /// Add HOA metadata; the semantics are the same as
  /// add_direct_speakers_block, except because HOA metadata refers to multiple
  /// channels, a stream identifier is used rather than a channel number.
  /// @param stream arbitrary value used to identify which HOA stream this
  ///     metadata applies to
  bool add_hoa_block(size_t stream, HOAInput metadata);

  /// start time of the first sample in the next block
  Time get_block_start_time() const;

  /// set the start time of the first sample in the next block
  ///
  /// note that this is purely convenience to apply an offset to the rtimes of
  /// the added blocks at the time they are pushed; the specified timing
  /// behaviour still applies
  void set_block_start_time(const Time &time);

  /// get the delay added by the renderer
  Time get_delay() const;

  /// set the listener position and orientation in the next frame. If
  /// interpolation_time is specified, then the change will possibly be
  /// interpolated over more than one frame; this may be useful if the head
  /// position update rate is slower than the frame rate.
  ///
  /// note that we don't keep a reference to Listener; you should call this
  /// every frame (or so).
  void set_listener(const Listener &l, const boost::optional<Time> &interpolation_time = {});

 private:
  std::unique_ptr<RendererImpl> impl;
};

};  // namespace bear
