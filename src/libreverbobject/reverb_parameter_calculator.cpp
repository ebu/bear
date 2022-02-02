/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "reverb_parameter_calculator.hpp"

#include <libefl/basic_matrix.hpp>
#include <libefl/vector_functions.hpp>

#include <libobjectmodel/object_vector.hpp>
#include <libobjectmodel/point_source_with_reverb.hpp>

#include <librbbl/object_channel_allocator.hpp>

#include <algorithm>
#include <cassert>
#include <ciso646>
#include <cmath>
#include <limits>
#include <numeric>
#include <vector>

namespace visr
{
namespace reverbobject
{


namespace
{

/**
 * Local helper function to find the maximum difference between corresponding elements of
 * two sequences of equal length
 */
SampleType maxDiff( objectmodel::PointSourceWithReverb::LateReverbCoeffs const & lhs,
                         objectmodel::PointSourceWithReverb::LateReverbCoeffs const & rhs )
{
  SampleType const res = std::inner_product(lhs.begin(), lhs.end(), rhs.begin(), 0.0f,
                            [](SampleType v1, SampleType v2) { return std::max( v1, v2 ); },
                            [](SampleType v1, SampleType v2) { return std::abs( v1 - v2 ); } );
  return res;
}

/**
 * Local helper function to compare two LateReverb objects up to a given tolerance value.
 */
bool equal( objectmodel::PointSourceWithReverb::LateReverb const & lhs,
            objectmodel::PointSourceWithReverb::LateReverb const & rhs,
            SampleType limit )
{
  return (std::abs( lhs.onsetDelay() - rhs.onsetDelay() ) <= limit )
   and (maxDiff( lhs.levels(), rhs.levels()) <= limit )
   and (maxDiff( lhs.decayCoeffs(), rhs.decayCoeffs()) <= limit )
   and (maxDiff( lhs.attackTimes(), rhs.attackTimes()) <= limit );
}

} // unnamed namespace

/*static*/ const objectmodel::PointSourceWithReverb::LateReverb
ReverbParameterCalculator::cDefaultLateReverbParameter( 0.0, {0.0f}, {0.0f}, { 0.0f });

  /**
   * A table holding the previous states of the reverb parameters for the reverb channel.
   * Used to detect changes in the that trigger an retransmission to the LateReverbFilterCalculator component.
   */
  std::vector<objectmodel::PointSourceWithReverb::LateReverb> mPreviousLateReverbs;


  ReverbParameterCalculator::ReverbParameterCalculator( SignalFlowContext const & context,
                                                        char const * name,
                                                        CompositeComponent * parent /*= nullptr*/ )
 : AtomicComponent( context, name, parent )
 , mMaxNumberOfObjects( 0 )
 , cLateReverbParameterComparisonLimit( std::numeric_limits<SampleType>::epsilon( ) )
 , mObjectInput( "objectInput", *this, pml::EmptyParameterConfig() )
 , mSignalRoutingOutput( "signalRoutingOut", *this, pml::EmptyParameterConfig() )
 , mDiscreteReflectionGainOutput( "discreteGainOut", *this )
 , mDiscreteReflectionDelayOutput( "discreteDelayOut", *this )
 , mDiscreteReflectionFilterCoeffOutput( "discreteEqOut", *this )
 , mDiscretePanningGains( "discretePanningGainOut", *this )
 , mLateReflectionGainOutput( "lateGainOut", *this )
 , mLateReflectionDelayOutput( "lateDelayOut", *this )
 , mLateSubbandOutput( "lateSubbandOut", *this )
 , mTmpPanningGains( cVectorAlignmentSamples )
{
}

ReverbParameterCalculator::~ReverbParameterCalculator()
{
}

void ReverbParameterCalculator::setup( panning::LoudspeakerArray const & arrayConfig,
                                       std::size_t numberOfObjects,
                                       std::size_t numberOfDiscreteReflectionsPerSource,
                                       std::size_t numBiquadSectionsReflectionFilters,
                                       SampleType lateReflectionLengthSeconds,
                                       std::size_t numLateReflectionSubBandFilters )
{
  mMaxNumberOfObjects=numberOfObjects;
  mNumberOfDiscreteReflectionsPerSource = numberOfDiscreteReflectionsPerSource;
  mNumberOfBiquadSectionsReflectionFilters = numBiquadSectionsReflectionFilters;
  mLateReflectionLengthSeconds = lateReflectionLengthSeconds;
  mNumberOfLateReflectionSubBandFilters = numLateReflectionSubBandFilters;

  if( mNumberOfBiquadSectionsReflectionFilters != objectmodel::PointSourceWithReverb::cNumDiscreteReflectionBiquads )
  {
    throw std::invalid_argument( "The number of biquad sections for the discrete reflections differs from the constant used in the object definition." );
  }

  mChannelAllocator.reset( new rbbl::ObjectChannelAllocator( mMaxNumberOfObjects ) );

  // Configure the VBAP calculator
  mNumberOfPanningLoudspeakers = arrayConfig.getNumRegularSpeakers();

  mVbapCalculator.reset( new panning::VBAP( arrayConfig ) );
  mTmpPanningGains.resize( mNumberOfPanningLoudspeakers );

  mPreviousLateReverbs.resize( mMaxNumberOfObjects, cDefaultLateReverbParameter );
  // Set one member to an invalid value to trigger sending of late reverb messages on the first call to process();
  std::for_each(mPreviousLateReverbs.begin(), mPreviousLateReverbs.end(), []( objectmodel::PointSourceWithReverb::LateReverb & v ) { v.setOnsetDelay( -1.0f); } );

  // Set up the parameter ports.
  pml::VectorParameterConfig const discreteGainDelayConfig( mNumberOfDiscreteReflectionsPerSource * mMaxNumberOfObjects );
  pml::VectorParameterConfig const lateGainDelayConfig( mMaxNumberOfObjects );
  pml::MatrixParameterConfig const iirMatrixConfig( mNumberOfDiscreteReflectionsPerSource * mMaxNumberOfObjects, objectmodel::PointSourceWithReverb::cNumDiscreteReflectionBiquads );
  pml::MatrixParameterConfig const discretePanningConfig( mNumberOfPanningLoudspeakers, mNumberOfDiscreteReflectionsPerSource * mMaxNumberOfObjects );
  mSignalRoutingOutput.setParameterConfig( pml::EmptyParameterConfig() );
  mDiscreteReflectionGainOutput.setParameterConfig( discreteGainDelayConfig );
  mDiscreteReflectionDelayOutput.setParameterConfig( discreteGainDelayConfig );
  mDiscreteReflectionFilterCoeffOutput.setParameterConfig( iirMatrixConfig );
  mDiscretePanningGains.setParameterConfig( discretePanningConfig );
  mLateReflectionGainOutput.setParameterConfig( lateGainDelayConfig );
  mLateReflectionDelayOutput.setParameterConfig( lateGainDelayConfig );
  mLateSubbandOutput.setParameterConfig( pml::EmptyParameterConfig() );
}

/**
* The process function.
* It takes a vector of objects as input and calculates a vector of output gains.
*/
void ReverbParameterCalculator::process()
{
  // Get and check the data members.
  pml::SignalRoutingParameter & signalRouting = mSignalRoutingOutput.data();
  efl::BasicVector<SampleType> & discreteReflGains = mDiscreteReflectionGainOutput.data();
  efl::BasicVector<SampleType> & discreteReflDelays = mDiscreteReflectionDelayOutput.data();
  pml::BiquadParameterMatrix<SampleType> & biquadCoeffs = mDiscreteReflectionFilterCoeffOutput.data();
  efl::BasicMatrix<SampleType> & discretePanningMatrix = mDiscretePanningGains.data();
  efl::BasicVector<SampleType> & lateReverbGains = mLateReflectionGainOutput.data();
  efl::BasicVector<SampleType> & lateReverbDelays = mLateReflectionDelayOutput.data();

  std::vector<objectmodel::ObjectId> foundReverbObjects;
  pml::ObjectVector const & objects = mObjectInput.data();
  for( objectmodel::Object const & obj : objects )
  {
    objectmodel::ObjectTypeId const ti = obj.type();
    // Process reverb objects, ignore others
    switch( ti )
    {
      case objectmodel::ObjectTypeId::PointSourceWithReverb:
        // This check should be done elsewhere (e.g., parsing of point sources).
        if( obj.numberOfChannels() != 1 )
        {
          status( StatusMessage::Error, "ReverbParameterCalculator: reverb objects must be single-channel." );
          continue;
        }
        foundReverbObjects.push_back( obj.id() );
        break;
      default: // Default handling to prevent compiler warnings about unhandled enumeration values
      {
        // Do nothing
      }
    }
  } //for( objectmodel::ObjectVector::value_type const & objEntry : objects )

  // Check the list of found reverb objects against the existing entries.
  if( foundReverbObjects.size() > mMaxNumberOfObjects )
  {
    throw std::runtime_error( "The number of reverb objects exceeds the maximum admissible number." );
  }
  mChannelAllocator->setObjects( foundReverbObjects );
  for( std::size_t chIdx( 0 ); chIdx < mMaxNumberOfObjects; ++chIdx )
  {
    objectmodel::ObjectId const objId = mChannelAllocator->getObjectForChannel( chIdx );
    if( objId == objectmodel::Object::cInvalidChannelIndex )
    {
      signalRouting.removeEntry( chIdx );
      clearSingleObject( chIdx, discreteReflGains, discreteReflDelays, biquadCoeffs, discretePanningMatrix,
                         lateReverbGains, lateReverbDelays );
    }
    else
    {
      objectmodel::PointSourceWithReverb const & rsao = dynamic_cast<objectmodel::PointSourceWithReverb const &>(objects.at( objId ));
      signalRouting.addRouting( rsao.channelIndex(0), chIdx ); // This clear existing routings to this rendering channel.
      processSingleObject( rsao, chIdx, discreteReflGains, discreteReflDelays, biquadCoeffs, discretePanningMatrix, 
                           lateReverbGains, lateReverbDelays);
    }
  }
  mSignalRoutingOutput.swapBuffers();
  mDiscreteReflectionGainOutput.swapBuffers();
  mDiscreteReflectionDelayOutput.swapBuffers();
  mDiscreteReflectionFilterCoeffOutput.swapBuffers();
  // No swapBuffers() mDiscretePanningGains, because this port uses the pml::SharedDataProtocol.
  mLateReflectionGainOutput.swapBuffers();
  mLateReflectionDelayOutput.swapBuffers();
} //process

void ReverbParameterCalculator::processSingleObject( objectmodel::PointSourceWithReverb const & rsao,
                                                     std::size_t renderChannel,
                                                     efl::BasicVector<SampleType> & discreteReflGains,
                                                     efl::BasicVector<SampleType> & discreteReflDelays,
                                                     pml::BiquadParameterMatrix<SampleType> & biquadCoeffs,
                                                     efl::BasicMatrix<SampleType> & discretePanningMatrix,
                                                     efl::BasicVector<SampleType> & lateReverbGains,
                                                     efl::BasicVector<SampleType> & lateReverbDelays )
{
  // TODO: Assign the biquad coefficients for the direct path.

  if( rsao.numberOfDiscreteReflections() > mNumberOfDiscreteReflectionsPerSource )
  {
    throw std::invalid_argument( "ReverbParameterCalculator::process(): Number of discrete reflections exceed maximum." );
  }
 
  // Define a starting index into the all parameter matrices 
  std::size_t const startIdx = renderChannel * mNumberOfDiscreteReflectionsPerSource;

  // Assign the positions for the discrete reflections.
  for( std::size_t srcIdx( 0 ); srcIdx < rsao.numberOfDiscreteReflections( ); ++srcIdx )
  {
    objectmodel::PointSourceWithReverb::DiscreteReflection const & discRefl = rsao.discreteReflection( srcIdx );
    std::size_t const matrixIdx = startIdx + srcIdx;

    mVbapCalculator->calculateGains( discRefl.positionX(), discRefl.positionY(), discRefl.positionZ(), mTmpPanningGains.data(),
                                     false /*planeWave=false means discrete reflections are considered as point sources.*/ );
    efl::ErrorCode const res = efl::vectorCopyStrided( mTmpPanningGains.data(), &discretePanningMatrix(0, matrixIdx ), 1, 
      discretePanningMatrix.stride(), mNumberOfPanningLoudspeakers, 0 /*no assumptions on stride possible*/ );
    if( res != efl::noError )
    {
      status( StatusMessage::Error, "Error while assigning discrete panning gains: ", efl::errorMessage( res ) );
      return;
    }

    // Set the biquad filters
    for( std::size_t biquadIdx( 0 ); biquadIdx < mNumberOfBiquadSectionsReflectionFilters; ++biquadIdx )
    {
      biquadCoeffs( matrixIdx, biquadIdx ) = discRefl.reflectionFilter( biquadIdx );
    }

    // Set the gain and delay for each discrete reflection
    discreteReflGains[matrixIdx] = discRefl.level();
    discreteReflDelays[matrixIdx] = discRefl.delay();
  }

  // Fill the remaining discrete reflections with neutral values.
  static const rbbl::BiquadCoefficient<SampleType> defaultBiquad; // Neutral flat biquad (default constructed)
  for( std::size_t srcIdx( rsao.numberOfDiscreteReflections() ); srcIdx < mNumberOfDiscreteReflectionsPerSource; ++srcIdx )
  {
    std::size_t const matrixIdx = startIdx + srcIdx;
    for( std::size_t biquadIdx( 0 ); biquadIdx < mNumberOfBiquadSectionsReflectionFilters; ++biquadIdx )
    {
      biquadCoeffs( matrixIdx, biquadIdx ) = defaultBiquad;
    }
    discreteReflGains[matrixIdx] = 0.0f;
    discreteReflDelays[matrixIdx] = 0.0f;

    // Set the panning gains for unused reflections to zero.
    for( std::size_t lspIdx(0 ); lspIdx < mNumberOfPanningLoudspeakers; ++lspIdx )
    {
      discretePanningMatrix( lspIdx, matrixIdx ) = 0.0f;
    }
  }
  // The unused rows are already zeroed.

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Late reflection part.

  // TODO: As of now, both the gain and dely are unused. If it remains this way, 
  // consider to remove the 'LateReverbGainDelay' component and associated
  // parameter connections from the reverb renderer signal flow.
  lateReverbDelays[renderChannel] = 0.0f; // Unused for now.  We might apply the frequency-independent offset delay here.
  lateReverbGains[renderChannel] = 1.0f; // The object level is already applied
  // to the audio signal received in the reverb renderer.
  if( not equal( mPreviousLateReverbs[renderChannel], rsao.lateReverb(), cLateReverbParameterComparisonLimit ))
  {
    mPreviousLateReverbs[renderChannel] = rsao.lateReverb();
    mLateSubbandOutput.enqueue( LateReverbParameter( renderChannel, mPreviousLateReverbs[renderChannel] ) );
  }
}

void ReverbParameterCalculator::clearSingleObject( std::size_t renderChannel,
                                                   efl::BasicVector<SampleType> & discreteReflGains,
                                                   efl::BasicVector<SampleType> & discreteReflDelays,
                                                   pml::BiquadParameterMatrix<SampleType> & biquadCoeffs,
                                                   efl::BasicMatrix<SampleType> & discretePanningMatrix,
                                                   efl::BasicVector<SampleType> & lateReverbGains,
                                                   efl::BasicVector<SampleType> & lateReverbDelays )
{
  // Define a starting index into the all parameter matrices 
  std::size_t const startRow = renderChannel * mNumberOfDiscreteReflectionsPerSource;

  static const panning::XYZ defaultPosition( 1.0f, 0.0f, 0.0f );
  static const rbbl::BiquadCoefficient<SampleType> defaultBiquad; // Neutral flat biquad (default constructed)
  for( std::size_t srcIdx( 0 ); srcIdx < mNumberOfDiscreteReflectionsPerSource; ++srcIdx )
  {
    std::size_t const matrixRow = startRow + srcIdx;
    efl::ErrorCode const res = efl::vectorFillStrided( static_cast<SampleType>(0.0f), &discretePanningMatrix( 0, matrixRow ),
      discretePanningMatrix.stride(), mNumberOfPanningLoudspeakers, 0 /*no assumptions on stride possible*/ );
    if( res != efl::noError )
    {
      status( StatusMessage::Error, "Error while zeroing discrete panning gains for unused discrete reflection: ", efl::errorMessage( res ) );
      return;
    }

    for( std::size_t biquadIdx( 0 ); biquadIdx < mNumberOfBiquadSectionsReflectionFilters; ++biquadIdx )
    {
      biquadCoeffs( matrixRow, biquadIdx ) = defaultBiquad;
    }
    discreteReflGains[matrixRow] = 0.0f;
    discreteReflDelays[matrixRow] = 0.0f;
  }
  // Late reflection part
  lateReverbDelays[renderChannel] = 0.0f;
  lateReverbGains[renderChannel] = 0.0f;  

  if( not equal( mPreviousLateReverbs[renderChannel], cDefaultLateReverbParameter, cLateReverbParameterComparisonLimit ))
  {
    mPreviousLateReverbs[renderChannel] = cDefaultLateReverbParameter;
    mLateSubbandOutput.enqueue( LateReverbParameter( renderChannel, mPreviousLateReverbs[renderChannel] ) );
  }

}

} // namespace reverbobject
} // namespace visr
