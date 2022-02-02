//
//  CAP_VBAP.h
//
//  Created by Dylan Menzies on 10/11/2014.
//  Copyright (c) ISVR, University of Southampton. All rights reserved.
//

#ifndef __S3A_renderer_dsp__CAP_VBAP__
#define __S3A_renderer_dsp__CAP_VBAP__

#include "defs.h"
#include "LoudspeakerArray.h"

#include "export_symbols.hpp"

#include <libefl/basic_matrix.hpp>

namespace visr
{
namespace panning
{

/**
 * Specialised version of the VBAP renderer to be used in combination with CAP (compensated amplitude panning).
 */
class VISR_PANNING_LIBRARY_SYMBOL CAP_VBAP
{
public:
  /**
   * Construct an initialised VBAP configuration.
   * @param array array of loudspeakers).
   * @param x Cartesian x coordinate of the listener position
   * @param y Cartesian y coordinate of the listener position
   * @param z Cartesian z coordinate of the listener position
   */
  explicit CAP_VBAP( const LoudspeakerArray &array, SampleType x = 0.0f, SampleType y = 0.0f, SampleType z = 0.0f );

  /**
   * Deleted copy constructor, avoid unintentional copying include pass-by-value
   */
  CAP_VBAP( CAP_VBAP const & ) = delete;
  
  /**
   * Calculate the panning gains for a single source position and apply power normalisation.
   * @param x Cartesian x coordinate of the source position
   * @param y Cartesian y coordinate of the source position
   * @param z Cartesian z coordinate of the source position
   * @param[out] gains array holding the panning gains for the regular (non-virtual) loudspeakers).
   * Buffer must provide space for at least getNumSpeakers() values.
   * @param planeWave Whether the source is handles a point source (false) or plane wave (true)
   */
  void calculateGains( SampleType x, SampleType y, SampleType z, SampleType * gains, bool planeWave /*= false*/) const;

  /**
  * Calculate the panning gains for a single source position without normalisation.
  * @param x Cartesian x coordinate of the source position
  * @param y Cartesian y coordinate of the source position
  * @param z Cartesian z coordinate of the source position
  * @param planeWave Whether the source is handles a point source (false) or plane wave (true)
  * Buffer must provide space for at least getNumSpeakers() values.
  * @param[out] gains array holding the panning gains for the regular (non-virtual) loudspeakers).
  */
  void calculateGainsUnNormalised( SampleType x, SampleType y, SampleType z, SampleType * gains, bool planeWave /*= false*/) const;

  /**
   * Reset the listener position.
   * This causes a recalculation of the internal data structures (inverse matrices)
   * @param x Cartesian x coordinate of the new listener position
   * @param y Cartesian y coordinate of the new listener position
   * @param z Cartesian z coordinate of the new listener position
   */
  void setListenerPosition( SampleType x, SampleType y, SampleType z );
    
  /**
   * Set the 2D fading mode.
   */
  void set2Dfade( bool m2Dfade );
    
  void setNearTripletBoundaryCosTheta( SampleType t ) {
      mNearTripletBoundaryCosTheta = t;
  }
  
private:
  
  bool mArrayIs2D;
  bool mArrayIsInfinite;
  bool m2Dfade;
  
  size_t numTotLoudspeakers;
  size_t numRegLoudspeakers;
  size_t numVirtLoudspeakers;
  size_t numTriplets;

  SampleType mNearTripletBoundaryCosTheta;
    
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
   * Vector holding the triplets of loudspeakers for CAP_VBAP calculation.
   */
  std::vector<LoudspeakerArray::TripletType> mTriplets;
  
  /**
   * Vector holding the cartesian coordinates of the listener.
   */
  std::array<SampleType, 3> mListenerPos;
  /**
   * Vector holding the CAP_VBAP gains for all loudspeakers.
   * Size: number of regular loudspeakers + number of virtual loudspeakers.
   * The gains are stored in order of their zero-offset logical loudspeaker index (not output signal index).
   * The gains of virtual loudspeakers are after the regular loudspeaker gains.
   */
  mutable std::vector<SampleType> mGain;
    

    /**
     * State indicates whether is listener is near to triplet boundary (in 2D only for now)
     * Used for 360 HF integration with CAP.
     */
  mutable std::vector<bool> mListenerIsNearTripletBoundary;

    
  
  /**
   * Calculates the inverse matrix required for CAP_VBAP gain calculation.
   */
  void calcInvMatrices();
  
  /**
   * Perform the basic CAP_VBAP gain calculation.
   * @param posX Cartesian x coordinate of the sound source position
   * @param posY Cartesian y coordinate of the sound source position
   * @param posZ Cartesian z coordinate of the sound source position
   * @param planeWave Flag whether the source is a poiunt source (false) or plane wave (true)
   */
  void calcPlainGains( SampleType posX, SampleType posY, SampleType posZ, bool planeWave ) const;
  
  /**
   * Applies rerouting coefficients to VBAP gains
   */
  void applyRerouting() const;
  
};
  
} // namespace panning
} // namespace visr

#endif /* defined(__S3A_renderer_dsp__CAP_VBAP__) */
