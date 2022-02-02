/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <librrl/runtime_profiler.hpp>

#include <libefl/basic_vector.hpp>

#include <python/libpythonbindinghelpers/ndarray_from_basic_vector.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <algorithm>
#include <ciso646>
#include <vector>

namespace visr
{

using rrl::RuntimeProfiler;
namespace py = pybind11;

namespace python
{
namespace rrl
{

void exportRuntimeProfiler( py::module & m )
{
  py::class_< RuntimeProfiler >( m, "RuntimeProfiler" )
   // No constructoers are exported, as we will only use a reference returned
  // from the audio signal flow.
  .def_property_readonly( "numberOfComponents", &RuntimeProfiler::numberOfComponents )
  .def( "getComponentNames", &RuntimeProfiler::getComponentNames )
  .def( "measurementSamples", &RuntimeProfiler::measurementSamples, py::call_guard<py::gil_scoped_release>() )
  .def( "statisticsSamples", &RuntimeProfiler::statisticsSamples, py::call_guard<py::gil_scoped_release>() )
  .def( "getMeasurements", [](RuntimeProfiler & self )
  {
    std::size_t numAllTimings;
    // This swaps the measurment buffer and returns the number of // currently recorded measurements.
    RuntimeProfiler::MeasurementBuffer const * measurements{ nullptr };
    {
      py::gil_scoped_release guard;
      measurements = &(self.getMeasurements( numAllTimings ));
    }
    std::size_t numTimings{ std::min( numAllTimings, 
      self.measurementBufferSize() ) };
    py::array_t<RuntimeProfiler::TimeType > ret( 
      { numTimings, self.numberOfComponents() } // shape
    );
    std::size_t destRowIdx{ 0 };
    if( numAllTimings > self.measurementBufferSize() )
    {
      std::size_t const currentPos{ self.measurementSamples() % self.measurementBufferSize()};
      std::size_t const wrappedTimings
        = self.measurementBufferSize() - currentPos;
      for( std::size_t srcRowIdx{ currentPos }; destRowIdx < wrappedTimings;
        ++srcRowIdx, ++destRowIdx )
      {
        efl::vectorCopy( measurements->row( srcRowIdx ),
          ret.mutable_data( destRowIdx ), 
          self.numberOfComponents(),
          0/*alignment*/ );
      }
      // numTimings -= wrappedTimings;
    }
    for( std::size_t srcRowIdx{ 0 }; destRowIdx < numTimings;
      ++srcRowIdx, ++destRowIdx )
    {
      efl::vectorCopy( measurements->row( srcRowIdx ),
        ret.mutable_data( destRowIdx ), 
        self.numberOfComponents(),
        0/*alignment*/ );
    }
    return ret;
  } )
  .def( "getMean", []( RuntimeProfiler const & self )
  {
    visr::efl::BasicVector<RuntimeProfiler::TimeType> res( self.numberOfComponents(), 0/*alignment*/ );
    {
      py::gil_scoped_release guard;
      self.getMean( res );
    }
    auto retVal = ::visr::python::bindinghelpers::ndArrayFromBasicVector( res );
    return retVal;
  })
  .def( "getVariance", []( RuntimeProfiler const & self )
  {
    visr::efl::BasicVector<RuntimeProfiler::TimeType> res( self.numberOfComponents(), 0/*alignment*/ );
    {
      py::gil_scoped_release guard;
      self.getVariance( res );
    }
    auto retVal = ::visr::python::bindinghelpers::ndArrayFromBasicVector( res );
    return retVal;
  })
  .def( "getStatistics", []( RuntimeProfiler const & self )
   { 
    visr::efl::BasicVector<RuntimeProfiler::TimeType> mean( self.numberOfComponents(), 0/*alignment*/ );
    visr::efl::BasicVector<RuntimeProfiler::TimeType> variance( self.numberOfComponents(), 0/*alignment*/ );
    std::size_t numCycles;
    {
      py::gil_scoped_release guard;
      numCycles = self.getStatistics( mean, variance );
    }
    py::array_t<RuntimeProfiler::TimeType> meanArray = ::visr::python::bindinghelpers::ndArrayFromBasicVector( mean );
    py::array_t<RuntimeProfiler::TimeType> varianceArray = ::visr::python::bindinghelpers::ndArrayFromBasicVector( variance );
    py::tuple ret = py::make_tuple( numCycles, meanArray, varianceArray );
    return ret;
   } )
   .def( "getAndResetStatistics", []( RuntimeProfiler & self )
   { 
    visr::efl::BasicVector<RuntimeProfiler::TimeType> mean( self.numberOfComponents(), 0/*alignment*/ );
    visr::efl::BasicVector<RuntimeProfiler::TimeType> variance( self.numberOfComponents(), 0/*alignment*/ );
    std::size_t numCycles;
    {
      py::gil_scoped_release guard;
      numCycles = self.getAndResetStatistics( mean, variance );
    }
    py::array_t<RuntimeProfiler::TimeType> meanArray = ::visr::python::bindinghelpers::ndArrayFromBasicVector( mean );
    py::array_t<RuntimeProfiler::TimeType> varianceArray = ::visr::python::bindinghelpers::ndArrayFromBasicVector( variance );
    py::tuple ret = py::make_tuple( numCycles, meanArray, varianceArray );
    return ret;
   } )
  ;
}

} // namespace rrl
} // namespace python
} // namespace visr
