/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "reverb_object_renderer.hpp"

#include <libvisr/signal_flow_context.hpp>

#include <libefl/db_linear_conversion.hpp>
#include <libefl/basic_matrix.hpp>
#include <libefl/vector_functions.hpp>

#include <libpanning/LoudspeakerArray.h>

#include <libpml/empty_parameter_config.hpp>

#include <librbbl/fft_wrapper_factory.hpp>
#include <librbbl/fft_wrapper_base.hpp>

#include <librcl/biquad_iir_filter.hpp>
#include <librcl/fir_filter_matrix.hpp>
#include <librcl/crossfading_fir_filter_matrix.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real_distribution.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

// If active, the matrix of generated late reverb decorrelation filters is
// written into a text file in working directory of the application.
// #define DEBUG_DECORR_FILTER_GENERATION

#ifdef DEBUG_DECORR_FILTER_GENERATION
#include <iterator>
#include <fstream>
#endif

#include <algorithm>
#include <cmath>

namespace visr
{
namespace reverbobject
{

namespace // unnamed
{
  /**
   * Create a routing that routes \p numChannels input signals to a single output (adding them).
   * @param numChannels The number of input signals, which are translated to the indices 0..numChanels-1
   * @todo Consider to make this an utility function (e.g., in librbbl)
   */
  rbbl::FilterRoutingList allToOneRouting( std::size_t numChannels )
  {
    rbbl::FilterRoutingList res;
    for (std::size_t chIdx(0); chIdx < numChannels; ++chIdx)
    {
      res.addRouting(chIdx, 0, chIdx, 1.0f);
    }
    return res;
  }

  /**
   * Create a routing that routes a single input to \p numchannels output signals, filtering each with an individual filter.
   * @param numChannels The number of output signals, which are translated to the indices 0..numChannels-1
   * @todo Consider to make this an utility function (e.g., in librbbl)
   */
  rbbl::FilterRoutingList oneToAllRouting(std::size_t numChannels)
  {
    rbbl::FilterRoutingList res;
    for (std::size_t chIdx(0); chIdx < numChannels; ++chIdx)
    {
      res.addRouting(0, chIdx, chIdx, 1.0f);
    }
    return res;
  }

  /**
   * Create a set of random-phase allpass filters. The frequency response is unity at the frequency grid points, but the phase is unformly
   * distributed in [-pi,pi].
   * @param filters The matrix to be filled. Must be set to the desired dimensions beforehand.
   * @param decorrelatorGain Scaling factor (linear scale) for the filter gain. 
   * @todo Consider promoting this into a general-purpose library function (rbbl or efl).
   */
  template<typename DataType>
  void generateRandomPhaseAllpass(efl::BasicMatrix<DataType> & filters, DataType decorrelatorGain )
  {
    std::size_t const numFilters{ filters.numberOfRows() };
    std::size_t const filterLength{ filters.numberOfColumns() };
    std::size_t const freqResponseSize{ filterLength / 2 + 1 }; // Frequency

    std::vector<std::complex<DataType> > freqResponse(freqResponseSize);
    // 0-Hz and fs/2 frequency bins must be real-valued for a real-valued
    freqResponse.front() = std::complex<DataType>{ 1.0f, 0.0 };
    freqResponse.back() = std::complex<DataType>{ 1.0f, 0.0 };

    std::unique_ptr<rbbl::FftWrapperBase<DataType> >  fft
      = rbbl::FftWrapperFactory<DataType>::create("default", filterLength, 0 /*no alignment specified*/);
    DataType const scaleFactor{ decorrelatorGain / (static_cast<DataType>(filterLength) * fft->inverseScalingFactor()) };

    boost::random::mt19937 gen;
    boost::random::uniform_real_distribution<DataType> uniformPhaseGen{ -boost::math::constants::pi<DataType>(), boost::math::constants::pi<DataType>() };

#ifdef DEBUG_DECORR_FILTER_GENERATION
    std::stringstream fileName;
    fileName << "late_decorrelation_filters.dat";
    std::ofstream out(fileName.str(), std::ios_base::out);
#endif

    for (std::size_t filterIdx{ 0 }; filterIdx < numFilters; ++filterIdx)
    {
      std::generate( freqResponse.begin()+1, freqResponse.end()-1,
        [&]()
        {
          DataType const ang{ uniformPhaseGen(gen) };
          std::complex<DataType> const val{ std::cos(ang), std::sin(ang) };
          return val;
        }
      );
      fft->inverseTransform( &freqResponse[0], filters.row(filterIdx));
      efl::ErrorCode const res = efl::vectorMultiplyConstantInplace<DataType>( scaleFactor, filters.row(filterIdx), filterLength, 0/*no alignment*/);
      if (res != efl::noError)
      {
        throw std::runtime_error(detail::composeMessageString("Error creating an random-phase allpass:", efl::errorMessage(res) ) );
      }

#ifdef DEBUG_DECORR_FILTER_GENERATION
      std::copy(filters.row(filterIdx), filters.row(filterIdx) + filterLength, std::ostream_iterator<DataType>(out, " "));
      out << "\n";
#endif
    }
  }

} // unnamed namespace

ReverbObjectRenderer::ReverbObjectRenderer( SignalFlowContext const & context,
                                      char const * name,
                                      CompositeComponent * parent,
                                      std::string const & reverbConfig,
                                      panning::LoudspeakerArray const & arrayConfig, 
                                      std::size_t numberOfObjectSignals )
  : CompositeComponent( context, name, parent )
  , mObjectSignalInput( "in", *this, numberOfObjectSignals )
  , mLoudspeakerOutput( "out", *this, arrayConfig.getNumRegularSpeakers() )
  , mObjectVector( "objectIn", *this, pml::EmptyParameterConfig() )
  , mReverbParameterCalculator( context, "parameterCalculator", this )
  , mReverbSignalRouting( context, "signalRouting", this )
  , mDiscreteReverbDelay( context, "discreteReverbDelay", this )
  , mDiscreteReverbPanningMatrix( context, "discretePanningMatrix", this )
  , mLateReverbFilterCalculator()
  , mLateReverbGainDelay( context, "lateReverbGainDelay", this )
  , mReverbMix( context, "Sum", this, arrayConfig.getNumRegularSpeakers(), 2 )
{
  // Parse and apply default values for the reverb parameters.
  std::stringstream stream( reverbConfig );
  boost::property_tree::ptree tree;
  try
  {
    read_json( stream, tree );
  }
  catch( std::exception const & ex )
  {
    throw std::invalid_argument( std::string( "Error while parsing reverb config string: " ) + ex.what() );
  }

  std::size_t const maxNumReverbObjects = tree.get<std::size_t>( "numReverbObjects", 0 );
  SampleType const lateReverbFilterLengthSeconds = tree.get<SampleType>( "lateReverbFilterLength", 0 );
  // Enforce a minimum late reverb filter length of 1 sample. This is currently needed as not all components are safe when used with a filter length of 0.
  std::size_t const lateReverbFilterLengthSamples = std::max( static_cast<std::size_t>(std::ceil( lateReverbFilterLengthSeconds * samplingFrequency() )),
    static_cast<std::size_t>(1) );

  std::string const lateReverbDecorrFilterName( tree.get<std::string>( "lateReverbDecorrelationFilters", std::string() ));
  std::size_t const numDiscreteReflectionsPerObject = tree.get<std::size_t>( "discreteReflectionsPerObject", 0 );

  std::size_t const numWallReflBiquads = objectmodel::PointSourceWithReverb::cNumDiscreteReflectionBiquads;

  SampleType const maxDiscreteReflectionDelay = tree.get<SampleType>( "maxDiscreteReflectionDelay",  1.0f );

  SampleType const lateReverbCrossfadeTime = tree.get<SampleType>("lateReverbCrossfadeTime", 2.0f); // Default value
  std::size_t const lateReverbCrossfadeSamples{ static_cast<std::size_t>(std::max(0.0f, std::ceil(lateReverbCrossfadeTime*context.samplingFrequency()) )) };

  // The maximum number of late filter recalculations per period.
  std::size_t const cLateFilterUpdatesPerPeriod( tree.get<std::size_t>( "lateReverbFilterUpdatesPerPeriod", 1 ));

  // Optional argument for the gain of the decorrelation filter for
  // the late reverb tail.
  // The default value is 1/sqrt(#loudspeakers)
  boost::optional<SampleType> const lateReverbDecorrelatorGainOpt = tree.get_optional<SampleType>( "lateReverbDecorrelationGain" );
  SampleType const defaultLateDecorrelatorGain = 1.0f / std::sqrt( static_cast<float>(arrayConfig.getNumRegularSpeakers()) );
  SampleType const lateReverbDecorrelatorGain = lateReverbDecorrelatorGainOpt
    ? efl::dB2linear( *lateReverbDecorrelatorGainOpt ) : defaultLateDecorrelatorGain;

  pml::MatrixParameter<SampleType> lateDecorrelationFilters(cVectorAlignmentSamples );
    if( lateReverbDecorrFilterName.empty() )
    {
      lateDecorrelationFilters.resize( arrayConfig.getNumRegularSpeakers(), 512 ); // default filter length
      generateRandomPhaseAllpass( lateDecorrelationFilters, lateReverbDecorrelatorGain );
    }
    else
    {
      boost::filesystem::path const lateReverbDecorrFilterPath( lateReverbDecorrFilterName );
      if( not exists( lateReverbDecorrFilterPath ) )
      {
        throw std::invalid_argument( "The file path \"lateReverbDecorrelationFilters\" provided in the reverb configuration does not exist." );
      }
#ifdef VISR_PML_USE_SNDFILE_LIBRARY
      pml::MatrixParameter<SampleType> const allLateDecorrelationFilters = pml::MatrixParameter<SampleType>::fromAudioFile( lateReverbDecorrFilterName, cVectorAlignmentSamples );
      std::size_t const lateDecorrelationFilterLength = allLateDecorrelationFilters.numberOfColumns();
      if( allLateDecorrelationFilters.numberOfRows() < arrayConfig.getNumRegularSpeakers() )
      {
        throw std::invalid_argument( "The number of loudspeakers exceeds the number of late reverberation decorrelation filters in the provided file." );
      }
      lateDecorrelationFilters.resize( arrayConfig.getNumRegularSpeakers(), lateDecorrelationFilterLength );
      for( std::size_t rowIdx( 0 ); rowIdx < arrayConfig.getNumRegularSpeakers(); ++rowIdx )
      {
        // Multiply the raw unit-magnitude filters by the scaling gain.
        if( efl::vectorMultiplyConstant( lateReverbDecorrelatorGain,
                                        allLateDecorrelationFilters.row( rowIdx ),
                                        lateDecorrelationFilters.row( rowIdx ),
                                        lateDecorrelationFilterLength,
                                        cVectorAlignmentSamples ) != efl::noError )
        {
          throw std::runtime_error( "Copying and scaling of late decorrelation filter rows failed." );
        }
      }
#else
      throw std::invalid_argument( "To load a late reverb decorrelation filter from an audio file, VISR must be built with the BUILD_USE_SNDFILE_LIBRARY option." );
#endif
    }

    std::size_t const interpolationSteps = period();

    // set up the components
    mReverbSignalRouting.setup( numberOfObjectSignals, maxNumReverbObjects, true /*contronInputs*/ );

    mReverbParameterCalculator.setup( arrayConfig, maxNumReverbObjects,
                                      numDiscreteReflectionsPerObject,
                                      numWallReflBiquads,
                                      lateReverbFilterLengthSeconds,
                                      objectmodel::PointSourceWithReverb::cNumberOfSubBands );
    mDiscreteReverbDelay.setup( maxNumReverbObjects*numDiscreteReflectionsPerObject,
                                interpolationSteps, maxDiscreteReflectionDelay, "lagrangeOrder3",
                                rcl::DelayVector::MethodDelayPolicy::Limit,
                                rcl::DelayVector::ControlPortConfig::All, 0.0f, 0.0f );
    mDiscreteReverbReflFilters.reset( new rcl::BiquadIirFilter( context, "discreteReverbReflectionFilters", this,
                                      maxNumReverbObjects*numDiscreteReflectionsPerObject, numWallReflBiquads, true /*controlInputs*/ ) );
    mDiscreteReverbPanningMatrix.setup( maxNumReverbObjects*numDiscreteReflectionsPerObject,
                                        arrayConfig.getNumRegularSpeakers(),
                                        interpolationSteps );
    mLateReverbFilterCalculator.reset( new LateReverbFilterCalculator( context, "lateReverbCalculator", this,
                                       maxNumReverbObjects, lateReverbFilterLengthSeconds,
                                       objectmodel::PointSourceWithReverb::cNumberOfSubBands,
                                       cLateFilterUpdatesPerPeriod ) );

  // TODO: Add configuration parameters for maximum delay of the late reverb onset delay.
  mLateReverbGainDelay.setup( maxNumReverbObjects,
                              period(),
                              maxDiscreteReflectionDelay, // For the moment, use the same max. delay as for discretes.
                              "lagrangeOrder3",
                              rcl::DelayVector::MethodDelayPolicy::Limit,
                              rcl::DelayVector::ControlPortConfig::All,
                              0.0f, 0.0f );

  // A single late reverb filter for each object, which are routed (summed) to a single output. 
  if( lateReverbCrossfadeSamples > 0 )
  {
    mLateReverbFilter.reset(new rcl::CrossfadingFirFilterMatrix(context,
                             "lateReverbFilter",
                              this,
                              maxNumReverbObjects,
                              1,
                              lateReverbFilterLengthSamples,
                              maxNumReverbObjects,
                              maxNumReverbObjects,
                              lateReverbCrossfadeSamples,
                              efl::BasicMatrix<SampleType>(), // No initial filters provided.
                              allToOneRouting(maxNumReverbObjects),
                              rcl::CrossfadingFirFilterMatrix::ControlPortConfig::Filters, "default"
                             ));
  }
  else
  {
    mLateReverbFilter.reset(new rcl::FirFilterMatrix(context,
                             "lateReverbFilter",
                             this,
                             maxNumReverbObjects,
                             1,
                             lateReverbFilterLengthSamples,
                             maxNumReverbObjects,
                             maxNumReverbObjects,
                            efl::BasicMatrix<SampleType>(), // No initial filters provided.
                            allToOneRouting(maxNumReverbObjects),
                            rcl::FirFilterMatrix::ControlPortConfig::Filters, "default"
    ));
  }

  // Create #loudspeakers decorrelated signals from a single, each filtered with an individual decorrelation filter
  mLateDiffusionFilter.reset(new rcl::FirFilterMatrix(context, "decorrelationFilter", this, 
                             1, arrayConfig.getNumRegularSpeakers( ), lateDecorrelationFilters.numberOfColumns(),
                             arrayConfig.getNumRegularSpeakers( ), arrayConfig.getNumRegularSpeakers( ),
                             lateDecorrelationFilters, oneToAllRouting( arrayConfig.getNumRegularSpeakers()) ) );

  audioConnection( mObjectSignalInput, mReverbSignalRouting.audioPort("in") );
  std::size_t const totalDiscreteReflections = maxNumReverbObjects*numDiscreteReflectionsPerObject;
  std::vector<ChannelList::IndexType> discreteFanOut( totalDiscreteReflections );
  // Construct a sequence of indices [0 0 .. 0 1 1 ..1 M-1 M-1 .. M-1] with M = maxNumReverbObjects and
  // numDiscreteReflectionsPerObject repetitions each.
  for( std::size_t chIdx(0); chIdx < totalDiscreteReflections; ++chIdx )
  {
    discreteFanOut[chIdx] = chIdx / numDiscreteReflectionsPerObject;
  }
  audioConnection( mReverbSignalRouting.audioPort("out"), ChannelList(discreteFanOut),
                   mDiscreteReverbDelay.audioPort("in"), ChannelRange(0,totalDiscreteReflections) );
  audioConnection( mDiscreteReverbDelay.audioPort("out"), mDiscreteReverbReflFilters->audioPort("in") );
  audioConnection( mDiscreteReverbReflFilters->audioPort("out"), mDiscreteReverbPanningMatrix.audioPort("in") );
  audioConnection( mDiscreteReverbPanningMatrix.audioPort("out"), mReverbMix.audioPort("in0") );

  audioConnection( mReverbSignalRouting.audioPort("out"), mLateReverbGainDelay.audioPort("in") );
  audioConnection( mLateReverbGainDelay.audioPort("out"), mLateReverbFilter->audioPort("in") );
  audioConnection( mLateReverbFilter->audioPort("out"), mLateDiffusionFilter->audioPort("in") );
  audioConnection( mLateDiffusionFilter->audioPort("out"), mReverbMix.audioPort("in1") );

  audioConnection( mReverbMix.audioPort("out"), mLoudspeakerOutput );

  parameterConnection( mObjectVector, mReverbParameterCalculator.parameterPort("objectInput") );
  parameterConnection( mReverbParameterCalculator.parameterPort("signalRoutingOut"), mReverbSignalRouting.parameterPort("controlInput") );
  parameterConnection( mReverbParameterCalculator.parameterPort("discreteGainOut"), mDiscreteReverbDelay.parameterPort( "gainInput" ) );
  parameterConnection( mReverbParameterCalculator.parameterPort("discreteDelayOut"), mDiscreteReverbDelay.parameterPort( "delayInput" ) );
  parameterConnection( mReverbParameterCalculator.parameterPort("discreteEqOut"), mDiscreteReverbReflFilters->parameterPort( "eqInput" ) );
  parameterConnection( mReverbParameterCalculator.parameterPort("discretePanningGainOut"), mDiscreteReverbPanningMatrix.parameterPort( "gainInput" ) );
  parameterConnection( mReverbParameterCalculator.parameterPort("lateGainOut"), mLateReverbGainDelay.parameterPort( "gainInput" ) );
  parameterConnection( mReverbParameterCalculator.parameterPort("lateDelayOut"), mLateReverbGainDelay.parameterPort( "delayInput" ) );

  parameterConnection( mReverbParameterCalculator.parameterPort("lateSubbandOut"), mLateReverbFilterCalculator->parameterPort("subbandInput") );
  parameterConnection( mLateReverbFilterCalculator->parameterPort("lateFilterOutput"), mLateReverbFilter->parameterPort("filterInput") );

}

ReverbObjectRenderer::~ReverbObjectRenderer() = default;

} // namespace reverbobject
} // namespace visr
