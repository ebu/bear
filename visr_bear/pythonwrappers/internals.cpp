#include <pybind11/eigen.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include <libvisr/signal_flow_context.hpp>

#include "boost_variant.hpp"
#include "direct_speakers_gain_calc.hpp"
#include "gain_calc_hoa.hpp"
#include "gain_calc_objects.hpp"
#include "parameters.hpp"
#include "select_brir.hpp"

namespace bear {
namespace python {

  namespace py = pybind11;

  void export_internals(pybind11::module &m)
  {
    init_parameters();

    // make Config convertable to ConfigImpl; this is what the internals use,
    // and this means that there's only one config interface on the python side
    // where we don't need to care about ABI compatibility.
    py::class_<ConfigImpl>(m, "_ConfigImpl").def(py::init([](const Config &config) {
      return ConfigImpl(config.get_impl());
    }));
    py::implicitly_convertible<Config, ConfigImpl>();

    py::class_<Panner, std::shared_ptr<Panner>>(m, "Panner").def(py::init<const std::string &>());

    pybind11::class_<ADMParameter<ObjectsInput>, ParameterBase>(m, "ADMParameterObjects")
        .def(py::init<size_t, ObjectsInput>());

    py::class_<GainCalcObjects, visr::AtomicComponent>(m, "GainCalcObjects")
        .def(py::init<const SignalFlowContext &,
                      const char *,
                      CompositeComponent *,
                      const ConfigImpl &,
                      std::shared_ptr<Panner>>());

    py::class_<DirectSpeakersGainCalc, visr::AtomicComponent>(m, "DirectSpeakersGainCalc")
        .def(py::init<const SignalFlowContext &,
                      const char *,
                      CompositeComponent *,
                      const ConfigImpl &,
                      std::shared_ptr<Panner>>());

    pybind11::class_<ADMParameter<DirectSpeakersInput>, ParameterBase>(m, "ADMParameterDirectSpeakers")
        .def(py::init<size_t, DirectSpeakersInput>());

    pybind11::class_<ListenerParameter, ParameterBase>(m, "ListenerParameter")
        .def(py::init<>())
        .def_readwrite("position", &ListenerParameter::position)
        .def_property(
            "orientation",
            [](const ListenerParameter &lp) {
              return Eigen::Vector4d(
                  lp.orientation.w(), lp.orientation.x(), lp.orientation.y(), lp.orientation.z());
            },
            [](ListenerParameter &lp, Eigen::Vector4d v) {
              lp.orientation = {v(0), v(1), v(2), v(3)};
            });

    py::class_<SelectBRIR, visr::AtomicComponent>(m, "SelectBRIR")
        .def(py::init<const SignalFlowContext &,
                      const char *,
                      CompositeComponent *,
                      const ConfigImpl &,
                      std::shared_ptr<Panner>>());

    py::class_<GainCalcHOA, visr::AtomicComponent>(m, "GainCalcHOA")
        .def(py::init<const SignalFlowContext &,
                      const char *,
                      CompositeComponent *,
                      const ConfigImpl &,
                      std::shared_ptr<Panner>>());

    pybind11::class_<ADMParameter<HOAInput>, ParameterBase>(m, "ADMParameterHOA")
        .def(py::init<size_t, HOAInput>());
  }

}  // namespace python
}  // namespace bear
