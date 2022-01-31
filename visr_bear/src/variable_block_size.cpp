#include "bear/variable_block_size.hpp"

#include "config_impl.hpp"
#include "utils.hpp"

namespace bear {
class VariableBlockSizeAdapterImpl {
 public:
  VariableBlockSizeAdapterImpl(const Config &config_, std::shared_ptr<Renderer> renderer_)
      : config(config_.get_impl()),
        renderer(std::move(renderer_)),
        objects_queue(config.num_objects_channels),
        direct_speakers_queue(config.num_direct_speakers_channels),
        vbs(config.period_size,
            num_input_channels(config),
            2,
            [&](const float *const *in, float *const *out) { process_fixed_size(in, out); }),
        temp_input(num_input_channels(config))
  {
  }

  void process(size_t nsamples,
               const Sample *const *objects_input,
               const Sample *const *direct_speakers_input,
               const Sample *const *hoa_input,
               Sample *const *output)
  {
    size_t j = 0;
    for (size_t i = 0; i < config.num_objects_channels; i++) temp_input[j++] = objects_input[i];
    for (size_t i = 0; i < config.num_direct_speakers_channels; i++)
      temp_input[j++] = direct_speakers_input[i];
    for (size_t i = 0; i < config.num_hoa_channels; i++) temp_input[j++] = hoa_input[i];

    vbs.process(nsamples, temp_input.data(), output);
    num_samples_processed += nsamples;
  }

  bool add_objects_block(size_t nsamples, size_t channel, ObjectsInput metadata)
  {
    if (!metadata.rtime || *metadata.rtime < next_block_start_time(nsamples)) {
      objects_queue.at(channel).push(metadata);
      return true;
    } else
      return false;
  }

  bool add_direct_speakers_block(size_t nsamples, size_t channel, DirectSpeakersInput metadata)
  {
    if (!metadata.rtime || *metadata.rtime < next_block_start_time(nsamples)) {
      direct_speakers_queue.at(channel).push(metadata);
      return true;
    } else
      return false;
  }

  bool add_hoa_block(size_t nsamples, size_t stream, HOAInput metadata)
  {
    if (!metadata.rtime || *metadata.rtime < next_block_start_time(nsamples)) {
      auto it = hoa_queue.find(stream);
      if (it == hoa_queue.end()) {
        bool inserted;
        std::tie(it, inserted) = hoa_queue.insert({stream, {}});
        assert(inserted);
      }
      it->second.push(metadata);
      return true;
    } else
      return false;
  }

 private:
  Time next_block_start_time(size_t nsamples)
  {
    return Time{num_samples_processed + nsamples, config.sample_rate};
  }

  void process_fixed_size(const float *const *in, float *const *out)
  {
    for (size_t i = 0; i < config.num_objects_channels; i++)
      while (objects_queue[i].size() && renderer->add_objects_block(i, objects_queue[i].front()))
        objects_queue[i].pop();

    for (size_t i = 0; i < config.num_direct_speakers_channels; i++)
      while (direct_speakers_queue[i].size() &&
             renderer->add_direct_speakers_block(i, direct_speakers_queue[i].front()))
        direct_speakers_queue[i].pop();

    for (auto &hoa_queue_pair : hoa_queue) {
      size_t stream = hoa_queue_pair.first;
      std::queue<HOAInput> &hoa_queue_i = hoa_queue_pair.second;

      while (hoa_queue_i.size() && renderer->add_hoa_block(stream, hoa_queue_i.front())) hoa_queue_i.pop();
    }

    renderer->process(in,
                      in + config.num_objects_channels,
                      in + config.num_objects_channels + config.num_direct_speakers_channels,
                      out);
  }

  ConfigImpl config;
  std::shared_ptr<Renderer> renderer;
  std::vector<std::queue<ObjectsInput>> objects_queue;
  std::vector<std::queue<DirectSpeakersInput>> direct_speakers_queue;
  std::map<size_t, std::queue<HOAInput>> hoa_queue;
  size_t num_samples_processed = 0;
  ear::dsp::VariableBlockSizeAdapter vbs;
  std::vector<const Sample *> temp_input;
};

VariableBlockSizeAdapter::VariableBlockSizeAdapter(const Config &config, std::shared_ptr<Renderer> renderer)
    : impl(std::make_unique<VariableBlockSizeAdapterImpl>(config, std::move(renderer)))
{
}
VariableBlockSizeAdapter::~VariableBlockSizeAdapter() = default;

void VariableBlockSizeAdapter::process(size_t nsamples,
                                       const Sample *const *objects_input,
                                       const Sample *const *direct_speakers_input,
                                       const Sample *const *hoa_input,
                                       Sample *const *output)
{
  impl->process(nsamples, objects_input, direct_speakers_input, hoa_input, output);
}

bool VariableBlockSizeAdapter::add_objects_block(size_t nsamples, size_t channel, ObjectsInput metadata)
{
  return impl->add_objects_block(nsamples, channel, std::move(metadata));
}

bool VariableBlockSizeAdapter::add_direct_speakers_block(size_t nsamples,
                                                         size_t channel,
                                                         DirectSpeakersInput metadata)
{
  return impl->add_direct_speakers_block(nsamples, channel, std::move(metadata));
}

bool VariableBlockSizeAdapter::add_hoa_block(size_t nsamples, size_t stream, HOAInput metadata)
{
  return impl->add_hoa_block(nsamples, stream, std::move(metadata));
}
}  // namespace bear
