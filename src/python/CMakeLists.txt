# Copyright Institute of Sound and Vibration Research - All rights reserved

# This is not possible because the static boost libraries are normally not
# compiled as position-independent code, which means they cannot used in shared
# objects as Python externals.
# This definitely leads to link errors on Linux and MacOS, but it should
# conceptually be the same problem on Windows.
if( Boost_USE_STATIC_LIBS )
  message( SEND_ERROR
   "Cannot build Python bindings when using static boost libraries." )
endif( Boost_USE_STATIC_LIBS )

add_subdirectory( visr )

add_subdirectory( efl )

add_subdirectory( signalflows )

add_subdirectory( rbbl )

add_subdirectory( rcl )

add_subdirectory( rrl )

add_subdirectory( panning )

add_subdirectory( panningdsp )

add_subdirectory( pml )

add_subdirectory( objectmodel )

add_subdirectory( reverbobject )

add_subdirectory( audiointerfaces )

add_subdirectory( libpythonbindinghelpers )

# "Meta-target" combining all Python externals to serve as a single dependency.
add_custom_target( pythonexternals
  DEPENDS visrpython eflpython signalflowspython rbblpython
  rclpython rrlpython panningpython pmlpython
  objectmodelpython objectmodelpython reverbobjectpython
  audiointerfacespython )
set_target_properties( pythonexternals PROPERTIES FOLDER python_externals )
