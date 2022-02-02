 /* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "panning_calculator.hpp"

#include <libvisr/status_message.hpp>

#include <libefl/basic_matrix.hpp>
#include <libefl/cartesian_spherical_conversion.hpp>
#include <libefl/degree_radian_conversion.hpp>
#include <libefl/matrix_functions.hpp>
#include <libefl/vector_functions.hpp>

#include <libobjectmodel/object_vector.hpp>
#include <libobjectmodel/channel_object.hpp>
#include <libobjectmodel/diffuse_source.hpp>
#include <libobjectmodel/point_source.hpp>
#include <libobjectmodel/point_source_with_diffuseness.hpp>
#include <libobjectmodel/plane_wave.hpp>

#include <libpanning/LoudspeakerArray.h>

#include <libpml/listener_position.hpp>
#include <libpml/shared_data_protocol.hpp>
#include <libpml/matrix_parameter.hpp>
#include <libpml/object_vector.hpp>
#include <libpml/double_buffering_protocol.hpp>


#include <algorithm>
#include <ciso646>
#include <cmath>
#include <cstdio>
#include <iterator>
#include <numeric>
#include <type_traits>
#include <valarray>

// for math utility functions (see implementations in the unnamed namespace below)

namespace visr
{
namespace rcl
{

PanningCalculator::PanningCalculator( SignalFlowContext const & context,
                                      char const * name,
                                      CompositeComponent * parent,
                                      std::size_t numberOfObjects,
                                      panning::LoudspeakerArray const & arrayConfig,
                                      bool adaptiveListenerPosition,
                                      PanningMode panningMode /*= PanningMode::LF*/,
                                      Normalisation lfNormalisation /*= Normalisation::Default*/,
                                      Normalisation hfNormalisation /*= Normalisation::Default*/,
                                      Normalisation diffuseNormalisation /*= Normalisation::Default*/,
                                      pml::ListenerPosition const & listenerPosition /*= pml::ListenerPosition( 0.0f, 0.0f, 0.0f )*/ )
 : AtomicComponent( context, name, parent )
 , mNumberOfObjects( numberOfObjects )
 , mNumberOfRegularLoudspeakers( arrayConfig.getNumRegularSpeakers() )
 , mNumberOfAllLoudspeakers( arrayConfig.getNumSpeakers() )
 , mVectorDimension( arrayConfig.is2D() ? 2 : 3 )
 , mLoudspeakerPositions( mVectorDimension, mNumberOfAllLoudspeakers )
 , mTmpGains( mNumberOfRegularLoudspeakers, cVectorAlignmentSamples )
 , mTmpHfGains( mNumberOfRegularLoudspeakers, cVectorAlignmentSamples )
 , mTmpDiffuseGains( mNumberOfRegularLoudspeakers, cVectorAlignmentSamples )
 , mLfNormalisation( (lfNormalisation == Normalisation::Default)
                     ? ((panningMode & PanningMode::HF) == PanningMode::Nothing ? Normalisation::Energy : Normalisation::Amplitude ) : lfNormalisation )
 , mHfNormalisation( (hfNormalisation == Normalisation::Default) ? Normalisation::Energy : hfNormalisation )
 , mDiffuseNormalisation( (diffuseNormalisation == Normalisation::Default) ? Normalisation::Energy : diffuseNormalisation )
 , mLabelLookup( fillLabelLookup( arrayConfig ) )
{
    //mVbapCalculator->setNearTripletBoundaryCosTheta(-0.7);

    // Initialise for all (regular and virtual) loudspeakers.
    for( std::size_t lspIdx( 0 ); lspIdx < mNumberOfAllLoudspeakers; ++lspIdx )
    {
      panning::XYZ pos = arrayConfig.getPosition( lspIdx );
      pos.normalise();
      mLoudspeakerPositions( 0, lspIdx ) = pos.x;
      mLoudspeakerPositions( 1, lspIdx ) = pos.y;
      if( mVectorDimension > 2 )
      {
        mLoudspeakerPositions( 2, lspIdx ) = pos.z;
      }
    }

    // Compute the triplet centers
    efl::BasicMatrix<CoefficientType> tripletCenters( mVectorDimension, arrayConfig.getNumTriplets() );
    std::vector<bool> tripletProcessed( arrayConfig.getNumTriplets(), false );

    mVbapCalculator.reset( new panning::VBAP( arrayConfig ) );

    // set the default initial listener position. This also initialises the internal data members (e.g. inverse matrices)
    setListenerPosition( listenerPosition );

    mObjectVectorInput.reset( new ObjectPort( "objectVectorInput", *this, pml::EmptyParameterConfig() ) );

    if( (PanningMode::LF & panningMode) != PanningMode::Nothing )
    {
      mLowFrequencyGainOutput.reset( new MatrixPort( "vbapGains", *this, pml::MatrixParameterConfig( mNumberOfRegularLoudspeakers, mNumberOfObjects ) ) );
    }
    if( (panningMode & PanningMode::HF) != PanningMode::Nothing )
    {
      mHighFrequencyGainOutput.reset( new MatrixPort( "vbipGains", *this, pml::MatrixParameterConfig( mNumberOfRegularLoudspeakers, mNumberOfObjects ) ) );
    }
    if( (panningMode & PanningMode::Diffuse) != PanningMode::Nothing )
    {
      mDiffuseGainOutput.reset( new MatrixPort( "diffuseGains", *this, pml::MatrixParameterConfig( mNumberOfRegularLoudspeakers, mNumberOfObjects ) ) );
    }

    if( adaptiveListenerPosition )
    {
      mListenerPositionInput.reset( new ListenerPositionPort( "listenerPosition", *this, pml::EmptyParameterConfig() ) );
    }
}

PanningCalculator::PanningCalculator( SignalFlowContext const & context,
   char const * name,
   CompositeComponent * parent,
   std::size_t numberOfObjects,
   panning::LoudspeakerArray const & arrayConfig,
   bool adaptiveListenerPosition /*= false*/,
   bool separateLowpassPanning /*= false*/ )
  : PanningCalculator( context, name, parent, numberOfObjects, arrayConfig,
                       adaptiveListenerPosition,
                       separateLowpassPanning ? PanningMode::Dualband : PanningMode::LF
                     ) // Remaining arguments use default values
{
}

PanningCalculator::~PanningCalculator() = default;

void PanningCalculator::setListenerPosition( CoefficientType x, CoefficientType y, CoefficientType z )
{
  mVbapCalculator->setListenerPosition( x, y, z );
}

void PanningCalculator::setListenerPosition( pml::ListenerPosition const & pos )
{
  setListenerPosition( pos.x(), pos.y(), pos.z() );
}




namespace // unnamed
{
  void normalise( SampleType const * in, SampleType * out, std::size_t numberOfElements,
                  PanningCalculator::Normalisation mode, std::size_t outputStride = 1,
                  SampleType targetLevel = 1.0f )
  {
    SampleType sigNorm;
    switch( mode )
    {
      case PanningCalculator::Normalisation::Amplitude:
        sigNorm = std::accumulate( in, in + numberOfElements, 0.0f,
          []( SampleType acc, SampleType val ) { return acc + std::abs( val ); } );
        break;
      case PanningCalculator::Normalisation::Energy:
       sigNorm = std::sqrt( std::accumulate( in, in + numberOfElements, 0.0f,
          []( SampleType acc, SampleType val ) { return acc + val * val; } ) );
       break;
      default:
        sigNorm = 1.0f;
    }
    SampleType const normFactor = targetLevel / std::max( sigNorm, std::numeric_limits<SampleType>::epsilon() );
    for( std::size_t sampleIdx(0); sampleIdx < numberOfElements; ++sampleIdx, ++in, out += outputStride )
    {
      *out = normFactor * *in;
    }
  }
}




void PanningCalculator::process()
{
  bool const listenerPosChanged = mListenerPositionInput && mListenerPositionInput->changed();
  if( listenerPosChanged )
  {
    setListenerPosition( mListenerPositionInput->data( ) );
    mListenerPositionInput->resetChanged( );
  }

  if( mObjectVectorInput->changed() )
  {
    objectmodel::ObjectVector const & objects = mObjectVectorInput->data( );
#if 1
    pml::MatrixParameter<CoefficientType> * lfGains = mLowFrequencyGainOutput ? &mLowFrequencyGainOutput->data() : nullptr;
    pml::MatrixParameter<CoefficientType> * hfGains = mHighFrequencyGainOutput ? &mHighFrequencyGainOutput->data() : nullptr;
    pml::MatrixParameter<CoefficientType> * diffuseGains = mDiffuseGainOutput ? &mDiffuseGainOutput->data() : nullptr;

    if( lfGains )
    {
      lfGains->zeroFill();
    }
    if( hfGains )
    {
      hfGains->zeroFill();
    }
    if( diffuseGains )
    {
      diffuseGains->zeroFill();
    }

    // Loop over all objects.
    for( objectmodel::Object const & obj : objects )
    {
      mTmpGains.zeroFill();
      mTmpHfGains.zeroFill();
      mTmpDiffuseGains.zeroFill();
      bool objectHandled = false; // Determine whehter the present object has been handled by the panner.
      // ChannelObjects are special because they can be multichannel.
      objectmodel::ChannelObject const * co = dynamic_cast<objectmodel::ChannelObject const *>( &obj );
      if( co )
      {
        SampleType diffuseRatio = co->diffuseness();
        std::size_t const numChannels = co->numberOfChannels();
        for( std::size_t chIdx(0); chIdx < numChannels; ++chIdx )
        {
          std::size_t const objChannelIdx = co->channelIndex( chIdx );
          if( chIdx >= mNumberOfObjects )
          {
            status( StatusMessage::Warning, "Object channel index for channel object (", chIdx,  ") exceeds admissible range." );
          }
          mTmpGains.zeroFill();
          objectmodel::ChannelObject::OutputChannelList const & outputChannels = co->outputChannel( chIdx );
          for( objectmodel::ChannelObject::OutputChannelId const & outLabel : outputChannels )
          {
            ChannelLabelLookup::const_iterator findIdx = mLabelLookup.find( outLabel );
            if( findIdx == mLabelLookup.end() )
            {
              status( StatusMessage::Warning, "Object channel index for channel object (", chIdx, ") exceeds admissible range." );
            }
            assert( findIdx->second < mNumberOfRegularLoudspeakers ); // The lookup contains only regular loudspeakers
            mTmpGains[findIdx->second] = 1.0f; // Normalisation and direct/diffuse scaling is performed later on
          }
          SampleType directRatio = 1.0f - diffuseRatio;
          if( lfGains )
          {
            normalise( mTmpGains.data(), lfGains->data() + objChannelIdx, mNumberOfRegularLoudspeakers,
                       mLfNormalisation, lfGains->stride(), directRatio );
          }
          if( hfGains )
          {
            normalise( mTmpGains.data(), hfGains->data() + objChannelIdx, mNumberOfRegularLoudspeakers,
              mHfNormalisation, hfGains->stride(), directRatio );
          }
          if( diffuseGains )
          {
            normalise( mTmpGains.data(), diffuseGains->data() + objChannelIdx, mNumberOfRegularLoudspeakers,
              mDiffuseNormalisation, diffuseGains->stride(), diffuseRatio );
          }
        }
        continue;
      }
      // From here on we only deal with single-channel objects. That means we can skip all other objects
      if( obj.numberOfChannels() != 1 )
      {
        continue;
      }
      objectmodel::Object::ChannelIndex const objChannelIdx = obj.channelIndex(0);
      objectmodel::DiffuseSource const * ds = dynamic_cast<objectmodel::DiffuseSource const *>(&obj);
      if( ds and diffuseGains )
      {
        mTmpDiffuseGains.fillValue( 1.0f ); // All levels to same value
        normalise( mTmpDiffuseGains.data(), diffuseGains->data() + objChannelIdx, mNumberOfRegularLoudspeakers,
          mDiffuseNormalisation, diffuseGains->stride(), 1.0f );
        continue;
      }
      SampleType diffuseRatio = 0.0f; // Note: 0.0f is also used as a 'magic value' later on.
      objectmodel::PlaneWave const * pw = dynamic_cast<objectmodel::PlaneWave const *>(&obj);
      if( pw )
      {
        SampleType x,y,z;
        std::tie( x, y, z ) = efl::spherical2cartesian( pw->incidenceAzimuth(), pw->incidenceElevation(), pw->referenceDistance() );
        mVbapCalculator->calculateGainsUnNormalised( x, y, z, &mTmpGains[0], true /*planeWave*/ );
        diffuseRatio = 0.0f;
        objectHandled = true;
      }
      objectmodel::PointSource const * ps = dynamic_cast<objectmodel::PointSource const *>(&obj);
      if( ps )
      {
        objectHandled = true;
        mVbapCalculator->calculateGainsUnNormalised( ps->x(), ps->y(), ps->z(), mTmpGains.data(), false /*planeWave*/ );

        objectmodel::PointSourceWithDiffuseness const * psd = dynamic_cast<objectmodel::PointSourceWithDiffuseness const *>(&obj);
        diffuseRatio = psd ? psd->diffuseness() : 0.0f;

        // In the current implementation, the diffuse part of a PointSourceWithDiffuseness if uniformly distributed over all loudspeakers.
        if( diffuseRatio >= std::numeric_limits<SampleType>::epsilon() )
        {
          mTmpDiffuseGains.fillValue( 1.0f );
        }
        // TODO: Add special handling (panning function) for extent sources (subtype of PointSourceWithDiffuseness)
      }
      if( objectHandled )
      {
        SampleType directRatio = 1.0f - diffuseRatio;
        if( lfGains )
        {
          normalise( mTmpGains.data(), lfGains->data() + objChannelIdx, mNumberOfRegularLoudspeakers,
            mLfNormalisation, lfGains->stride(), directRatio );
        }
        if( hfGains )
        {
          std::transform( mTmpGains.data(), mTmpGains.data()+mNumberOfRegularLoudspeakers, mTmpHfGains.data(), [](SampleType val ){ return std::sqrt(val); } );
          normalise( mTmpHfGains.data(), hfGains->data() + objChannelIdx, mNumberOfRegularLoudspeakers,
            mHfNormalisation, hfGains->stride(), directRatio );
        }
        if( diffuseGains )
        {
          normalise( mTmpDiffuseGains.data(), diffuseGains->data() + objChannelIdx, mNumberOfRegularLoudspeakers,
            mDiffuseNormalisation, diffuseGains->stride(), diffuseRatio );
        }
      }
    }
#else
    pml::MatrixParameter<CoefficientType> & gainMatrix = mLowFrequencyGainOutput->data( );

    if( (gainMatrix.numberOfRows( ) != mNumberOfRegularLoudspeakers) or( gainMatrix.numberOfColumns( ) != mNumberOfObjects ) )
    {
      throw std::invalid_argument( "PanningCalculator::process(): The size of the gain matrix does not match the object/loudspeaker configuration." );
    }
    // Not necessary if we opt to set the complete matrix.
    gainMatrix.zeroFill( );

    // For the moment, we assume that the audio channels of the objects are identical to the final channel numbers.
    // Any potential re-routing will be added later.
    for( objectmodel::Object const & obj : objects )
    {
      if( obj.numberOfChannels( ) != 1 )
      {
        std::cerr << "PanningCalculator: Only monaural object types are supported at the moment." << std::endl;
        continue;
      }

      objectmodel::Object::ChannelIndex const channelId = obj.channelIndex( 0 );
      if( channelId >= mNumberOfObjects )
      {
        std::cerr << "PanningCalculator: Channel index \"" << channelId << "\" of object id#" << obj.id()
            << "exceeds number of channels (" << mNumberOfObjects << ")." << std::endl;
        continue;
      }

      // Use C++ type information to check whether the source is a pointsource.
      objectmodel::PointSource const * pointSrc = dynamic_cast<objectmodel::PointSource const *>(&obj);
      if( pointSrc )
      {
        // channelLock processing first
        std::valarray<SampleType> sourcePos( mVectorDimension );
        // Source positions need to be normalised before channel lock processing
        sourcePos[0] = pointSrc->x();
        sourcePos[1] = pointSrc->y();
        if( mVectorDimension == 3 )
        {
          sourcePos[2] = pointSrc->z();
          SampleType const norm = std::sqrt( sourcePos[0] * sourcePos[0] + sourcePos[1] * sourcePos[1] + sourcePos[2] * sourcePos[2] );
          sourcePos = static_cast<SampleType>(1.0)/norm * sourcePos;
        }
        else
        {
          SampleType const norm = std::sqrt( sourcePos[0] * sourcePos[0] + sourcePos[1] * sourcePos[1] );
          sourcePos = static_cast<SampleType>(1.0) / norm * sourcePos;
        }

        efl::ErrorCode res = efl::product( &sourcePos[0], mLoudspeakerPositions.data(), mTmpGains.data(),
                                           1 /*numResultRows*/, mNumberOfRegularLoudspeakers /* numResultColumns */, mVectorDimension /*numOp1Columns*/,
                                           1 /* op1RowStride */, 1 /*op1ColumnStride*/,
                                           mLoudspeakerPositions.stride() /*op2RowStride*/, 1 /*op2ColumnStride*/,
                                           1 /*resRowStride, not used, since there is only one row. */, 1 /*resColumnStride*/ );
        if( res != efl::noError )
        {
          // TODO: Replace by error API call.
          throw std::runtime_error( "PanningCalculator: dot product for computing channel lock failed.");
        }
        // Search only across the regular loudspeakers
        // We search for the maximum, because we are using the dot product, i.e., the cosine of the angular distance.
        // In case of equidistant nearest speakers, the first is returned.
        CoefficientType const * maxDotProduct = std::max_element( mTmpGains.data(), mTmpGains.data() + mNumberOfRegularLoudspeakers );
        if( maxDotProduct - mTmpGains.data() >= static_cast<std::ptrdiff_t>(mNumberOfRegularLoudspeakers) )
        {
          // TODO: Replace by error API call.
          throw std::runtime_error( "PanningCalculator: dot product for computing channel lock failed.");
        }
        CoefficientType const lockLimit = std::cos( efl::degree2radian(pointSrc->channelLockDistance() ) );
        if( *maxDotProduct >= lockLimit )
        {
          // Cast is safe because maxDotProduct >= mLoudspeakerDotProducts.data() always holds because of the way it is obtained
          std::size_t const lockLspIdx = static_cast<std::size_t>( maxDotProduct - mTmpGains.data() );
          mTmpGains.zeroFill();
          mTmpGains[lockLspIdx] = static_cast<SampleType>(1.0);
        }
        else
        {
          mVbapCalculator->calculateGains( pointSrc->x(), pointSrc->y(), pointSrc->z(),
                                           mTmpGains.data() );
        }
        // special handling for point sources with diffuseness: Adjust the level of the direct sound.
        objectmodel::PointSourceWithDiffuseness const * psdSrc = dynamic_cast<objectmodel::PointSourceWithDiffuseness const *>(&obj);
        if( psdSrc )
        {
          efl::ErrorCode res = efl::vectorMultiplyConstantInplace( 1.0f-psdSrc->diffuseness(), mTmpGains.data(), mNumberOfRegularLoudspeakers );
          if( res != efl::noError )
          {
            status( StatusMessage::Error, "Gain adjustment for source diffuseness failed: ", efl::errorMessage(res) );
            return;
          }
        }
      }
      else
      {
        objectmodel::PlaneWave const * pwSrc = dynamic_cast<objectmodel::PlaneWave const *>(&obj);
        {
          if( pwSrc )
          {
            SampleType posX, posY, posZ;
            std::tie( posX, posY, posZ )
            = efl::spherical2cartesian( efl::degree2radian( pwSrc->incidenceAzimuth() ),
                                        efl::degree2radian( pwSrc->incidenceElevation() ), 1.0f );
            mVbapCalculator->calculateGains( posX, posY, posZ, mTmpGains.data() );
            // We need to copy the data explicitly into a matrix column of a row-major matrix.
            // This could be replaced by a copy function with a stride argument.
            efl::ErrorCode res = efl::vectorCopyStrided( mTmpGains.data(), &gainMatrix( 0, channelId ), 1, gainMatrix.stride(),
                                                         mNumberOfRegularLoudspeakers, 0/*no assumptions about alignment possible*/ );
            if( res != efl::noError )
            {
              status( StatusMessage::Error, "Error while copying panning gains: ", efl::errorMessage(res) );
              return;
            }
          }
        }
      }
      if( separateLowpassPanning() ) // If there
      {
        // First compute the LF gains by re-normalising to the coherent sound pressure (sum of gains)
        // In case of nonnegative gains this is equivalent to the l1 norm.
        // If this cannot be ensured, we need to decide whether we want to normalise for the l1 norm or the sound pressure.
        SampleType const sum = std::accumulate( mTmpGains.data(), mTmpGains.data()+mNumberOfRegularLoudspeakers,
                                                static_cast<SampleType>(0.0),
                                                [](SampleType acc, SampleType val ){ return acc + val; } );
        SampleType const scaleFactor = static_cast<SampleType>(1.0) / sum;
        pml::MatrixParameter<SampleType> & lfGains = mLowFrequencyGainOutput->data();
        for( std::size_t lspIdx(0); lspIdx < mNumberOfRegularLoudspeakers; ++lspIdx )
        {
          lfGains( lspIdx, channelId ) = scaleFactor * mTmpGains[lspIdx];
        }

        // Now compute the VBIP (HF) gains  (generalise with pow)
//        std::for_each( mTmpGains.data(), mTmpGains.data()+mNumberOfRegularLoudspeakers,
                       // Safeguard against potentially negative values that would yield NaNs
                       [](SampleType & val ){ val = std::pow( std::max( val, static_cast<SampleType>(0.0) ), 0.0 ); });

        std::for_each( mTmpGains.data(), mTmpGains.data()+mNumberOfRegularLoudspeakers,
                        // Safeguard against potentially negative values that would yield NaNs
                        [](SampleType & val ){ val = std::sqrt( std::max( val, static_cast<SampleType>(0.0) ) ); });

        // Re-normalise with l2 (power) norm.
        SampleType const l2Sqr = std::accumulate( mTmpGains.data(), mTmpGains.data()+mNumberOfRegularLoudspeakers,
                                                  static_cast<SampleType>(0.0),
                                                  [](SampleType acc, SampleType val ){ return acc + val*val; } );
        // l2Sqr should be identical to 'sum' provided that the gains are nonnegative.
        SampleType const l2ScaleFactor = static_cast<SampleType>(1.0)/ std::sqrt(l2Sqr);
        std::for_each( mTmpGains.data(), mTmpGains.data()+mNumberOfRegularLoudspeakers,
                       [l2ScaleFactor](SampleType & val ){ val = l2ScaleFactor * val; } );
        // Fall through to copy the HF gains
      }
      //  We need to copy the data explicitly into a matrix column of a row-major matrix.
      efl::ErrorCode const res = efl::vectorCopyStrided( mTmpGains.data(), &gainMatrix( 0, channelId ), 1, gainMatrix.stride(),
                                                         mNumberOfRegularLoudspeakers, 0/*no assumptions about alignment possible*/ );
      if( res != efl::noError )
      {
        status( StatusMessage::Error, "Error while copying panning gains: ", efl::errorMessage(res) );
        return;
      }
    }
#endif
    mObjectVectorInput->resetChanged();
  }
}

PanningCalculator::PanningMode operator&( PanningCalculator::PanningMode lhs,
                                          PanningCalculator::PanningMode rhs )
{
  using T = std::underlying_type<PanningCalculator::PanningMode>::type;
  return static_cast<PanningCalculator::PanningMode>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

PanningCalculator::PanningMode operator|( PanningCalculator::PanningMode lhs,
                                          PanningCalculator::PanningMode rhs )
{
  using T = std::underlying_type<PanningCalculator::PanningMode>::type;
  return static_cast<PanningCalculator::PanningMode>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

/*static*/ PanningCalculator::ChannelLabelLookup
PanningCalculator::fillLabelLookup( panning::LoudspeakerArray const & config )
{
  PanningCalculator::ChannelLabelLookup table;
  std::size_t const numSpeakers = config.getNumRegularSpeakers();
  for( std::size_t channelIndex( 0 ); channelIndex < numSpeakers; ++channelIndex )
  {
    // Use the translation function of the loudspeaker array.
    panning::LoudspeakerArray::LoudspeakerIdType const lspId
      = config.loudspeakerId( channelIndex );
    bool insertRes;
    std::tie( std::ignore, insertRes ) = table.insert(
      std::make_pair( static_cast<objectmodel::ChannelObject::OutputChannelId>(lspId), channelIndex ) );
    if( not insertRes )
    {
      throw std::invalid_argument( "Insertion of channel index routing failed." );
    }
  }
  return table;
}


} // namespace rcl
} // namespace visr
