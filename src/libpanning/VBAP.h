//
//  VBAP.h
//
//  Created by Dylan Menzies on 10/11/2014.
//  Copyright (c) ISVR, University of Southampton. All rights reserved.
//

#ifndef __S3A_renderer_dsp__VBAP__
#define __S3A_renderer_dsp__VBAP__

// uncomment to enable debug output
// #define VBAP_DEBUG_MESSAGES

#include "defs.h"
#include "LoudspeakerArray.h"

#include "export_symbols.hpp"

#include <libefl/basic_matrix.hpp>

namespace visr
{
namespace panning
{
  
class VISR_PANNING_LIBRARY_SYMBOL VBAP
{
public:
  /**
   * Construct an initialised VBAP configuration.
   * @param array array of loudspeakers).
   * @param x Cartesian x coordinate of the listener position
   * @param y Cartesian y coordinate of the listener position
   * @param z Cartesian z coordinate of the listener position
   */
  explicit VBAP( const LoudspeakerArray &array, SampleType x = 0.0f, SampleType y = 0.0f, SampleType z = 0.0f );
  VBAP( VBAP const & ) = delete;
  
  /**
   * Calculate the panning gains for a single source position and apply power normalisation.
   * @param x Cartesian x coordinate of the source position
   * @param y Cartesian y coordinate of the source position
   * @param z Cartesian z coordinate of the source position
   * @param planeWave Whether the source is handles a point source (false) or plane wave (true)
   * @param[out] gains array holding the panning gains for the regular (non-virtual) loudspeakers).
   * Buffer must provide space for at least getNumSpeakers() values.
   */
  void calculateGains( SampleType x, SampleType y, SampleType z, SampleType * gains, bool planeWave ) const;

  /**
  * Calculate the panning gains for a single source position without normalisation.
  * @param x Cartesian x coordinate of the source position
  * @param y Cartesian y coordinate of the source position
  * @param z Cartesian z coordinate of the source position
  * @param planeWave Whether the source is handles a point source (false) or plane wave (true)
  * @param[out] gains array holding the panning gains for the regular (non-virtual) loudspeakers).
  * Buffer must provide space for at least getNumSpeakers() values.
  */
  void calculateGainsUnNormalised( SampleType x, SampleType y, SampleType z, SampleType * gains, bool planeWave ) const;

  /**
   * Reset the listener position.
   * This causes a recalculation of the internal data structures (inverse matrices)
   * @param x Cartesian x coordinate of the new listener position
   * @param y Cartesian y coordinate of the new listener position
   * @param z Cartesian z coordinate of the new listener position
   */
  void setListenerPosition( SampleType x, SampleType y, SampleType z );
  
private:
  
  bool is2D;
  bool isInfinite;
  
  size_t numTotLoudspeakers;
  size_t numRegLoudspeakers;
  size_t numVirtLoudspeakers;
  size_t numTriplets;
  /**
   * Inverse matrix of the vector components of loudspeakers positions considering loudspeaker triplets.
   * Dimension: # triplets(Rows) x # vectorComponents(Columns).
   */
  efl::BasicMatrix<SampleType> mInvMatrix;
  
  /**
   * Matrix that contains the positions of the loudspeakers.
   * Dimension: # realLoudspeakers(Rows) x # 3Dcoordinates(Columns).
   */
  efl::BasicMatrix<SampleType> mPositions;
  
  /**
   * Matrix that contains the rereouting coefficients from virtual to real loudspeakers.
   * Dimension: # virtualLoudspeakers(Rows) x # realLoudspeakers(Columns).
   */
  efl::BasicMatrix<SampleType> mReroutingMatrix;
  
  /**
   * Vector holding the triplets of loudspeakers for VBAP calculation.
   */
  std::vector<LoudspeakerArray::TripletType> mTriplets;
  
  /**
   * Vector holding the cartesian coordinates of the listener.
   */
  std::array<SampleType, 3> mListenerPos;
  /**
   * Vector holding the VBAP gains for all loudspeakers.
   * Size: number of regular loudspeakers + number of virtual loudspeakers.
   * The gains are stored in order of their zero-offset logical loudspeaker index (not output signal index).
   * The gains of virtual loudspeakers are after the regular loudspeaker gains.
   */
  mutable std::vector<SampleType> mGain;
  
  /**
   * Calculates the inverse matrix required for VBAP gain calculation.
   */
  void calcInvMatrices();
  
  /**
   * Perform the basic VBAP gain calculation.
   * @param posX Cartesian x coordinate of the sound source position
   * @param posY Cartesian y coordinate of the sound source position
   * @param posZ Cartesian z coordinate of the sound source position
   * @param planeWave Flag whether the source is a poiunt source (false) or plane wave (true)
   */
  void calcPlainVBAP( SampleType posX, SampleType posY, SampleType posZ, bool planeWave ) const;
  
  /**
   * Applies rereouting coefficients to VBAP gains
   */
  void applyRerouting() const;
  
};
  
} // namespace panning
} // namespace visr

#endif /* defined(__S3A_renderer_dsp__VBAP__) */
