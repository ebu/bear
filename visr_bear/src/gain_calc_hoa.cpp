#include "gain_calc_hoa.hpp"

#include <libvisr/time.hpp>
#include <map>

#include "ear_bits/hoa.hpp"
#include "sh_rotation.hpp"

namespace {
const std::map<std::string, bear::ear_bits::hoa::norm_f_t &> ADM_norm_types{
    {"N3D", bear::ear_bits::hoa::norm_N3D},
    {"SN3D", bear::ear_bits::hoa::norm_SN3D},
    {"FuMa", bear::ear_bits::hoa::norm_FuMa},
};
}

namespace bear {

GainCalcHOA::GainCalcHOA(const SignalFlowContext &ctx,
                         const char *name,
                         CompositeComponent *parent,
                         const ConfigImpl &config,
                         std::shared_ptr<Panner> panner_)
    : AtomicComponent(ctx, name, parent),
      panner(std::move(panner_)),
      sample_rate(config.sample_rate),
      metadata_in("metadata_in", *this, pml::EmptyParameterConfig()),
      gains_out(
          "gains_out", *this, pml::MatrixParameterConfig(panner->n_hoa_channels(), config.num_hoa_channels)),
      listener_in("listener_in", *this, pml::EmptyParameterConfig()),
      per_channel_data(config.num_hoa_channels, {panner->n_hoa_channels()}),
      sh_rotation_matrix(Eigen::MatrixXd::Identity(panner->n_hoa_channels(), panner->n_hoa_channels())),
      num_hoa_channels(panner->n_hoa_channels()),
      order(panner->hoa_order())
{
}

size_t GainCalcHOA::to_sample(const Time &t)
{
  return boost::rational_cast<size_t>(t * (int64_t)sample_rate);
}

GainCalcHOA::PerChannel::PerChannel(size_t n_hoa_channels) : to_hoa(n_hoa_channels) {}

void GainCalcHOA::update_for(std::size_t stream, const HOAInput &input)
{
  if (input.channels.size() != input.type_metadata.orders.size())
    throw std::invalid_argument("number of HOA input channels must match orders");
  if (input.channels.size() != input.type_metadata.degrees.size())
    throw std::invalid_argument("number of HOA input channels must match degrees");

  auto norm_it = ADM_norm_types.find(input.type_metadata.normalization);
  if (norm_it == ADM_norm_types.end())
    throw ear::adm_error("unknown normalization type: '" + input.type_metadata.normalization + "'");

  ear_bits::hoa::norm_f_t &norm_from = norm_it->second;
  ear_bits::hoa::norm_f_t &norm_to = ear_bits::hoa::norm_SN3D;

  Time block_end{0, 1};
  bool infinite_block = false;
  if (input.rtime && input.duration)
    block_end = *input.rtime + *input.duration;
  else
    infinite_block = true;

  for (size_t i = 0; i < input.channels.size(); i++) {
    int n = input.type_metadata.orders[i];
    int m = input.type_metadata.degrees[i];

    double convert = norm_to(n, std::abs(m)) / norm_from(n, std::abs(m));

    int acn = ear_bits::hoa::to_acn(n, m);

    PerChannel &channel = per_channel_data.at(input.channels[i]);
    channel.stream = stream;
    channel.last_block_end = block_end;
    channel.infinite_block = infinite_block;
    channel.changed = true;

    channel.to_hoa.setZero();
    // ignore orders that are not in the decode matrix
    if (acn < channel.to_hoa.size()) channel.to_hoa(acn) = convert;
  }
}

void GainCalcHOA::process()
{
  bool listener_changed = false;

  if (listener_in.changed()) {
    listener_changed = true;
    quaternion_to_sh_rotation_matrix(listener_in.data().orientation, order, sh_rotation_matrix);

    listener_in.resetChanged();
  }

  while (!metadata_in.empty()) {
    const auto &param = metadata_in.front();
    update_for(param.index, param.value);

    metadata_in.pop();
  }

  Time block_end{time().sampleCount() + period(), sample_rate};

  for (size_t i = 0; i < per_channel_data.size(); i++) {
    PerChannel &channel = per_channel_data.at(i);
    if (!channel.infinite_block && channel.last_block_end < block_end) {
      for (size_t j = 0; j < num_hoa_channels; j++) {
        gains_out.data()(j, i) = 0.0f;
      }
    } else {
      if (channel.changed || listener_changed) {
        using StrideT = Eigen::InnerStride<Eigen::Dynamic>;
        using MapT = Eigen::Map<Eigen::VectorXf, 0, StrideT>;

        MapT col(&(gains_out.data()(0, i)), num_hoa_channels, StrideT(gains_out.data().stride()));
        col = (sh_rotation_matrix * channel.to_hoa).cast<float>();
      }
    }

    channel.changed = false;
  }
}

}  // namespace bear
