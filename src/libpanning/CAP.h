//
//  CAP.h
//  Created by Dylan Menzies on 24/1/2018.
//  Copyright (c) ISVR, University of Southampton. All rights reserved.
//
//  Minimum energy Multichannel Compensated Amplitude Panning
//  Panning compensated for head orientation, valid in ITD frequency range ~(0,1000)Hz
//

#ifndef __S3A_renderer_dsp__CAP__
#define __S3A_renderer_dsp__CAP__

// uncomment to enable debug output
// #define CAP_DEBUG_MESSAGES

#include "defs.h"
#include "LoudspeakerArray.h"

#include "export_symbols.hpp"

#include <libefl/basic_matrix.hpp>

namespace visr
{
namespace panning
{

class VISR_PANNING_LIBRARY_SYMBOL CAP
{
private:
    
    LoudspeakerArray const * m_array;
    efl::BasicMatrix<Afloat> m_gain;
    XYZ m_listenerPos;
    XYZ m_listenerAuralAxis; // normal vector through ears
    XYZ const * m_sourcePos;
    std::size_t m_nSources;
    Afloat m_maxGainPreComp = 1.5f; // max panning gain prior to distance compensation
    Afloat m_maxGain = 10.0f; // overall max panning gain
//  Afloat m_reorientMatrix[3][3] = {{1.0f,0.0f,0.0f}, {0.0f,1.0f,0.0f}, {0.0f,0.0f,1.0f}};
    bool m_HFmode = false;  // experimental
    
    
public:
  /**
   * Default constructor.
   * Sets object to save values (no array, zero sources).
   */
  CAP();

    int setLoudspeakerArray(LoudspeakerArray const * array){
        m_array = array;
        m_gain.resize( m_nSources, array->getNumRegularSpeakers( ) );
        return 0;
    }

    int setListenerPosition(Afloat x, Afloat y, Afloat z){
//      printf("setListenerPosition %f %f %f\n",x,y,z);
        
        
#ifdef CAP_DEBUG_MESSAGES
      printf("setListenerPosition %f %f %f\n",x,y,z);
#endif

        // printf("setListenerPosition %f %f %f\n",x,y,z);

        m_listenerPos.set(x, y, z);

        return 0;
    }
    
    
    
    int setListenerOrientation(Afloat yaw, Afloat pitch, Afloat roll, bool zero);

    
    
    int setListenerAuralAxis(Afloat x, Afloat y, Afloat z){
        
#ifdef CAP_DEBUG_MESSAGES
        printf("setListenerAuralAxis %f %f %f\n",x,y,z);
#endif
        
        m_listenerAuralAxis.set(x, y, z);
        
        return 0;
    }

    
    int setSourcePositions(XYZ const *sp ) {
        m_sourcePos = sp;
        return 0;
    }
    
    std::size_t setNumSources( std::size_t n) {
        m_nSources = n;
	// Take care of the fact that the loudspeaker
	// array might not been set yet.
	std::size_t const numSpeakers = m_array ? m_array->getNumSpeakers() : 0;
        m_gain.resize( m_nSources, numSpeakers );
        return n;
    }
    
    int setMaxGain(Afloat mg) { m_maxGain = mg; return 0; }
    
    int setHFmode() { m_HFmode = true; return 0; }
    
    std::size_t getNumSpeakers() const { return m_array->getNumSpeakers(); }
    
    int calcGains();
    
    efl::BasicMatrix<Afloat> const & getGains() const {
       return m_gain;
    }
    

    
};

} // namespace panning
} // namespace visr

#endif /* defined(__S3A_renderer_dsp__CAP__) */
