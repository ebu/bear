/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <librbbl/interpolating_convolver_uniform.hpp>

#include <libefl/basic_matrix.hpp>

#include <libpml/matrix_parameter.hpp>

#include <librbbl/filter_routing.hpp>

#include <libvisr/detail/compose_message_string.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace visr
{
namespace rbbl
{
namespace python
{

namespace // unnamed
{

template<typename DataType>
pybind11::array_t<DataType> wrapProcess( InterpolatingConvolverUniform<DataType> & convolver, pybind11::array const & input )
{
  if( not input.dtype().is( pybind11::dtype::of<DataType>() ) )
  {
    throw std::invalid_argument( "InterpolatingConvolverUniform::process(): The data type input matrix does not match the used sample data type." );
  }
  if( input.ndim() != 2 )
  {
    throw std::invalid_argument( "InterpolatingConvolverUniform::process(): The input matrix is not 2D" );
  }
  if( input.shape( 0 ) != static_cast<pybind11::ssize_t>(convolver.numberOfInputs()) )
  {
    throw std::invalid_argument( "InterpolatingConvolverUniform::process(): Dimension 1 input of the input matrix does not match the number of capture channels." );
  }
  if( input.shape( 1 ) != static_cast<pybind11::ssize_t>(convolver.blockLength()) )
  {
    throw std::invalid_argument( "InterpolatingConvolverUniform::process(): Dimension 0 input of the input matrix does not match the block size of the convolver." );
  }
  if( input.strides(1) != sizeof(DataType) )
  {
    throw std::invalid_argument( "InterpolatingConvolverUniform::process(): The channel data must be consecutive." );
  }

  pybind11::array outputSignal( pybind11::dtype::of<DataType>(),
  { convolver.numberOfOutputs(), convolver.blockLength() },
      { sizeof( DataType )*convolver.blockLength(), sizeof( DataType ) } // TODO: Take care of alignment
      );
      std::size_t const outChannelStride = outputSignal.strides( 0 ) / sizeof( DataType );
      try
      {
        convolver.process( static_cast<DataType const *>(input.data()),
          input.strides( 0 ) / sizeof( DataType ),
          static_cast<DataType *>(outputSignal.mutable_data()),
          outChannelStride );
      }
      catch( std::exception const & ex )
      {
        // For the time being we are using std::invalid_argument because it is recognised by pybind11 and translated to a proper Python exeption
        // todo: register a more fitting exception, e.g., std::runtime_error
        throw std::invalid_argument( detail::composeMessageString( "Exception while execution signal flow:", ex.what() ) );
      }
      return outputSignal;
    }


/**
 * Templated export function for concrete InterpolatingConvolverUniform instantiations with different
 * sample types.
 */
template<typename ElementType>
void exportInterpolatingConvolverUniform( pybind11::module & m, char const * name )
{
  pybind11::class_< InterpolatingConvolverUniform<ElementType> >( m, name )
    .def( pybind11::init<std::size_t, std::size_t, std::size_t, std::size_t, std::size_t, std::size_t, std::size_t, std::size_t,
    		rbbl::FilterRoutingList const &, InterpolationParameterSet const &,
      efl::BasicMatrix<ElementType> const &, std::size_t, char const *>(),
        pybind11::arg( "numberOfInputs" ),
        pybind11::arg( "numberOfOutputs" ),
        pybind11::arg( "blockLength" ),
        pybind11::arg( "maxFilterLength" ),
        pybind11::arg( "maxRoutingPoints" ),
        pybind11::arg( "maxFilterEntries" ),
		pybind11::arg( "numberOfInterpolants" ),
        pybind11::arg( "transitionSamples"),
        pybind11::arg( "initialRoutings" ) = rbbl::FilterRoutingList(),
        pybind11::arg( "initialInterpolants" ) = rbbl::InterpolationParameterSet(),
        pybind11::arg( "initialFilters" ) /*= pml::MatrixParameter<ElementType>()*/,
        pybind11::arg( "alignment" ) = 0,
        pybind11::arg( "fftImplementation") = "default" )
    .def_property_readonly_static( "numberOfInputs", &InterpolatingConvolverUniform<ElementType>::numberOfInputs )
    .def_property_readonly_static( "numberOfOutputs", &InterpolatingConvolverUniform<ElementType>::numberOfOutputs )
    .def_property_readonly_static( "blockLength", &InterpolatingConvolverUniform<ElementType>::blockLength )
    .def_property_readonly_static( "maxNumberOfRoutingPoints", &InterpolatingConvolverUniform<ElementType>::maxNumberOfRoutingPoints )
    .def_property_readonly_static( "maxNumberOfFilterEntries", &InterpolatingConvolverUniform<ElementType>::maxNumberOfFilterEntries )
    .def_property_readonly_static( "maxFilterLength", &InterpolatingConvolverUniform<ElementType>::maxFilterLength )
    .def_property_readonly_static( "numberOfRoutingPoints", &InterpolatingConvolverUniform<ElementType>::numberOfRoutingPoints )
    .def_property_readonly_static( "numberOfInterpolants", &InterpolatingConvolverUniform<ElementType>::numberOfInterpolants )
    .def( "process", []( InterpolatingConvolverUniform<ElementType> & convolver, pybind11::array const & input )
       { return wrapProcess( convolver, input ); }, pybind11::arg("input") )
    .def( "clearRoutingTable", &InterpolatingConvolverUniform<ElementType>::clearRoutingTable )
    .def( "initRoutingTable", &InterpolatingConvolverUniform<ElementType>::initRoutingTable, pybind11::arg( "routings" ) )
    .def( "setRoutingEntry", static_cast<void(InterpolatingConvolverUniform<ElementType>::*)(rbbl::FilterRouting const &)>(&InterpolatingConvolverUniform<ElementType>::setRoutingEntry), pybind11::arg("routing") )
    .def( "setRoutingEntry", static_cast<void(InterpolatingConvolverUniform<ElementType>::*)(std::size_t, std::size_t, std::size_t, ElementType)>
      (&InterpolatingConvolverUniform<ElementType>::setRoutingEntry),
      pybind11::arg( "inputIndex"), pybind11::arg( "outputIndex"), pybind11::arg( "filterIndex"), pybind11::arg( "gain") = 1.0 )
    .def( "removeRoutingEntry", static_cast<bool(InterpolatingConvolverUniform<ElementType>::*)(std::size_t, std::size_t)>(&InterpolatingConvolverUniform<ElementType>::removeRoutingEntry),
        pybind11::arg( "inputIndex" ), pybind11::arg( "outputIndex" ) )
    .def( "clearFilters", &InterpolatingConvolverUniform<ElementType>::clearFilters )
    .def( "initFilters", &InterpolatingConvolverUniform<ElementType>::initFilters, pybind11::arg( "newFilters" ) )
    .def( "setImpulseResponse", &InterpolatingConvolverUniform<ElementType>::setImpulseResponse,
      pybind11::arg("ir"), pybind11::arg( "filterLength"), pybind11::arg( "filterIdx" ),
      pybind11::arg( "alignment" ) = 0 )
    .def( "setInterpolant", &InterpolatingConvolverUniform<ElementType>::setInterpolant, pybind11::arg( "interpolant" ), pybind11::arg( "startTransition" ) )
    .def( "setInterpolants", &InterpolatingConvolverUniform<ElementType>::setInterpolants, pybind11::arg( "interpolants" ), pybind11::arg( "startTransition" ) )
    .def( "clearInterpolants", &InterpolatingConvolverUniform<ElementType>::clearInterpolants )
  ;
}

} // unnamed namespace

void exportInterpolatingConvolversUniform( pybind11::module & m )
{
  exportInterpolatingConvolverUniform<float>( m, "InterpolatingConvolverUniformFloat" );
  exportInterpolatingConvolverUniform<double>( m, "InterpolatingConvolverUniformDouble" );
}

} // namespace python
} // namepace rbbl
} // namespace visr
