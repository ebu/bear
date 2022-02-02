# Copyright Institute of Sound and Vibration Research - All rights reserved

# This is not possible because the static boost libraries are normally not
# compiled as position-independent code, which means they cannot used in shared
# objects as Matlab externals.
# This definitely leads to link errors on Linux and MacOS, but it should
# conceptually be the same problem on Windows.
if( Boost_USE_STATIC_LIBS )
  message( SEND_ERROR
    "Cannot build Matlab externals when using static boost libraries" )
endif( Boost_USE_STATIC_LIBS )

# Simple external to test the functionality of the mex interface
# ADD_SUBDIRECTORY( biquad_iir_filter )
# ADD_SUBDIRECTORY( delay_vector )
ADD_SUBDIRECTORY( feedthrough )
# ADD_SUBDIRECTORY( gain_matrix )
# ADD_SUBDIRECTORY( late_reverb_filter_calculator )
# ADD_SUBDIRECTORY( multichannel_convolver )
ADD_SUBDIRECTORY( time_frequency_feedthrough )
