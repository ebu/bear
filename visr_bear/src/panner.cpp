#include "panner.hpp"

#include "utils.hpp"

namespace {
ear::PolarPosition load_position(const rapidjson::Value &position_j)
{
  return {position_j["azimuth"].GetDouble(),
          position_j["elevation"].GetDouble(),
          position_j["distance"].GetDouble()};
}

ear::Channel load_channel(const rapidjson::Value &channel_j)
{
  ear::Channel channel;
  channel.name(channel_j["name"].GetString());

  channel.polarPosition(load_position(channel_j["polar_position"]));
  channel.polarPositionNominal(load_position(channel_j["polar_nominal_position"]));

  const auto &az_range_j = channel_j["az_range"].GetArray();
  channel.azimuthRange({{az_range_j[0].GetDouble(), az_range_j[1].GetDouble()}});

  const auto &el_range_j = channel_j["el_range"].GetArray();
  channel.elevationRange({{el_range_j[0].GetDouble(), el_range_j[1].GetDouble()}});

  channel.isLfe(channel_j["is_lfe"].GetBool());

  return channel;
}

ear::Layout load_layout(const rapidjson::Value &layout_j)
{
  std::vector<ear::Channel> channels;
  for (auto &channel_j : layout_j["channels"].GetArray()) {
    channels.push_back(load_channel(channel_j));
  }

  return {layout_j["name"].GetString(), channels};
}

void check(bool x, const std::string &message)
{
  if (!x) throw std::runtime_error("BEAR data file error: " + message);
}
}  // namespace

namespace bear {
using namespace Eigen;

Panner::Panner(const std::string &brir_file_name)
{
  auto tf = tensorfile::read(brir_file_name);

  views = tf.unpack<float>(tf.metadata["views"]);
  brirs = tf.unpack<float>(tf.metadata["brirs"]);
  delays = tf.unpack<float>(tf.metadata["delays"]);
  decorrelation_filters = tf.unpack<float>(tf.metadata["decorrelation_filters"]);
  fs = tf.metadata["fs"].GetDouble();
  decorrelation_delay_ = tf.metadata["decorrelation_delay"].GetDouble();
  front_loudspeaker_ = tf.metadata["front_loudspeaker"].GetUint();

  // check and unpack dimensions
  check(views->ndim() == 2, "views must have 2 dimensions");
  check(brirs->ndim() == 4, "brirs must have 4 dimensions");
  check(delays->ndim() == 3, "delays must have 3 dimensions");
  check(decorrelation_filters->ndim() == 2, "decorrelation_filters must have 2 dimensions");

  n_views_ = brirs->shape(0);
  n_virtual_loudspeakers_ = brirs->shape(1);
  brir_length_ = brirs->shape(3);
  decorrelator_length_ = decorrelation_filters->shape(1);

  check(views->shape(0) == n_views_, "views axis 0 is wrong size");
  check(views->shape(1) == 3, "views axis 1 is wrong size");

  check(brirs->shape(0) == n_views_, "brirs axis 0 is wrong size");
  check(brirs->shape(1) == n_virtual_loudspeakers_, "brirs axis 1 is wrong size");
  check(brirs->shape(2) == 2, "brirs axis 2 is wrong size");
  check(brirs->shape(3) == brir_length_, "brirs axis 3 is wrong size");

  check(delays->shape(0) == n_views_, "delays axis 0 is wrong size");
  check(delays->shape(1) == n_virtual_loudspeakers_, "delays axis 1 is wrong size");
  check(delays->shape(2) == 2, "delays axis 2 is wrong size");

  check(decorrelation_filters->shape(0) == n_virtual_loudspeakers_,
        "decorrelation filters axis 0 is wrong size");
  check(decorrelation_filters->shape(1) == decorrelator_length_,
        "decorrelation filters axis 1 is wrong size");

  // load layout
  ear::Layout layout;
  if (tf.metadata["layout"].IsString()) {
    std::string layout_name = tf.metadata["layout"].GetString();
    layout = ear::getLayout(layout_name).withoutLfe();
  } else {
    layout = load_layout(tf.metadata["layout"]);
  }

  // make gain calculators
  n_gains_ = layout.channels().size();
  gain_calc = std::make_unique<ear::GainCalculatorObjects>(layout);
  direct_speakers_gain_calc = std::make_unique<ear::GainCalculatorDirectSpeakers>(layout);

  temp_direct.resize(num_gains());
  temp_diffuse.resize(num_gains());
  temp_direct_diffuse.resize(num_gains() * 2);
  temp_direct_speakers.resize(num_gains());

  if (tf.metadata.HasMember("gain_norm_quick")) {
    gain_comp_type = GainCompType::QUICK;
    gain_comp_factors = tf.unpack<float>(tf.metadata["gain_norm_quick"]["factors"]);
    check(gain_comp_factors->ndim() == 5, "gain comp factors must have 5 dimensions");
    // TODO: check max delays?
    check(gain_comp_factors->shape(1) == n_views_, "gain comp factors axis 1 is wrong size");
    check(gain_comp_factors->shape(2) == n_virtual_loudspeakers_ * 2,
          "gain comp factors axis 2 is wrong size");
    check(gain_comp_factors->shape(3) == n_virtual_loudspeakers_ * 2,
          "gain comp factors axis 3 is wrong size");
    check(gain_comp_factors->shape(4) == 2, "gain comp factors axis 4 is wrong size");
  }

  check(tf.metadata.HasMember("hoa"), "HOA decoder not present");
  hoa_irs = tf.unpack<float>(tf.metadata["hoa"]["irs"]);
  check(hoa_irs->ndim() == 3, "HOA decoder must have 3 dimensions");
  n_hoa_channels_ = hoa_irs->shape(0);
  check(hoa_irs->shape(1) == 2, "HOA decoder axis 1 is wrong size");

  if (tf.metadata["hoa"].HasMember("delay")) hoa_delay_ = tf.metadata["hoa"]["delay"].GetDouble();

  hoa_order_ = ((size_t)std::round(std::sqrt(n_hoa_channels_))) - 1;
  size_t nch_for_order = (hoa_order_ + 1) * (hoa_order_ + 1);
  if (nch_for_order != n_hoa_channels_) throw std::logic_error("bad number of hoa channels");
}

void Panner::calc_objects_gains(const ear::ObjectsTypeMetadata &type_metadata,
                                DirectDiffuse<Ref<VectorXd>> gains) const
{
  if ((size_t)gains.direct.rows() != num_gains())
    throw std::invalid_argument("direct gains has wrong number of rows");
  if ((size_t)gains.diffuse.rows() != num_gains())
    throw std::invalid_argument("diffuse gains has wrong number of rows");

  gain_calc->calculate(type_metadata, temp_direct, temp_diffuse);

  for (size_t i = 0; i < num_gains(); i++) {
    gains.direct(i) = temp_direct[i];
    gains.diffuse(i) = temp_diffuse[i];
  }
}

void Panner::get_vs_gains(const DirectDiffuse<const Ref<const VectorXd> &> &gains,
                          DirectDiffuse<Ref<VectorXd>> vs_gains,
                          SelectedBRIR selected_brir) const
{
  if ((size_t)gains.direct.rows() != num_gains())
    throw std::invalid_argument("direct gains has wrong number of rows");
  if ((size_t)gains.diffuse.rows() != num_gains())
    throw std::invalid_argument("diffuse gains has wrong number of rows");
  if ((size_t)vs_gains.direct.rows() != num_virtual_loudspeakers())
    throw std::invalid_argument("direct vs_gains has wrong number of rows");
  if ((size_t)vs_gains.diffuse.rows() != num_virtual_loudspeakers())
    throw std::invalid_argument("diffuse vs_gains has wrong number of rows");

  vs_gains.direct = gains.direct;
  vs_gains.diffuse = gains.diffuse;
}

double Panner::get_default_direct_delay() const
{
  double front_delay = ((*delays)((size_t)0, front_loudspeaker_, (size_t)0) +
                        (*delays)((size_t)0, front_loudspeaker_, (size_t)1)) /
                       2.0;
  return (front_delay + decorrelation_delay_) / fs;
}

double Panner::get_default_static_delay() const
{
  double front_delay = ((*delays)((size_t)0, front_loudspeaker_, (size_t)0) +
                        (*delays)((size_t)0, front_loudspeaker_, (size_t)1)) /
                       2.0;
  return front_delay / fs;
}

template <typename Derived>
LeftRight<double> Panner::get_delays(const Eigen::DenseBase<Derived> &gains, SelectedBRIR selected_brir) const
{
  double total_gains_sum = gains.sum();

  LeftRight<double> mean_delays;

  if (total_gains_sum > 1e-6) {
    LeftRight<double> totals{0.0, 0.0};
    for (size_t i = 0; i < num_gains(); i++) {
      totals.left += (*delays)(selected_brir.brir_index, i, (size_t)0) * gains(i);
      totals.right += (*delays)(selected_brir.brir_index, i, (size_t)1) * gains(i);
    }
    mean_delays = {totals.left / total_gains_sum, totals.right / total_gains_sum};
  } else {
    mean_delays = {(*delays)(selected_brir.brir_index, front_loudspeaker_, (size_t)0),
                   (*delays)(selected_brir.brir_index, front_loudspeaker_, (size_t)1)};
  }

  return {(mean_delays.left + decorrelation_delay_) / fs, (mean_delays.right + decorrelation_delay_) / fs};
}

LeftRight<double> Panner::get_direct_delays(const DirectDiffuse<const Ref<const VectorXd> &> &gains,
                                            SelectedBRIR selected_brir) const
{
  // use auto / templates to avoid allocating a temporary
  auto total_gains = gains.direct + gains.diffuse;
  return get_delays(total_gains, selected_brir);
}

LeftRight<double> Panner::get_direct_speakers_delays(const Ref<const VectorXd> &gains,
                                                     SelectedBRIR selected_brir) const
{
  return get_delays(gains, selected_brir);
}

void Panner::get_diffuse_delays(LeftRight<Ref<VectorXd>> diffuse_delays, SelectedBRIR selected_brir) const
{
  if ((size_t)diffuse_delays.left.rows() != num_virtual_loudspeakers())
    throw std::invalid_argument("diffuse_delays left has wrong number of rows");
  if ((size_t)diffuse_delays.right.rows() != num_virtual_loudspeakers())
    throw std::invalid_argument("diffuse_delays right has wrong number of rows");

  for (size_t i = 0; i < num_virtual_loudspeakers(); i++) {
    diffuse_delays.left(i) = (*delays)(selected_brir.brir_index, i, (size_t)0) / fs;
    diffuse_delays.right(i) = (*delays)(selected_brir.brir_index, i, (size_t)1) / fs;
  }
}

void Panner::calc_direct_speakers_gains(const ear::DirectSpeakersTypeMetadata &type_metadata,
                                        Ref<VectorXd> gains) const
{
  if ((size_t)gains.rows() != num_gains()) throw std::invalid_argument("gains has wrong number of rows");

  direct_speakers_gain_calc->calculate(type_metadata, temp_direct_speakers);

  for (size_t i = 0; i < num_gains(); i++) {
    gains(i) = temp_direct_speakers[i];
  }
}

size_t Panner::decorrelator_length() const { return decorrelator_length_; }
const float *Panner::get_decorrelator(size_t i) const
{
  bear_assert(decorrelation_filters->stride(1) == 1, "decorrelation filters must be stored in C order");

  return &(*decorrelation_filters)(i, (size_t)0);
}

size_t Panner::brir_length() const { return brir_length_; }
const float *Panner::get_brir(size_t view, size_t virtual_loudspeaker, size_t ear) const
{
  bear_assert(brirs->stride(3) == 1, "BRIRs must be stored in C order");

  return &(*brirs)(view, virtual_loudspeaker, ear, (size_t)0);
}

Eigen::MatrixX3d Panner::get_views() const
{
  Eigen::MatrixX3d views_m(num_views(), 3);

  for (std::size_t view = 0; view < num_views(); view++)
    for (std::size_t i = 0; i < 3; i++) views_m(view, i) = (*views)(view, i);

  return views_m;
}

bool Panner::has_gain_compensation() const { return gain_comp_type != GainCompType::NONE; }

double Panner::get_real_gain_quick(double *gains,
                                   LeftRight<double> direct_delays,
                                   SelectedBRIR selected_brir) const
{
  // possible improvements:
  // - rearange the matrix so that the inner loop is along the last dimension,
  //   enabling vectorization, and so that the brir index is the first
  //   dimension, improving locality
  // - support sparse gains?
  // - there's no need to consider delays if there's only direct or diffuse
  //   gains; this would mean that all accesses can be made in a smaller region of memory
  tensorfile::NDArrayT<float> &factors = *gain_comp_factors;
  double sum = 0.0;
  for (size_t ear = 0; ear < 2; ear++) {
    double direct_delay_s = ear == 0 ? direct_delays.left : direct_delays.right;

    double delay_unclamped = direct_delay_s * fs - decorrelation_delay_;

    // clamp this so that we don't read out-of-bounds. First the delay is
    // clamped so that it's reading a valid point, then delay_int is clamped so
    // that both samples are valid. if delay_unclamped is shape - 1, then
    // delay_int will be shape - 2, and p will be 0 then 1
    double delay = std::min(std::max(0.0, delay_unclamped), (double)factors.shape(0) - 1);
    size_t delay_int = std::min((size_t)std::floor(delay), factors.shape(0) - 2);
    bear_assert(delay_int + 1 < factors.shape(0), "delay too large for factors");

    for (size_t sample_i = 0; sample_i < 2; sample_i++) {
      size_t sample = delay_int + sample_i;
      double p = 1 - std::abs(delay - sample);
      for (size_t i = 0; i < num_gains() * 2; i++) {
        double i_sum = 0.0;
        for (size_t j = 0; j < num_gains() * 2; j++)
          i_sum += gains[j] * factors(sample, selected_brir.brir_index, i, j, ear);
        sum += p * gains[i] * i_sum;
      }
    }
  }

  return std::sqrt(sum);
}

double Panner::get_real_gain_quick_direct(const Eigen::Ref<const Eigen::VectorXd> &gains,
                                          SelectedBRIR selected_brir) const
{
  tensorfile::NDArrayT<float> &factors = *gain_comp_factors;
  double sum = 0.0;
  for (size_t ear = 0; ear < 2; ear++) {
    for (size_t i = 0; i < num_gains(); i++) {
      double i_sum = 0.0;
      for (size_t j = 0; j < num_gains(); j++)
        i_sum += gains(j) * factors(0u, selected_brir.brir_index, i, j, ear);
      sum += gains(i) * i_sum;
    }
  }

  return std::sqrt(sum);
}

double Panner::get_expected_gain_quick(double *gains,
                                       LeftRight<double> direct_delays,
                                       SelectedBRIR selected_brir) const
{
  double sum = 0.0;
  tensorfile::NDArrayT<float> &factors = *gain_comp_factors;

  for (size_t ear = 0; ear < 2; ear++) {
    for (size_t j = 0; j < num_gains() * 2; j++)
      sum += gains[j] * gains[j] * factors(0u, selected_brir.brir_index, j, j, ear);
  }
  return std::sqrt(sum);
}

double Panner::get_expected_gain_quick_direct(const Eigen::Ref<const Eigen::VectorXd> &gains,
                                              SelectedBRIR selected_brir) const
{
  double sum = 0.0;
  tensorfile::NDArrayT<float> &factors = *gain_comp_factors;

  for (size_t ear = 0; ear < 2; ear++) {
    for (size_t j = 0; j < num_gains(); j++)
      sum += gains(j) * gains(j) * factors(0u, selected_brir.brir_index, j, j, ear);
  }
  return std::sqrt(sum);
}

double Panner::compensation_gain(double *gains,
                                 LeftRight<double> direct_delays,
                                 SelectedBRIR selected_brir) const
{
  if (gain_comp_type == GainCompType::NONE)
    return 1.0;
  else if (gain_comp_type == GainCompType::QUICK) {
    double expected_gain = get_expected_gain_quick(gains, direct_delays, selected_brir);
    double real_gain = get_real_gain_quick(gains, direct_delays, selected_brir);

    if (std::abs(real_gain) < 1e-6) return 1.0;
    return expected_gain / real_gain;
  } else
    throw std::logic_error("unknown gain_comp_type");
}

double Panner::compensation_gain(DirectDiffuse<Ref<VectorXd>> gains,
                                 LeftRight<double> direct_delays,
                                 SelectedBRIR selected_brir) const
{
  for (size_t i = 0; i < num_gains(); i++) {
    temp_direct_diffuse[i] = gains.direct(i);
    temp_direct_diffuse[num_gains() + i] = gains.diffuse(i);
  }

  return compensation_gain(temp_direct_diffuse.data(), direct_delays, selected_brir);
}

double Panner::compensation_gain_direct(const Ref<const VectorXd> &gains, SelectedBRIR selected_brir) const
{
  if (gain_comp_type == GainCompType::NONE)
    return 1.0;
  else if (gain_comp_type == GainCompType::QUICK) {
    double expected_gain = get_expected_gain_quick_direct(gains, selected_brir);
    double real_gain = get_real_gain_quick_direct(gains, selected_brir);

    if (std::abs(real_gain) < 1e-6) return 1.0;
    return expected_gain / real_gain;
  } else
    throw std::logic_error("unknown gain_comp_type");
}

const double Panner::decorrelation_delay() const { return (double)decorrelation_delay_ / fs; }

const float *Panner::get_hoa_ir(size_t channel, size_t ear) const
{
  bear_assert(hoa_irs->stride(2) == 1, "HOA decoder must be stored in C order");
  return &(*hoa_irs)(channel, ear, 0u);
}

}  // namespace bear
