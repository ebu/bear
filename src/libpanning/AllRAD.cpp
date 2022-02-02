//
//  AllRAD.cpp
//
// Created by Dylan Menzies on 12/12/2014.
// Modified: Andreas Franck 23/09/2015
// Copyright (c) ISVR, University of Southampton. All rights reserved.
//

#include "AllRAD.h"

#include "LoudspeakerArray.h"

#include <libefl/matrix_functions.hpp>

namespace visr
{
namespace panning
{

AllRAD::AllRAD(LoudspeakerArray const & regularArray,
               LoudspeakerArray const & realArray,
               efl::BasicMatrix<Afloat> const & decodeCoeffs,
               unsigned int maxHoaOrder )
 : mRegularLoudspeakerPositions( regularArray.getNumRegularSpeakers(), 3, decodeCoeffs.alignmentElements() )
 , mRealDecoder( realArray )
 , mNumberOfHarmonics( (maxHoaOrder + 1) * (maxHoaOrder + 1) )
 , mRegularDecodeCoefficients( decodeCoeffs.numberOfRows( ), decodeCoeffs.numberOfColumns( ), decodeCoeffs.alignmentElements( ) )
 , mRealDecodeCoefficients( mNumberOfHarmonics, realArray.getNumRegularSpeakers(), decodeCoeffs.alignmentElements() )
 , mRegularArraySize( regularArray.getNumSpeakers( ) )
 , mRegularToRealDecodeCoefficients( mRegularArraySize, realArray.getNumRegularSpeakers(), decodeCoeffs.alignmentElements() )
{
  if( decodeCoeffs.numberOfRows() != mNumberOfHarmonics )
  {
    throw std::invalid_argument( "AllRad coefficient calculator: The size (number of rows) of the decode coefficient matrix does not match the specified maximum HOA order.");
  }
  if( decodeCoeffs.numberOfColumns() != mRegularArraySize )
  {
    throw std::invalid_argument( "AllRad coefficient calculator: The size (number of columns) of the decode coefficient matrix does not match the size or the regular loudspeaker array.");
  }
  for( std::size_t regSpkIdx(0); regSpkIdx < regularArray.getNumRegularSpeakers(); ++regSpkIdx )
  {
    XYZ const & pos = regularArray.getPosition( regSpkIdx );
    mRegularLoudspeakerPositions( regSpkIdx, 0 ) = pos.x;
    mRegularLoudspeakerPositions( regSpkIdx, 1 ) = pos.y;
    mRegularLoudspeakerPositions( regSpkIdx, 2 ) = pos.z;
  }
  mRegularDecodeCoefficients.copy( decodeCoeffs );
  updateDecodingCoefficients();
}

void AllRAD::setListenerPosition( SampleType x, SampleType y, SampleType z )
{
  mRealDecoder.setListenerPosition( x, y, z );
  updateDecodingCoefficients();
}

//void AllRAD::setListenerPosition( XYZ const & newPosition )
//{
//  setListenerPosition( newPosition.x, newPosition.y, newPosition.z );
//}

void AllRAD::updateDecodingCoefficients()
{
  for( std::size_t regSpeakerIdx(0); regSpeakerIdx < mRegularArraySize; ++regSpeakerIdx )
  {
    mRealDecoder.calculateGains( mRegularLoudspeakerPositions(regSpeakerIdx,0),
                                 mRegularLoudspeakerPositions(regSpeakerIdx,1),
                                 mRegularLoudspeakerPositions(regSpeakerIdx,2),
                                 mRegularToRealDecodeCoefficients.row( regSpeakerIdx ),
                                 false /* planeWave=false, i.e., use the standard VBAP point source algorithm.
                                         This means that the source positions are translated by the tracked listener position.*/
                                );
  }
  efl::ErrorCode const res = efl::product( mRegularDecodeCoefficients.data(),
                                      mRegularToRealDecodeCoefficients.data(),
                                      mRealDecodeCoefficients.data(),
                                      mNumberOfHarmonics /*numResultRows*/,
                                      mRealDecodeCoefficients.numberOfColumns() /*numResultColumns*/,
                                      mRegularArraySize /*numOp1Columns*/,
                                      mRegularDecodeCoefficients.stride() /*op1RowStride*/,
                                      1 /*op1ColumnStride*/,
                                      mRegularToRealDecodeCoefficients.stride() /*op2RowStride*/,
                                      1 /*op2ColumnStride*/,
                                      mRealDecodeCoefficients.stride() /*resRowStride*/,
                                      1 /*resColumnStride*/ );
  if( res != efl::noError )
  {
    throw std::runtime_error( std::string("AllRad coefficient calculator: Numeric error while calculating the final decode oefficients:.")
    + efl::errorMessage(res) );
  }
}

} // namespace panning
} // namespace visr
