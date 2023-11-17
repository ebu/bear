# Find module for libsndfile.
# Adapted from https://github.com/andresy/lua---sndfile/blob/master/cmake/FindSndFile.cmake
# Found on http://hg.kvats.net

# Andreas Franck <andreas.franck@audioscenic.com>

#
# - Try to find libsndfile
#
# Once done this will define
#
#  Target SndFile::sndfile
#  Variable SndFile_FOUND - system has libsndfile
#
#  SndFile_INCLUDE_DIRS - the libsndfile include directory
#  SndFile_LIBRARIES - Link these to use libsndfile
#
#  Copyright (C) 2006  Wengo
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

if (SndFile_LIBRARIES AND SndFile_INCLUDE_DIRS)
  # in cache already
  set(SndFile_FOUND TRUE)
else (SndFile_LIBRARIES AND SndFile_INCLUDE_DIRS)

  find_path(SndFile_INCLUDE_DIR
    NAMES
      sndfile.h
    PATHS
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )

  find_library(SndFile_LIBRARY
    NAMES
      libsndfile libsndfile-1
      sndfile sndfile-1
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  set(SndFile_INCLUDE_DIRS
    ${SndFile_INCLUDE_DIR}
  )
  set(SndFile_LIBRARIES
    ${SndFile_LIBRARY}
  )

  if (SndFile_INCLUDE_DIRS AND SndFile_LIBRARIES)
    set(SndFile_FOUND TRUE)
  endif (SndFile_INCLUDE_DIRS AND SndFile_LIBRARIES)

  if (SndFile_FOUND)
    if (NOT SndFile_FIND_QUIETLY)
      message(STATUS "Found libsndfile: ${SndFile_LIBRARIES}")
    endif (NOT SndFile_FIND_QUIETLY)
  else (SndFile_FOUND)
    if (SndFile_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find libsndfile")
    endif (SndFile_FIND_REQUIRED)
  endif (SndFile_FOUND)

  # show the SndFile_INCLUDE_DIRS and SndFile_LIBRARIES variables only in the advanced view
  mark_as_advanced(SndFile_INCLUDE_DIRS SndFile_LIBRARIES)

endif (SndFile_LIBRARIES AND SndFile_INCLUDE_DIRS)

add_library(SndFile::sndfile SHARED IMPORTED)
set_target_properties( SndFile::sndfile PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${SndFile_INCLUDE_DIRS}
)

# TODO: On Windows, find the DLL file and assign it to the IMPORTED_LOCATION
# target property

if( WIN32 )
  set_target_properties( SndFile::sndfile PROPERTIES
    IMPORTED_IMPLIB ${SndFile_LIBRARIES}
  )
  get_filename_component( _SndFile_LIBRARY_DIR ${SndFile_LIBRARIES} DIRECTORY )
  get_filename_component( _SndFile_LIBRARY_NAME_WE ${SndFile_LIBRARIES} NAME_WE )
  get_filename_component( _SndFile_DLL_PATH ${_SndFile_LIBRARY_DIR}/${_SndFile_LIBRARY_NAME_WE}.dll ABSOLUTE )
  set_target_properties( SndFile::sndfile PROPERTIES
    IMPORTED_LOCATION ${_SndFile_DLL_PATH} )
else()
set_target_properties( SndFile::sndfile PROPERTIES
  IMPORTED_LOCATION ${SndFile_LIBRARIES}
)
endif( WIN32 )
