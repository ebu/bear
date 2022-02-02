//
//  XYZ.h
//
//  Created by Dylan Menzies on 09/12/2014.
//  Copyright (c) ISVR, University of Southampton. All rights reserved.
//

#ifndef __S3A_renderer_dsp__XYZ__
#define __S3A_renderer_dsp__XYZ__

#include "defs.h"

#include "export_symbols.hpp"

#include <iostream>
#include <cmath>

namespace visr
{
namespace panning
{

class VISR_PANNING_LIBRARY_SYMBOL XYZ {
 public:
 /**
  * Default constructor.
  */
    XYZ();
  /**
   * Constructor with specified coordinates.
   */
  explicit XYZ( Afloat pX, Afloat pY, Afloat pZ, bool pInfinity = false )
    : x( pX), y( pY ), z( pZ ), isInfinite( pInfinity )
  {}
 
  /**
   * Assignment operator. Uses default-constructed operator (no implementation required).
   */
  XYZ & operator=( XYZ const & rhs ) = default;
 
  /**
   * Copy constructor.
   * Uses automatically-generated default copy ctor, no implementation required.
   */
  XYZ( XYZ const & rhs ) = default;
  
    
  Afloat x, y, z;
  bool isInfinite; // eg for source at infinity.
    
    
  int set(Afloat X, Afloat Y, Afloat Z, bool inf = false) {
    x = X; y = Y; z =Z;
    isInfinite = inf;
    return 0;
  };
    
  Afloat getLength() const {
    return std::sqrt(x*x + y*y + z*z);
  }
    
  Afloat dot( XYZ v) const {
        return x * v.x + y * v.y + z * v.z;
  }

  int minus( XYZ v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return 0;
  }

  /**
   * Re-scale the vector to unit length.
   * @return The length of the vector before the normalisation.
   */
  Afloat normalise() {
    Afloat l = getLength();
    if (l != 0.0f ) {
      x /= l;
      y /= l;
      z /= l;
    }
    return l;
  }
};

} // namespace panning
} // namespace visr

#endif /* defined(__S3A_renderer_dsp__XYZ__) */
