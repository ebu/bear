#include "bear/api.hpp"

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include "array_conversion.hpp"
#include "boost_variant.hpp"
#include "config_impl.hpp"

namespace bear {
namespace python {

  namespace py = pybind11;

  class PyDistanceBehaviour : public DistanceBehaviour {
   public:
    double get_gain(double distance_m, boost::optional<double> absoluteDistance) override
    {
      PYBIND11_OVERRIDE_PURE(double, DistanceBehaviour, get_gain, distance_m, absoluteDistance);
    }
  };

  void export_api(pybind11::module &m)
  {
    using namespace ear;

    py::class_<Config>(m, "Config")
        .def(py::init<>())
        .def(py::init<Config>())
        .def_property(
            "num_objects_channels", &Config::get_num_objects_channels, &Config::set_num_objects_channels)
        .def_property(
            "num_objects_channels", &Config::get_num_objects_channels, &Config::set_num_objects_channels)
        .def_property("num_direct_speakers_channels",
                      &Config::get_num_direct_speakers_channels,
                      &Config::set_num_direct_speakers_channels)
        .def_property("num_hoa_channels", &Config::get_num_hoa_channels, &Config::set_num_hoa_channels)
        .def_property("period_size", &Config::get_period_size, &Config::set_period_size)
        .def_property("sample_rate", &Config::get_sample_rate, &Config::set_sample_rate)
        .def_property("data_path", &Config::get_data_path, &Config::set_data_path)
        .def_property("fft_implementation", &Config::get_fft_implementation, &Config::set_fft_implementation)
        .def("validate", &Config::validate);

    py::class_<DistanceBehaviour, PyDistanceBehaviour, std::shared_ptr<DistanceBehaviour>>(
        m, "DistanceBehaviour")
        .def(py::init<>())
        .def("get_gain", &DistanceBehaviour::get_gain);

    py::class_<AudioPackFormatData>(m, "AudioPackFormatData")
        .def(py::init<>())
        .def_readwrite("absoluteDistance", &AudioPackFormatData::absoluteDistance);

    py::class_<MetadataInput>(m, "MetadataInput")
        .def(py::init<>())
        .def_readwrite("rtime", &MetadataInput::rtime)
        .def_readwrite("duration", &MetadataInput::duration)
        .def_readwrite("audioPackFormat_data", &MetadataInput::audioPackFormat_data);

    py::class_<ObjectsInput, MetadataInput>(m, "ObjectsInput")
        .def(py::init<>())
        .def(py::init<const ObjectsInput &>())
        .def_readwrite("type_metadata", &ObjectsInput::type_metadata)
        .def_readwrite("interpolationLength", &ObjectsInput::interpolationLength)
        .def_readwrite("distance_behaviour", &ObjectsInput::distance_behaviour);

    py::class_<DirectSpeakersInput, MetadataInput>(m, "DirectSpeakersInput")
        .def(py::init<>())
        .def(py::init<const DirectSpeakersInput &>())
        .def_readwrite("type_metadata", &DirectSpeakersInput::type_metadata);

    py::class_<PolarPosition>(m, "PolarPosition")
        .def(py::init<double, double, double>())
        .def_readwrite("azimuth", &ear::PolarPosition::azimuth)
        .def_readwrite("elevation", &ear::PolarPosition::elevation)
        .def_readwrite("distance", &ear::PolarPosition::distance);

    py::class_<CartesianPosition>(m, "CartesianPosition")
        .def(py::init<double, double, double>())
        .def_readwrite("X", &ear::CartesianPosition::X)
        .def_readwrite("Y", &ear::CartesianPosition::Y)
        .def_readwrite("Z", &ear::CartesianPosition::Z);

    py::class_<ObjectsTypeMetadata>(m, "ObjectsTypeMetadata")
        .def(py::init<>())
        .def_readwrite("position", &ObjectsTypeMetadata::position)
        .def_readwrite("width", &ObjectsTypeMetadata::width)
        .def_readwrite("height", &ObjectsTypeMetadata::height)
        .def_readwrite("depth", &ObjectsTypeMetadata::depth)
        .def_readwrite("gain", &ObjectsTypeMetadata::gain)
        .def_readwrite("diffuse", &ObjectsTypeMetadata::diffuse);

    py::class_<PolarSpeakerPosition>(m, "PolarSpeakerPosition").def(py::init<double, double, double>());
    py::class_<CartesianSpeakerPosition>(m, "CartesianSpeakerPosition")
        .def(py::init<double, double, double>());

    py::class_<DirectSpeakersTypeMetadata>(m, "DirectSpeakersTypeMetadata")
        .def(py::init<>())
        .def_readwrite("speakerLabels", &DirectSpeakersTypeMetadata::speakerLabels)
        .def_readwrite("position", &DirectSpeakersTypeMetadata::position)
        .def_readwrite("channelFrequency", &DirectSpeakersTypeMetadata::channelFrequency)
        .def_readwrite("audioPackFormatID", &DirectSpeakersTypeMetadata::audioPackFormatID);

    py::class_<HOAInput, MetadataInput>(m, "HOAInput")
        .def(py::init<>())
        .def(py::init<const HOAInput &>())
        .def_readwrite("type_metadata", &HOAInput::type_metadata)
        .def_readwrite("channels", &HOAInput::channels);

    py::class_<HOATypeMetadata>(m, "HOATypeMetadata")
        .def(py::init<>())
        .def_readwrite("orders", &HOATypeMetadata::orders)
        .def_readwrite("degrees", &HOATypeMetadata::degrees)
        .def_readwrite("normalization", &HOATypeMetadata::normalization);

    py::class_<Listener>(m, "Listener")
        .def(py::init<>())
        .def_property("position_cart", &Listener::get_position_cart, &Listener::set_position_cart)
        .def_property("orientation_quaternion",
                      &Listener::get_orientation_quaternion,
                      &Listener::set_orientation_quaternion)
        .def_property_readonly("look", &Listener::look)
        .def_property_readonly("right", &Listener::right)
        .def_property_readonly("up", &Listener::up);

    py::class_<Renderer>(m, "RendererBase");

    struct RendererWrapper : public Renderer {
      RendererWrapper(const Config &config) : Renderer(config), config(config.get_impl()) {}
      ConfigImpl config;
    };

    py::class_<RendererWrapper, Renderer>(m, "Renderer")
        .def(py::init<const Config &>())
        .def("add_objects_block", &Renderer::add_objects_block)
        .def("add_direct_speakers_block", &Renderer::add_direct_speakers_block)
        .def("add_hoa_block", &Renderer::add_hoa_block)
        .def(
            "process",
            [](RendererWrapper &r,
               py::array_t<float, 0> objects_input,
               py::array_t<float, 0> direct_speakers_input,
               py::array_t<float, 0> hoa_input,
               py::array_t<float, 0> output) {
              std::vector<float *> objects_input_ptrs = py_array_to_pointers(
                  objects_input, r.config.num_objects_channels, r.config.period_size, "objects_input");
              std::vector<float *> direct_speakers_input_ptrs =
                  py_array_to_pointers(direct_speakers_input,
                                       r.config.num_direct_speakers_channels,
                                       r.config.period_size,
                                       "direct_speakers_input");
              std::vector<float *> hoa_input_ptrs = py_array_to_pointers(
                  hoa_input, r.config.num_hoa_channels, r.config.period_size, "hoa_input");
              std::vector<float *> output_ptrs =
                  py_array_to_pointers(output, 2, r.config.period_size, "output");

              r.process(objects_input_ptrs.data(),
                        direct_speakers_input_ptrs.data(),
                        hoa_input_ptrs.data(),
                        output_ptrs.data());
            },
            py::arg("objects_input").noconvert(true),
            py::arg("direct_speakers_input").noconvert(true),
            py::arg("hoa_input").noconvert(true),
            py::arg("output").noconvert(true))
        .def("get_block_start_time", &Renderer::get_block_start_time)
        .def("set_block_start_time", &Renderer::set_block_start_time)
        .def("set_listener", &Renderer::set_listener);

    py::class_<Time>(m, "Time")
        .def(py::init<int64_t>())
        .def(py::init<int64_t, int64_t>())
        .def_property_readonly("numerator", &Time::numerator)
        .def_property_readonly("denominator", &Time::denominator);

    py::class_<DataFileMetadata>(m, "DataFileMetadata")
        .def_static("read_from_file", &DataFileMetadata::read_from_file)
        .def_property_readonly("has_metadata", &DataFileMetadata::has_metadata)
        .def_property_readonly("label", &DataFileMetadata::get_label)
        .def_property_readonly("description", &DataFileMetadata::get_description)
        .def_property_readonly("is_released", &DataFileMetadata::is_released);
  }

}  // namespace python
}  // namespace bear
