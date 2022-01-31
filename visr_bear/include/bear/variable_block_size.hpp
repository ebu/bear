#pragma once
#include <map>
#include <queue>
#include <vector>

#include "api.hpp"
#include "ear/dsp/variable_block_size.hpp"

namespace bear {

class VariableBlockSizeAdapterImpl;

/// Adapter to allow using the renderer with variable block sizes.
///
/// This has an internal queue for each type to avoid metadata underruns. Note
/// that add_* should be called until they return false before process() with
/// the same value for numsamples to avoid metadata underruns. Seeking is
/// currently unsupported.
class VariableBlockSizeAdapter {
 public:
  VariableBlockSizeAdapter(const Config &config_, std::shared_ptr<Renderer> renderer_);
  ~VariableBlockSizeAdapter();

  void process(size_t nsamples,
               const Sample *const *objects_input,
               const Sample *const *direct_speakers_input,
               const Sample *const *hoa_input,
               Sample *const *output);

  bool add_objects_block(size_t nsamples, size_t channel, ObjectsInput metadata);

  bool add_direct_speakers_block(size_t nsamples, size_t channel, DirectSpeakersInput metadata);

  bool add_hoa_block(size_t nsamples, size_t stream, HOAInput metadata);

 private:
  std::unique_ptr<VariableBlockSizeAdapterImpl> impl;
};
};  // namespace bear
