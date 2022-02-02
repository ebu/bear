/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "cap_gain_calculator.hpp"

#include <libefl/basic_matrix.hpp>
#include <libefl/cartesian_spherical_conversion.hpp>
#include <libefl/degree_radian_conversion.hpp>
#include <libefl/vector_functions.hpp>

#include <libobjectmodel/object_vector.hpp>
// Note: At the moment, all supported source types are translated directly in this file
// TODO: For the future, consider moving this to another location.
#include <libobjectmodel/diffuse_source.hpp>
#include <libobjectmodel/point_source.hpp>
#include <libobjectmodel/point_source_with_diffuseness.hpp>
#include <libobjectmodel/plane_wave.hpp>

#include <libpanning/CAP_VBAP.h>

#include <libpml/listener_position.hpp>

#include <boost/filesystem.hpp>

#include <ciso646>
#include <cstdio>
#include <numeric>

namespace visr
{
namespace rcl
{

CAPGainCalculator::CAPGainCalculator( SignalFlowContext const & context,
                                      char const * name,
                                      CompositeComponent * parent,
                                      std::size_t numberOfObjects,
                                      panning::LoudspeakerArray const & arrayConfig,
                                      PanningMode panningMode /* = PanningMode::LF */,
                                      bool hfGainOutput /*= false*/
                                      )
 : AtomicComponent( context, name, parent )
 , mNumberOfObjects( numberOfObjects )
 , mNumberOfLoudspeakers( arrayConfig.getNumRegularSpeakers() )
 , mSourcePositions( mNumberOfObjects, panning::XYZ() )
 , mVbipCalculator( hfGainOutput ? new panning::CAP_VBAP( arrayConfig ) : nullptr )
 , mObjectVectorInput( "objectVectorInput", *this, pml::EmptyParameterConfig() )
 , mListenerPositionInput( "listenerPosition", *this, pml::EmptyParameterConfig() )
 , mGainOutput( "gainOutput", *this, pml::MatrixParameterConfig( arrayConfig.getNumRegularSpeakers(), mNumberOfObjects ) )
 , mHfGainOutput( hfGainOutput ? new GainOutput("hfGainOutput", *this, pml::MatrixParameterConfig(arrayConfig.getNumRegularSpeakers(), mNumberOfObjects) )
                               : nullptr )
{
  mLevels.resize( mNumberOfObjects );
  mLevels = 0.0f;

  mCapCalculator.setLoudspeakerArray( &arrayConfig );
  mCapCalculator.setNumSources( numberOfObjects );

  // Set initial positions
  mCapCalculator.setListenerPosition( static_cast<CoefficientType>(0.0), static_cast<CoefficientType>(0.0), static_cast<CoefficientType>(0.0) );
  mCapCalculator.setListenerAuralAxis( static_cast<CoefficientType>(1.0), static_cast<CoefficientType>(0.0), static_cast<CoefficientType>(0.0) );
    
  if (panningMode == PanningMode::HF) mCapCalculator.setHFmode();

  if( mVbipCalculator )
  {
    mVbipCalculator->setListenerPosition(static_cast<CoefficientType>(0.0), static_cast<CoefficientType>(0.0), static_cast<CoefficientType>(0.0));
    mTempHfGains.resize(arrayConfig.getNumRegularSpeakers());
  }
}

CAPGainCalculator::~CAPGainCalculator() = default;

void CAPGainCalculator::setNearTripletBoundaryCosTheta(SampleType ct)
{
  if( not mVbipCalculator )
  {
    throw std::logic_error( "CAPGainCalculator: setNearTripletBoundaryCosTheta() does not make sense if the HF gain calculation is deactivated." );
  }
  mVbipCalculator->setNearTripletBoundaryCosTheta(ct);
}

void CAPGainCalculator::process()
{
  if( mListenerPositionInput.changed() )
  {
    pml::ListenerPosition const & listener = mListenerPositionInput.data();
    // Set the listener position and orientation
    // TODO: Add direct inter-aural axis control
    mCapCalculator.setListenerPosition(listener.x(), listener.y(), listener.z());
    mCapCalculator.setListenerOrientation(listener.yaw(), listener.pitch(), listener.roll(), false);
    // printf("setListenerOrientation ypr %f %f %f %d\n", listener.yaw(), listener.pitch(), listener.roll(), false );

    if( mVbipCalculator )
    {
      mVbipCalculator->setListenerPosition(listener.x(), listener.y(), listener.z());
    }
    mListenerPositionInput.resetChanged();
  }

  // TODO: decide whether to always process
  if (mObjectVectorInput.changed() or mListenerPositionInput.changed())
  {
    efl::BasicMatrix<CoefficientType> & gains = mGainOutput.data();
    assert((gains.numberOfRows() == mNumberOfLoudspeakers) and (gains.numberOfColumns() == mNumberOfObjects));
    process(mObjectVectorInput.data(), gains);
  }

  if( mObjectVectorInput.changed() )
  {
    mObjectVectorInput.resetChanged();
  }
}

void CAPGainCalculator::process( objectmodel::ObjectVector const & objects, 
                                 efl::BasicMatrix<CoefficientType> & gainMatrix )
{
  gainMatrix.zeroFill(); // Zero the result matrix to get zero gains for all unused source channels.
  mLevels = 0.0f;
  // As not every source object in the VBAP calculator component might correspond to a real source, we have to set them to safe position beforehand.
  panning::XYZ defaultSource( 1.0f, 0.0f, 0.0f );
  for( std::size_t srcIdx(0); srcIdx < mNumberOfObjects; ++srcIdx )
  {
    mSourcePositions[ srcIdx ] = defaultSource;
  }

  // For the moment, we assume that the audio channels of the objects are identical to the final channel numbers.
  // Any potential re-routing will be added later.
  for( objectmodel::Object const & obj : objects )
  {
    if( obj.numberOfChannels() != 1 )
    {
      // Panning is implemented only for single-channel objects.
      // So we use this check as a first criterion to skip any sources of other types
      // Other non-matching types will be skipped later on.
      continue;
    }

    objectmodel::Object::ChannelIndex const channelId = obj.channelIndex( 0 );
    if( channelId >= mNumberOfObjects )
    {
      std::cerr << "CAPGainCalculator: Channel index \"" << channelId << "\" of object id#" << obj.id()
                << "exceeds number of channels (" << mNumberOfObjects << ")." << std::endl;
      continue;
    }

    objectmodel::ObjectTypeId const ti = obj.type();

    // For the moment, we treat the two supported source type here.
    // @todo find a proper abstraction to handle many source types.
    switch( ti )
    {
    case objectmodel::ObjectTypeId::PointSourceWithDiffuseness:
    {
      objectmodel::PointSourceWithDiffuseness const & psdSrc = dynamic_cast<objectmodel::PointSourceWithDiffuseness const &>(obj);
      mLevels[channelId] = (static_cast<objectmodel::LevelType>(1.0)-psdSrc.diffuseness()); // Adjust the amount of direct sound according to the diffuseness
      // Fall through intentionally
    }
    case objectmodel::ObjectTypeId::PointSource:
    {
      objectmodel::PointSource const & pointSrc = dynamic_cast<objectmodel::PointSource const &>(obj);
      mSourcePositions[channelId].set( pointSrc.x(), pointSrc.y(), pointSrc.z() );
      mLevels[channelId] = 1.0f;
      break;
    }
    case objectmodel::ObjectTypeId::PlaneWave:
    {
      objectmodel::PlaneWave const & planeSrc = dynamic_cast<objectmodel::PlaneWave const &>(obj);
      objectmodel::Object::Coordinate xPos, yPos, zPos;
      std::tie( xPos, yPos, zPos ) = efl::spherical2cartesian( efl::degree2radian( planeSrc.incidenceAzimuth() ),
                                                               efl::degree2radian( planeSrc.incidenceElevation() ),
                                                               1.0f);
      mSourcePositions[ channelId ].set( xPos, yPos, zPos, true /*atInfinity corresponds to a plane wave */);
      mLevels[channelId] = 1.0f;
      break;
    }
    default:
      // Ignore unknown source types by setting them to a zero level).
      // That means that the VBAP gains will be calculated for the default position, but zeroed afterwards.
      mLevels[channelId] = static_cast<objectmodel::LevelType>(0.0f);
    }
  } // for( objectmodel::ObjectVector::value_type const & objEntry : objects )
  mCapCalculator.setSourcePositions( &mSourcePositions[0] );

  if( mCapCalculator.calcGains() != 0 )
  {
    std::cout << "CAPGainCalculator: Error calculating VBAP gains." << std::endl;
  }
  
  efl::BasicMatrix<Afloat> const & capGains = mCapCalculator.getGains();
  // This is essentially a matrix transposition and a row-wise scalar multiplication.
  for( std::size_t chIdx(0); chIdx < mNumberOfObjects; ++chIdx )
  {
    Afloat const * const gainRow = capGains.row( chIdx );
    objectmodel::LevelType const level = mLevels[ chIdx ];
//      std::cout<<"gain obj "<<chIdx<<" [";
    for( std::size_t outIdx(0); outIdx < mNumberOfLoudspeakers; ++outIdx )
    {
      gainMatrix( outIdx, chIdx ) = level * gainRow[ outIdx ];
//        std::cout<<std::to_string(gainMatrix( outIdx, chIdx ))<<", ";
    }
//      std::cout<<"]"<<std::endl;
  }
  
  //if (rand() % 256 == 1)
  //printf("gains obj0 %f %f\n", gainMatrix(0,0), gainMatrix(1,0) );

  if( mVbipCalculator )
  {
    efl::BasicMatrix<Afloat> & vbipGains = mHfGainOutput->data();
    for (std::size_t chIdx(0); chIdx < mNumberOfObjects; ++chIdx)
    {
      panning::XYZ const & pos = mSourcePositions[chIdx];
      objectmodel::LevelType const level = mLevels[chIdx];
      mVbipCalculator->calculateGainsUnNormalised(pos.x, pos.y, pos.z, &mTempHfGains[0], pos.isInfinite );
      std::for_each(mTempHfGains.begin(), mTempHfGains.end(), [](Afloat & val) { return std::sqrt(std::max(val, 0.0f)); });
      Afloat const sigNorm = std::sqrt(std::accumulate(mTempHfGains.begin(), mTempHfGains.end(), 0.0f,
        [](SampleType acc, SampleType val) { return acc + val * val; }));
      Afloat const normedLevel = level / std::max(sigNorm, std::numeric_limits<SampleType>::epsilon());
      for (std::size_t outIdx(0); outIdx < mNumberOfLoudspeakers; ++outIdx)
      {
        vbipGains(outIdx, chIdx) = normedLevel * mTempHfGains[outIdx];
      }
    }
  }

}

} // namespace rcl
} // namespace visr
