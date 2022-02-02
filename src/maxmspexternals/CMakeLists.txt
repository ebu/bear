# Copyright Institute of Sound and Vibration Research - All rights reserved

# This is not possible because the static boost libraries are normally not
# compiled as position-independent code, which means they cannot used in shared
# objects as Max externals.
# This definitely leads to link errors on Linux and MacOS, but it should
# conceptually be the same problem on Windows.
if( Boost_USE_STATIC_LIBS )
  message( SEND_ERROR
    "Cannot build Max/MSP externals when using static boost libraries" )
endif( Boost_USE_STATIC_LIBS )

# Generic C++ classes to ease the implementation of Max/MSP external objects.
ADD_SUBDIRECTORY( libmaxsupport )

ADD_SUBDIRECTORY( gain_matrix )
ADD_SUBDIRECTORY( delay_vector )
ADD_SUBDIRECTORY( matrix_convolver )
ADD_SUBDIRECTORY( visr_renderer )
