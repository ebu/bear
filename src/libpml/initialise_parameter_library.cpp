/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "initialise_parameter_library.hpp"

#include "double_buffering_protocol.hpp"
#include "message_queue_protocol.hpp"
#include "shared_data_protocol.hpp"

#include "biquad_parameter.hpp"
#include "filter_routing_parameter.hpp"
#include "indexed_value_parameter.hpp"
#include "matrix_parameter.hpp"
#include "listener_position.hpp"
#include "interpolation_parameter.hpp"
#include "object_vector.hpp"
#include "scalar_parameter.hpp"
#include "string_parameter.hpp"
#include "signal_routing_parameter.hpp"
#include "sparse_gain_routing_parameter.hpp"
#include "time_frequency_parameter.hpp"
#include "vector_parameter.hpp"

#include <libvisr/parameter_factory.hpp>
#include <libvisr/communication_protocol_factory.hpp>

#include <vector>

namespace visr
{
namespace pml
{

void initialiseParameterLibrary()
{
  static ParameterRegistrar<
    BiquadParameterMatrix<float>,
    BiquadParameterMatrix<double>,
    FilterRoutingParameter,
    FilterRoutingListParameter,
    IndexedVectorDoubleType,
    IndexedVectorFloatType,
    IndexedStringType,
    InterpolationParameter,
    ListenerPosition,
    MatrixParameter<float>,
    MatrixParameter<double>,
    MatrixParameter<std::complex<float> >,
    MatrixParameter<std::complex<double> >,

    ObjectVector,

    ListenerPosition,

    ScalarParameter<bool>,
    ScalarParameter<int>,
    ScalarParameter<unsigned int>,
    ScalarParameter<float>,
    ScalarParameter<double>,
    ScalarParameter<std::complex<float> >,
    ScalarParameter<std::complex<double> >,

    SignalRoutingParameter,

    SparseGainRoutingListParameter,
    SparseGainRoutingParameter,

    StringParameter,

    TimeFrequencyParameter<float>,
    TimeFrequencyParameter<double>,

    VectorParameter<float>,
    VectorParameter<double>,
    VectorParameter< std::complex<float> >,
    VectorParameter< std::complex<double> > >

 sParameterRegistrar;

  static CommunicationProtocolRegistrar<
    DoubleBufferingProtocol,
    MessageQueueProtocol,
    SharedDataProtocol >
  sProtocolRegistrar;


}

} // namespace pml
} // namespace visr
