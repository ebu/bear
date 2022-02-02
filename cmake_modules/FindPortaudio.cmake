# - Try to find Portaudio
# Once done this will define
#
#  Portaudio_FOUND - system has Portaudio
#  Portaudio_INCLUDE_DIRS - the Portaudio include directory
#  Portaudio_LIBRARIES - Link these to use Portaudio
#  Portaudio_DEFINITIONS - Compiler switches required for using Portaudio
#  Portaudio_VERSION - Portaudio version
#
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
# Redistribution and use is allowed according to the terms of the New BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (Portaudio_LIBRARIES AND Portaudio_INCLUDE_DIRS)
  # in cache already
  set(Portaudio_FOUND TRUE)
else (Portaudio_LIBRARIES AND Portaudio_INCLUDE_DIRS)
  if (NOT WIN32)
   find_package(PkgConfig REQUIRED)
   pkg_check_modules(Portaudio2 portaudio-2.0)
  endif (NOT WIN32)

  if (Portaudio2_FOUND)
    set(Portaudio_INCLUDE_DIRS
      ${Portaudio2_INCLUDEDIR}
    )
    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set(Portaudio_LIBRARIES "${Portaudio2_LIBRARY_DIRS}/lib${Portaudio2_LIBRARIES}.dylib")
    else (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set(Portaudio_LIBRARIES
        ${Portaudio2_LINK_LIBRARIES}
      )
    endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(Portaudio_VERSION
      19
    )
    set(Portaudio_FOUND TRUE)
  else (Portaudio2_FOUND)
    find_path(Portaudio_INCLUDE_DIR
      NAMES
        portaudio.h
      PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
        /sw/include
    )
   
    find_library(Portaudio_LIBRARY
      NAMES
        portaudio
      PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /sw/lib
    )
   
    find_path(Portaudio_LIBRARY_DIR
      NAMES
        portaudio
      PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /sw/lib
    )
   
    set(Portaudio_INCLUDE_DIRS
      ${Portaudio_INCLUDE_DIR}
    )
    set(Portaudio_LIBRARIES
      ${Portaudio_LIBRARY}
    )
   
    set(Portaudio_LIBRARY_DIRS
      ${Portaudio_LIBRARY_DIR}
    )
   
    set(Portaudio_VERSION
      18
    )
   
    if (Portaudio_INCLUDE_DIRS AND Portaudio_LIBRARIES)
       set(Portaudio_FOUND TRUE)
    endif (Portaudio_INCLUDE_DIRS AND Portaudio_LIBRARIES)
   
    if (Portaudio_FOUND)
      if (NOT Portaudio_FIND_QUIETLY)
        message(STATUS "Found Portaudio: ${Portaudio_LIBRARIES}")
      endif (NOT Portaudio_FIND_QUIETLY)
    else (Portaudio_FOUND)
      if (Portaudio_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find Portaudio")
      endif (Portaudio_FIND_REQUIRED)
    endif (Portaudio_FOUND)
  endif (Portaudio2_FOUND)


  # show the Portaudio_INCLUDE_DIRS and Portaudio_LIBRARIES variables only in the advanced view
  mark_as_advanced(Portaudio_FOUND Portaudio_INCLUDE_DIRS Portaudio_LIBRARIES)

endif (Portaudio_LIBRARIES AND Portaudio_INCLUDE_DIRS)

# ${Portaudio_LIBRARIES} may contain multiple libraries, but 
# the IMPORTED LOCATION and IMPORTED_IMPLIB target properties support
# only the main library,
list( GET Portaudio_LIBRARIES 0 Portaudio_MAIN_LIBRARY)

add_library( Portaudio::portaudio SHARED IMPORTED )
set_target_properties( Portaudio::portaudio PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${Portaudio_INCLUDE_DIRS}
)
if( WIN32 )
  get_filename_component( PORTAUDIO_LIB_DIRECTORY ${Portaudio_LIBRARIES} DIRECTORY )
  set_target_properties( Portaudio::portaudio PROPERTIES
    IMPORTED_LOCATION ${PORTAUDIO_LIB_DIRECTORY}/portaudio_x64.dll
    IMPORTED_IMPLIB  ${Portaudio_MAIN_LIBRARY}
  )
else()
  set_target_properties( Portaudio::portaudio PROPERTIES
    IMPORTED_LOCATION ${Portaudio_MAIN_LIBRARY}
  )
endif( WIN32 )
