//
//  CAP.h
//  Created by Dylan Menzies on 24/1/2018.
//  Copyright (c) ISVR, University of Southampton. All rights reserved.
//  d.menzies@soton.ac.uk
//  dylan.menzies1@gmail.com
//
//  Minimum energy Multichannel Compensated Amplitude Panning
//  Panning compensated for head orientation, valid in ITD frequency range ~(0,1000)Hz
//

#include "CAP.h"

#include <boost/math/constants/constants.hpp>

#ifdef CAP_DEBUG_MESSAGES
#include <cstdio>
#endif

namespace visr
{
namespace panning
{
  
CAP::CAP()
 : m_array( 0 )
 , m_nSources( 0 )
{
}

int CAP::calcGains(){

    m_gain.zeroFill();
    
    // Abbreviated variables:
    // r    speaker normal vectors
    // rL   inter-aural axis normal vector
    // rI   image normal vector
    
    std::size_t const n = m_array->getNumRegularSpeakers();
    
    
    // Setup normal vectors from listener to speakers
    std::vector<XYZ> r;
    r.resize( n );
    
    for (std::size_t i = 0 ; i < n; i++) {
        r[i] = m_array->getPosition(i);
        r[i].minus(m_listenerPos);
        r[i].normalise();
    }
    
    
    XYZ & rL = m_listenerAuralAxis; // assume normalised.
    
    // printf("m_listenerAuralAxis %f %f %f\n", rL.x, rL.y, rL.z);
    
    if (n >= 3) { // Energy minimization
        
        std::vector<Afloat> a;
        a.resize(n);
        
        for(std::size_t j = 0; j < m_nSources; j++) {
            
            XYZ rI = m_sourcePos[j];
            
            Afloat rI0_ = rI.getLength();           // image - 0  distance
            
            if (!m_sourcePos[j].isInfinite) {     //!
                rI.minus(m_listenerPos);
            }
            
            Afloat rI_ = rI.getLength();  // rI_ : image-listener gain adjustment
            
            rI.normalise();               //! replace getLength to avoid length recalc
            
            Afloat b = 0, c = 0;
            
            for (std::size_t i = 0 ; i < n; i++) {
                XYZ rt = r[i];
                rt.minus(rI);
                a[i] = rL.dot(rt);
                
                b += a[i];
                c += a[i] * a[i];
            }
            
            Afloat d = (c*n - b*b)   * (rI_ + 0.01f) / rI0_;  // image distance correction added
            Afloat g;
            Afloat gMax = 0;
            
            Afloat gSum = 0;
            Afloat ggSum = 0;
            
            for (std::size_t i = 0 ; i < n; i++) {
                g = (-b * a[i] + c ) / d;
                if (g > gMax) gMax = g;
                gSum = gSum +g;
                ggSum = ggSum +g*g;
                m_gain(j, i) = g;
                //printf( g);
            }
            // printf("%f   %f\n", gSum, ggSum);
            
            // Limit gains
            if (gMax > m_maxGain) {
                Afloat f = m_maxGain / gMax;
                for (std::size_t i = 0 ; i < n; i++) {
                    m_gain(j, i) = m_gain(j, i) * f;
                }
            }

        }
    }
    else if (n == 2) { // For this case use the linear formula, because singularity limiting is simpler.
        
        for(std::size_t j = 0; j < m_nSources; j++) {
            
            XYZ rI = m_sourcePos[j];
            
            Afloat rI0_ = rI.getLength();            // image - 0  distance
            
            if (!m_sourcePos[j].isInfinite) {     //!
                rI.minus(m_listenerPos);
            }
            
            Afloat rI_ = rI.getLength();  // rI_ : image-listener gain adjustment
            
            rI.normalise();
            
            // if (j==2) printf("%f %f %f\n", rI.x, rI.y, rI.z); // gains for one object
            
            XYZ rt = rI;
            rt.minus(r[1]);
            Afloat g1 = rL.dot(rt);
                   
            rt = rI;
            rt.minus(r[0]);
            Afloat g2 = -rL.dot(rt);
            
            //Afloat d = g1 + g2;
            
            Afloat d = std::abs(g1 + g2); // denominator. abs() ensures continuity of normalised gains
            
            if (d < 0.001f) d = 0.001f;     // hard gain limit to avoid singularity
            
            g1 = g1 / d;
            g2 = g2 / d;
            
            
            // Experimental CAP - VBIP hybrid for HF
            if (m_HFmode) {
                if (g1*g2 > 0) { // both -ve or +ve
                    g1 = sqrt(g1); // gain remapping
                    g2 = sqrt(g2);
                    Afloat f = 1/sqrt(g1*g1 + g2*g2);
                    g1 = g1 * f; //  energy normalisation
                    g2 = g2 * f;
                }
                else {
                    g1 = 0;
                    g2 = 0;
                }
            }
            
            
            // Gain limiting, pre image/speaker distance compensation
            // - to make smooth transition through singularity
            Afloat gMax = std::abs(g1);
            if (std::abs(g2) > gMax) gMax = std::abs(g2);
            if (gMax > m_maxGainPreComp) {
                Afloat f = m_maxGainPreComp / gMax;
                g1 = g1 * f;
                g2 = g2 * f;
            }
            
            
            // Image/speaker distance compensation
            if (rI_ < 0.001f) rI_ = 0.001f;
            Afloat c = rI0_ / rI_;    // gain 1 when listener is at 0
                                      // Should replace rI0_ with rI1_ set in plugin - fixed unity-gain-distance parameter (rI0_ varies with pointsource distance).
            
            g1 = g1 * c;
            g2 = g2 * c;
            
        
            
            // Final gain limiting
            gMax = std::abs(g1);
            if (std::abs(g2) > gMax) gMax = std::abs(g2);
            if (gMax > m_maxGain) {
                Afloat f = m_maxGain / gMax;
                g1 = g1 * f;
                g2 = g2 * f;
            }
            
            // if (j==2) printf("%f %f\n", g1, g2); // gains for one object
            
            
            // use for latency test
//            if ( rL.z > 0 ) {
//                g1 = 1;
//                g2 = 1;
//            }
//            else {
//                g1 = 0;
//                g2 = 0;
//            }
            
            
            m_gain(j, 0) = g1;
            m_gain(j, 1) = g2;
            
            
        }
    }
    else { // n = 1
        for(std::size_t j = 0; j < m_nSources; j++) {
            m_gain(j, 0) = 1;
        }
    }

#ifdef CAP_DEBUG_MESSAGES
        printf("\n");
#endif
    return 0;
}
    
  
    
int CAP::setListenerOrientation(Afloat yaw, Afloat pitch, Afloat roll, bool zero){
        
        // zero = true  is used to set the identity orientation. (calibration / experimental)
        
        
//#ifdef CAP_DEBUG_MESSAGES
//      printf("setListenerOrientation ypr %f %f %f\n", yaw, pitch, roll);
//#endif
        
        // Convert to radians. !But shouldn't these be in radians already?
        //yaw = yaw * boost::math::constants::degree<Afloat>();
        //pitch = pitch * boost::math::constants::degree<Afloat>();
        //roll = roll * boost::math::constants::degree<Afloat>();
        
        
        Afloat cy = cos(yaw);
        Afloat sy = sin(yaw);
        Afloat cp = cos(pitch);
        Afloat sp = sin(pitch);
    
        Afloat cr = cos(roll);
        Afloat sr = sin(roll);

    
    
        // For Vive tracking system
        // m_listenerAuralAxis.set( cy*sp*sr-sy*cr, sy*sp*sr+cy*cr, cp*sr ); // standard
    
        m_listenerAuralAxis.set( cy*sp*sr-sy*cr, sy*sp*sr+cy*cr, -cp*sr );  // modified for -p, -r
    
        //printf("m_listenerAuralAxis %8.4f %8.4f %8.4f\n", m_listenerAuralAxis.x, m_listenerAuralAxis.y, m_listenerAuralAxis.z);
    
    
    
        // Kinect - Sparkfun IMU tracking system
        // m_listenerAuralAxis.set( -sy*cp, cy*cp, -sp );
    

    
        
        // // Experimental calibration stuff, double take allowing for arbitrary orientation of tracker on hat.
        //
        //        Afloat m[3][3]; // temp rotation matrix
        //
        //
        //        // phi the psi
        //
        //        Afloat cy = cos(yaw);
        //        Afloat sy = sin(yaw);
        //        Afloat cp = cos(pitch);
        //        Afloat sp = sin(pitch);
        //        Afloat cr = cos(roll);
        //        Afloat sr = sin(roll);
        //
        //
        //        m[0][0] = cp * cy;
        //        m[0][1] = cp * sy;
        //        m[0][2] = -sp; ??
        //
        //        m[1][0] = sr * sp * cy - cr * sy;
        //        m[1][1] = sr * sp * sy + cr * cy;
        //        m[1][2] = cp * sr;
        //
        //        m[2][0] = cr * sp * cy + sr * sy;
        //        m[2][1] = cr * sp * sy - sr * cy;
        //        m[2][2] = cp * cr;
        //
        //
        //        if (zero) { // Update reorientation matrix by finding the inverse of current matrix
        //            Afloat det = m[0][0] * ( m[1][1] * m[2][2] - m[2][1] * m[1][2] )
        //            - m[0][1] * ( m[1][0] * m[2][2] - m[1][2] * m[2][0] ) +
        //            m[0][2] * ( m[1][0] * m[2][1] - m[1][1] * m[2][0] );
        //
        //            Afloat invdet = 1 / det;
        //
        //            Afloat (*minv)[3] = m_reorientMatrix; // inverse of matrix m
        //            minv[0][0] = ( m[1][1] * m[2][2] - m[2][1] * m[1][2] ) * invdet;
        //            minv[0][1] = ( m[0][2] * m[2][1] - m[0][1] * m[2][2] ) * invdet;
        //            minv[0][2] = ( m[0][1] * m[1][2] - m[0][2] * m[1][1] ) * invdet;
        //            minv[1][0] = ( m[1][2] * m[2][0] - m[1][0] * m[2][2] ) * invdet;
        //            minv[1][1] = ( m[0][0] * m[2][2] - m[0][2] * m[2][0] ) * invdet;
        //            minv[1][2] = ( m[1][0] * m[0][2] - m[0][0] * m[1][2] ) * invdet;
        //            minv[2][0] = ( m[1][0] * m[2][1] - m[2][0] * m[1][1] ) * invdet;
        //            minv[2][1] = ( m[2][0] * m[0][1] - m[0][0] * m[2][1] ) * invdet;
        //            minv[2][2] = ( m[0][0] * m[1][1] - m[1][0] * m[0][1] ) * invdet;
        //        }
        //
        //        // Apply reorientation
        //
        //        Afloat m2[3][3]; // temp rotation matrix
        //
        //        int temp = 0;
        //        int i, j, k;
        //
        //        for(i = 0; i < 3; i++)
        //        {
        //            for(j = 0; j < 3; j++)
        //            {
        //                temp = 0;
        //                for(k = 0; k < 3; k++)
        //                {
        //                    temp += m_reorientMatrix[j][k] * m[k][i];
        //                }
        //                m2[j][i] = temp;
        //            }
        //        }
        //
        //
        //        m_listenerAuralAxis.set(m2[1][0], m2[1][1], m2[1][2]);
        //
        
        
        
        
        
        //        a11 = math.cos(the) * math.cos(phi)
        //        a12 = math.cos(the) * math.sin(phi)
        //        a13 = -math.sin(the)
        //
        //        a21 = math.sin(psi) * math.sin(the) * math.cos(phi) - math.cos(psi) * math.sin(phi)
        //        a22 = math.sin(psi) * math.sin(the) * math.sin(phi) + math.cos(psi) * math.cos(phi)
        //        a23 = math.cos(the) * math.sin(psi)
        //
        //        a31 = math.cos(psi) * math.sin(the) * math.cos(phi) + math.sin(psi) * math.sin(phi)
        //        a32 = math.cos(psi) * math.sin(the) * math.sin(phi) - math.sin(psi) * math.cos(phi)
        //        a33 = math.cos(the) * math.cos(psi)
        
        
        
        return 0;
    }

} // namespace panning
} // namespace visr
