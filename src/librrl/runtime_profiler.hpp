/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBRRL_RUNTIME_PROFILER_HPP_INCLUDED
#define VISR_LIBRRL_RUNTIME_PROFILER_HPP_INCLUDED

#include "export_symbols.hpp"

#include <libefl/basic_matrix.hpp>
#include <libefl/basic_vector.hpp>

#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace visr
{
class Component;

namespace rrl
{
class AudioSignalFlow;


/**
 * Class for collecting and calculating runtime performance statistics 
 * on the per-component level.
 * A RuntimeProfiler is instantiated in an AudioSignalFlow instance 
 * if profiling support is active in the build and it is activated 
 * for the AudioSignalFlow instance.
 * Profiling data is either calculated in the form of running mean and variance,
 * or accessible as raw data, that is execution times for each
 * iteration.
 */
class VISR_RRL_LIBRARY_SYMBOL RuntimeProfiler
{
public:
  /**
   * The type to represent all timing data (in seconds).
   */
  using TimeType = double;

  /**
   * Type to hold timing data per iteration.
   * This is a matrix with one row for each stored measurement, i.e.,
   * \p measureBufferSize, while the number of columns corresponds to the 
   * number of atomic components in the graph.
   */
  using MeasurementBuffer = efl::BasicMatrix< TimeType >;

  /**
   * Vector type to hold on value per measured atomic component.
   */
  using MeasurementVector = efl::BasicVector< TimeType >;
  
  /**
   * Constructor.
   * @param flow The component (composite or atomic) containing the processing
   * functionality.
   * @param measureBufferSize The size of the sample buffer for measurements,
   * i.e., how many iterations are kept in memory. A value of zero disables 
   * iteration-based measurements.
   */
  explicit RuntimeProfiler( AudioSignalFlow const & flow,
                           std::size_t measureBufferSize );

  /**
   * Destructor.
   */
  ~RuntimeProfiler();

  /**
   * Return the number of components for which runtime statistics are obtained.
   */
  std::size_t numberOfComponents() const;
    
  /**
   * Reset the iteration-wise timing data.
   * This resets the timing buffer counter.
   */
  void resetMeasurements();

  /**
   * Return the mean of the execution times
   * @param [out] mean A vector to be filled with the measurement data.
   * If no iterations have been measured since the last reset of the
   * statistics, all values will be set to NaN.
   * @throw std::invalid_argument If the return value buffer \p mean has
   * the wrong size.
   * Note: This method is thread safe, using the same critical section
   * guard as the associated AudioSignalFlow.
   */
  void getMean( MeasurementVector & mean ) const;

  /**
   * Return the variances since the last reset of the statistics counter.
   * @throw std::logic_error If there are less than 2 iterations since the last reset of the statistics.
   */
  void getVariance( MeasurementVector & variance ) const;

  /**
   * Return statistics data (mean and variance) and optionally reset the 
   * statistics.
   */
  std::size_t getStatistics( MeasurementVector & mean,
    MeasurementVector & variance ) const;

  std::size_t getAndResetStatistics( MeasurementVector & mean,
    MeasurementVector & variance );

  /**
   * Resets the timinng statistivs, i.e., mean and variance of 
   * execution times. This also resets the statistics counter.
   * 
   */
  void resetStatistics();
  
  /**
   * Return the names of the components as a list of strings.
   */
  std::vector< std::string > getComponentNames() const;
  
  /**
   * Return the names of the measured components into a stream.
   * @param os The stream to write into.
   * @param separator Separator string between the names, default is ','.
   */
  void writeComponentNames( std::ostream & os,
    std::string const & separator = std::string(",") ) const;
  
  /**
   * Return the number of measured iterations since the last reset of the 
   * measurement data.
   * This number is monotonously increasing (unless the measuremtn is
   * reset). That is the the counter does not not stop if measurement
   * buffer is filled completely, but continues counting as the buffer
   * contents is cyclically overwritten.
   * @note This method is thread-safe because it performs locking before
   * accessing data that is potentially accessed by other threads.
   */
  std::size_t measurementSamples() const;

  /**
   * @note This method is thread-safe because it performs locking before
   * accessing data that is potentially accessed by other threads.
   */
  std::size_t statisticsSamples() const;

  /**
   * Return the size of the measurement buffer, i.e., the number of iterations
   * for which data can be stored.
   * @note This method is thread-safe because it does not access modifiablea 
   * data shared with other threads.
   */
  std::size_t measurementBufferSize() const;

  /**
   */
  MeasurementBuffer const & getMeasurements( std::size_t & numIterations );

private:
  friend class AudioSignalFlow;

  /**
   * Provide the current execution time for a component.
   */
  void setTime( std::size_t componentIdx, TimeType timing );
  
  /**
   * Complete a processing iteration.
   * The execution times measured for the present iteration will be used to update the 
   * runtime statistics and, if active, the raw timing measurements.
   */
  void finishIteration();

  std::size_t measurementSamplesInternal() const
  {
    return mMeasurementSampleCounter;
  }

  std::size_t statisticsSamplesInternal() const
  {
    return mStatisticsSampleCounter;
  }


  MeasurementVector const & currentData() const;

  MeasurementVector & currentData();

  /**
   * Internal method to calculate the variance based
   * @param [out] ret Return buffer, must have size \p numberOfComponents().
   * This is not checked here.
   * @note This method does is not secure against simultaneaos data 
   * accesses from different threads. This has to be ensured by the 
   * calling function.
   */
  void calculateVariance( MeasurementVector & ret ) const;
  
  AudioSignalFlow const & mFlow;
  
  std::size_t const cNumberOfAtomicComponents;
  
  std::size_t mMeasurementSampleCounter;
  
  std::size_t mStatisticsSampleCounter;
  
  efl::BasicVector<TimeType> mCurrentData;
  
  efl::BasicMatrix<TimeType> mCurrentMeasurementBuffer;

  efl::BasicMatrix<TimeType> mBackMeasurementBuffer;

  efl::BasicVector<TimeType> mRunningMean;
  
  /**
   * Quantity used internally to compute the variance using Welford's algorithm.
   */
  efl::BasicVector<TimeType> mRunningM2;
  
  efl::BasicVector<TimeType> mTmpBuffer;
  efl::BasicVector<TimeType> mTmpBuffer2;
};

} // namespace rrl
} // namespace visr

#endif // #ifndef VISR_LIBRRL_RUNTIME_PROFILER_HPP_INCLUDED
