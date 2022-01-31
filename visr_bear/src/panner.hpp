#pragma once
#include <Eigen/Core>

#include "ear/ear.hpp"
#include "tensorfile.hpp"

namespace bear {

/// holds two arbitrary values relating to the direct and diffuse paths
template <typename T>
struct DirectDiffuse {
  T direct;
  T diffuse;
};

/// holds two arbitrary values relating to the left and right channels/ears
template <typename T>
struct LeftRight {
  T left;
  T right;
};

/// reference to a single view from a multi-view BRIR set
struct SelectedBRIR {
  size_t brir_index;
  // TODO: add orientation here
};

using namespace Eigen;

/// Holds all non-user-configurable renderer information (like virtual
/// loudspeaker layouts, BRIR sets, delay sets, decorrelation filters), and
/// provides methods intended to be used to drive the baseline DSP (like
/// calculating gains and delays, or accessing the BRIR sets).
///
/// For Objects, the data flow should look like this:
///
///   type_metadata -calc_objects_gains-> gains
///   gains, selected_brir -get_vs_gains-> vs_gains
///   gains, selected_brir -get_direct_delays-> direct_delays
///   selected_brir -get_diffuse_delays-> diffuse_delays
///
/// vs_gains, direct_delays and diffuse_delays are the gains and delays used by
/// the DSP components.
///
/// Note that the gains (length num_gains()) and virtual loudspeaker gains
/// (length num_virtual_loudspeakers()) are separate, so that extra
/// processing (e.g. gain normalisation, virtual loudspeaker remapping) can be
/// inserted without affecting the delay calculation.
class Panner {
 public:
  Panner(const std::string &brir_file_name);

  size_t num_gains() const { return n_gains_; }
  size_t num_virtual_loudspeakers() const { return n_virtual_loudspeakers_; }
  size_t num_views() const { return n_views_; }

  void calc_objects_gains(const ear::ObjectsTypeMetadata &type_metadata,
                          DirectDiffuse<Ref<VectorXd>> gains) const;

  void get_vs_gains(const DirectDiffuse<const Ref<const VectorXd> &> &gains,
                    DirectDiffuse<Ref<VectorXd>> vs_gains,
                    SelectedBRIR selected_brir = {}) const;

  // frontal delay used to initialise direct path
  double get_default_direct_delay() const;
  // frontal delay used to initialise static path
  double get_default_static_delay() const;

  LeftRight<double> get_direct_delays(const DirectDiffuse<const Ref<const VectorXd> &> &gains,
                                      SelectedBRIR selected_brir = {}) const;

  LeftRight<double> get_direct_speakers_delays(const Ref<const VectorXd> &gains,
                                               SelectedBRIR selected_brir = {}) const;

  void get_diffuse_delays(LeftRight<Ref<VectorXd>> diffuse_delays, SelectedBRIR selected_brir = {}) const;

  void calc_direct_speakers_gains(const ear::DirectSpeakersTypeMetadata &type_metadata,
                                  Ref<VectorXd> gains) const;

  size_t decorrelator_length() const;
  const float *get_decorrelator(size_t i) const;

  size_t brir_length() const;
  const float *get_brir(size_t view, size_t virtual_loudspeaker, size_t ear) const;

  Eigen::MatrixX3d get_views() const;

  bool has_gain_compensation() const;
  // delays in seconds
  double compensation_gain(double *gains,
                           LeftRight<double> direct_delays,
                           SelectedBRIR selected_brir = {}) const;
  double compensation_gain(DirectDiffuse<Ref<VectorXd>> gains,
                           LeftRight<double> direct_delays,
                           SelectedBRIR selected_brir) const;

  double compensation_gain_direct(const Ref<const VectorXd> &gains, SelectedBRIR selected_brir) const;

  const double decorrelation_delay() const;

  size_t n_hoa_channels() const { return n_hoa_channels_; }
  size_t hoa_order() const { return hoa_order_; }
  size_t hoa_ir_length() const { return hoa_irs->shape(2); }
  const float *get_hoa_ir(size_t channel, size_t ear) const;
  double hoa_delay() const { return hoa_delay_ / fs; }

 private:
  double get_real_gain_quick(double *gains,
                             LeftRight<double> direct_delays,
                             SelectedBRIR selected_brir) const;
  double get_expected_gain_quick(double *gains,
                                 LeftRight<double> direct_delays,
                                 SelectedBRIR selected_brir) const;

  double get_real_gain_quick_direct(const Ref<const VectorXd> &gains, SelectedBRIR selected_brir) const;
  double get_expected_gain_quick_direct(const Ref<const VectorXd> &gains, SelectedBRIR selected_brir) const;

  template <typename Derived>
  LeftRight<double> get_delays(const Eigen::DenseBase<Derived> &gains, SelectedBRIR selected_brir = {}) const;

  size_t n_gains_;
  size_t n_views_;
  size_t n_virtual_loudspeakers_;
  size_t brir_length_;
  size_t decorrelator_length_;
  size_t decorrelation_delay_;
  size_t front_loudspeaker_;
  size_t n_hoa_channels_;
  size_t hoa_order_;
  double hoa_delay_ = 0.0;
  enum class GainCompType { NONE, QUICK } gain_comp_type = GainCompType::NONE;

  std::shared_ptr<tensorfile::NDArrayT<float>> views;
  std::shared_ptr<tensorfile::NDArrayT<float>> brirs;
  std::shared_ptr<tensorfile::NDArrayT<float>> delays;
  std::shared_ptr<tensorfile::NDArrayT<float>> decorrelation_filters;
  std::shared_ptr<tensorfile::NDArrayT<float>> gain_comp_factors;
  std::shared_ptr<tensorfile::NDArrayT<float>> hoa_irs;
  double fs;

  std::unique_ptr<ear::GainCalculatorObjects> gain_calc;
  std::unique_ptr<ear::GainCalculatorDirectSpeakers> direct_speakers_gain_calc;

  mutable std::vector<double> temp_direct;
  mutable std::vector<double> temp_diffuse;
  mutable std::vector<double> temp_direct_diffuse;

  mutable std::vector<double> temp_direct_speakers;
};

}  // namespace bear
