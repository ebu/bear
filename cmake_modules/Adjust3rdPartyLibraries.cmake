# Copyright Institute of Sound and Vibration Research - All rights reserved


function(fix_rpath libpath )
  get_filename_component( libname ${${libpath}} NAME )
  set (EXECUTE_COMMAND  cp ${${libpath}} ${PROJECT_BINARY_DIR}/3rd/${libname})
  execute_process(COMMAND ${EXECUTE_COMMAND} RESULT_VARIABLE rv)
  set (EXECUTE_COMMAND  chmod 777 ${PROJECT_BINARY_DIR}/3rd/${libname})
  execute_process(COMMAND ${EXECUTE_COMMAND} RESULT_VARIABLE rv)
  get_filename_component( libname ${${libpath}} NAME )
  set( ${libpath} ${VISR_BUILD_3RD_PARTY_RUNTIME_LIBRARY_DIR}/${libname})
  set( ${libpath} ${VISR_BUILD_3RD_PARTY_RUNTIME_LIBRARY_DIR}/${libname} PARENT_SCOPE )
  set (EXECUTE_COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id @rpath/${libname} ${${libpath}})
  message( STATUS " output " ${libname} ": " ${${libpath}} )
  execute_process(COMMAND ${EXECUTE_COMMAND} RESULT_VARIABLE rv)
endfunction()

function( fixLibraryTarget target )
  get_target_property( LIBLOCATION ${target} IMPORTED_LOCATION )
  message( STATUS "fixLibraryTarget: " ${target} " liblocation: " ${LIBLOCATION} )
  get_filename_component( LIBREALPATH ${LIBLOCATION} REALPATH)
  fix_rpath( LIBREALPATH )
  get_filename_component( LIBNAME ${LIBREALPATH} NAME )
  set( ADJUSTEDLIBPATH ${PROJECT_BINARY_DIR}/3rd/${LIBNAME})
  set_target_properties( ${target} PROPERTIES
                         IMPORTED_LOCATION ${ADJUSTEDLIBPATH}
			 IMPORTED_LOCATION_DEBUG ${ADJUSTEDLIBPATH}
			 IMPORTED_LOCATION_RELEASE ${ADJUSTEDLIBPATH})
endfunction()

# Copy a file to a destination folder and set permissions suitable for a shared libary
function( copyLibrary libName targetDir )
  # message( STATUS "copyLibrary: " ${libName} " " ${targetDir} )
  file( COPY ${libName} DESTINATION ${targetDir}
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
	GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_WRITE WORLD_EXECUTE )
endfunction()

function( copyDllFromLibName libPath targetDir )
  # message( STATUS "copyDllFromLibName: " ${libPath} " " "${targetDir}" )
  get_filename_component( INPUT_LIBNAME ${libPath} NAME_WE )
  get_filename_component( INPUT_DIRECTORY ${libPath} DIRECTORY )
  set( INPUT_FILE_NAME ${INPUT_DIRECTORY}/${INPUT_LIBNAME}${CMAKE_SHARED_LIBRARY_SUFFIX} )
  copyLibrary( ${INPUT_FILE_NAME} ${targetDir} )
endfunction()

function( copyDllFromTarget tgt targetDir )
  get_target_property( LIBLOCATION ${tgt} IMPORTED_LOCATION_RELEASE )
  if( NOT LIBLOCATION )
      message( STATUS "target lib: " ${tgt} " IMPORTED_LOCATION_RELEASE not found." )
      get_target_property( LIBLOCATION ${tgt} IMPORTED_LOCATION )
  endif( NOT LIBLOCATION )
  message( STATUS "target lib: " " liblocation: " ${LIBLOCATION} )
  get_filename_component( LIBREALPATH ${LIBLOCATION} REALPATH)
  copyDllFromLibName( ${LIBREALPATH} ${targetDir} )

  # Take into account if there's a dedicated debug library.
  # In this case copy both into the target directory.
  get_target_property( DEBUGLIBLOCATION ${tgt} IMPORTED_LOCATION_DEBUG )
  if( DEBUGLIBLOCATION )
    get_filename_component( DEBUGLIBREALPATH ${DEBUGLIBLOCATION} REALPATH)
    if( NOT ${DEBUGLIBREALPATH} MATCHES ${LIBREALPATH} )
      copyDllFromLibName( ${DEBUGLIBREALPATH} ${targetDir} )
    endif( NOT ${DEBUGLIBREALPATH} MATCHES ${LIBREALPATH} )
  endif( DEBUGLIBLOCATION )
endfunction()

function( fixBoostLibrary BOOSTLIB )
  get_target_property( LIBLOCATION Boost::${BOOSTLIB} IMPORTED_LOCATION_RELEASE )
  # message( STATUS "lib: " ${BOOSTLIB} " liblocation: " ${LIBLOCATION} )
  get_filename_component( LIBREALPATH ${LIBLOCATION} REALPATH)
  fix_rpath( LIBREALPATH )
  get_filename_component( LIBNAME ${LIBREALPATH} NAME )
  set( ADJUSTEDLIBPATH ${PROJECT_BINARY_DIR}/3rd/${LIBNAME})
  set_target_properties( Boost::${BOOSTLIB} PROPERTIES
                         IMPORTED_LOCATION ${ADJUSTEDLIBPATH}
			 IMPORTED_LOCATION_DEBUG ${ADJUSTEDLIBPATH}
			 IMPORTED_LOCATION_RELEASE ${ADJUSTEDLIBPATH})
endfunction()

################################################################################
# copying 3rd party libraries in "3rd" folder, and changing the ID to @rpath/<libraryname>.dylib.

file( MAKE_DIRECTORY ${VISR_BUILD_3RD_PARTY_RUNTIME_LIBRARY_DIR} )

# List of 3rd party libraries that are not boost.
# More specifically, these are not imported targets as preferred in modern CMake.
set( FIX_LIBRARIES )

set( FIX_LIBRARY_TARGETS )

if( BUILD_AUDIOINTERFACES_PORTAUDIO )
  list( APPEND FIX_LIBRARY_TARGETS Portaudio::portaudio )
endif( BUILD_AUDIOINTERFACES_PORTAUDIO )

if( BUILD_USE_SNDFILE_LIBRARY )
  list( APPEND FIX_LIBRARY_TARGETS SndFile::sndfile )
  # On MacOS, sndfile depends on FLAC, OGG, and Vorbis.
  if( VISR_SYSTEM_NAME MATCHES "MacOS" )
   list( APPEND FIX_LIBRARIES FLAC_LIBRARY OGG_LIBRARY VORBIS_LIBRARY VORBISENC_LIBRARY )
  endif( VISR_SYSTEM_NAME MATCHES "MacOS" )
endif( BUILD_USE_SNDFILE_LIBRARY )

# On Mac OS, the Python library must also be treated because of the rpath.
if( BUILD_PYTHON_BINDINGS AND (VISR_SYSTEM_NAME MATCHES "MacOS") )
 list( APPEND FIX_LIBRARIES PYTHON_LIBRARIES )
endif( BUILD_PYTHON_BINDINGS AND (VISR_SYSTEM_NAME MATCHES "MacOS") )

if(VISR_SYSTEM_NAME MATCHES "MacOS")
  foreach(v ${FIX_LIBRARIES} )
    fix_rpath(${v})
    message( STATUS "FIX_LIBRARIES " ${v} ": " ${${v}} )
  endforeach()
  foreach(v ${FIX_LIBRARY_TARGETS} )
    fixLibraryTarget(${v})
    message( STATUS "FIX_LIBRARY_TARGET " ${v} ": " ${${v}} )
  endforeach()
  # Do the same for the boost libs.
  if( NOT Boost_USE_STATIC_LIBS )
    foreach( BOOSTLIB ${VISR_BOOST_LIBRARIES} )
      fixBoostLibrary( ${BOOSTLIB} )
    endforeach()
  endif( NOT Boost_USE_STATIC_LIBS )
endif(VISR_SYSTEM_NAME MATCHES "MacOS")

if(VISR_SYSTEM_NAME MATCHES "Windows")
# Unfortunately, there are no libraries where the standard procedure works, because
#
#  foreach(v ${FIX_LIBRARIES} )
#    copyDllFromLibName( ${${v}} ${VISR_BUILD_3RD_PARTY_RUNTIME_LIBRARY_DIR} )
#  endforeach()
  if( NOT Boost_USE_STATIC_LIBS )
    foreach( BOOST_LIBRARY IN LISTS VISR_BOOST_LIBRARIES )
      copyDllFromTarget( Boost::${BOOST_LIBRARY} ${VISR_BUILD_3RD_PARTY_RUNTIME_LIBRARY_DIR} )
    endforeach()
  endif( NOT Boost_USE_STATIC_LIBS )
  if( BUILD_AUDIOINTERFACES_PORTAUDIO )
    copyDllFromTarget( Portaudio::portaudio ${VISR_BUILD_3RD_PARTY_RUNTIME_LIBRARY_DIR} )
  endif( BUILD_AUDIOINTERFACES_PORTAUDIO )
  if( BUILD_USE_SNDFILE_LIBRARY )
    copyDllFromTarget( SndFile::sndfile ${VISR_BUILD_3RD_PARTY_RUNTIME_LIBRARY_DIR} )
  endif( BUILD_USE_SNDFILE_LIBRARY )
endif(VISR_SYSTEM_NAME MATCHES "Windows")

################################################################################
# Fixing internal dependencies of third party libraries.
# This is no longer required for boost libraries when using the imported target "Boost::<libraryname>" syntax
function(fix_dependencies_of_3rdparty depname libpath)
  set (EXECUTE_COMMAND bash "-c" "${PROJECT_SOURCE_DIR}/cmake_modules/./change_dependency_installname.sh ${depname} ${libpath}" )
  execute_process(COMMAND ${EXECUTE_COMMAND} OUTPUT_VARIABLE rv)
endfunction()

function(fix_dependencies_of_3rdparty_target depname target)
  get_target_property( LIBLOCATION ${target} IMPORTED_LOCATION )
  fix_dependencies_of_3rdparty( ${depname} ${LIBLOCATION} )
endfunction()

if( VISR_SYSTEM_NAME MATCHES "MacOS" AND BUILD_USE_SNDFILE_LIBRARY )
  get_filename_component(FLAC_LIBRARY_NAME ${FLAC_LIBRARY} NAME_WE)
  get_filename_component(OGG_LIBRARY_NAME ${OGG_LIBRARY} NAME_WE)
  get_filename_component(VORBIS_LIBRARY_NAME ${VORBIS_LIBRARY} NAME_WE)
  get_filename_component(VORBISENC_LIBRARY_NAME ${VORBISENC_LIBRARY} NAME_WE)

  fix_dependencies_of_3rdparty_target(${FLAC_LIBRARY_NAME} SndFile::sndfile)
  fix_dependencies_of_3rdparty_target(${OGG_LIBRARY_NAME} SndFile::sndfile)
  fix_dependencies_of_3rdparty_target(${VORBIS_LIBRARY_NAME} SndFile::sndfile)
  fix_dependencies_of_3rdparty_target(${VORBISENC_LIBRARY_NAME} SndFile::sndfile)
  fix_dependencies_of_3rdparty(${OGG_LIBRARY_NAME} ${FLAC_LIBRARY})
  fix_dependencies_of_3rdparty(${OGG_LIBRARY_NAME} ${VORBIS_LIBRARY})
  fix_dependencies_of_3rdparty(${OGG_LIBRARY_NAME} ${VORBISENC_LIBRARY})
  fix_dependencies_of_3rdparty(${VORBIS_LIBRARY_NAME} ${VORBISENC_LIBRARY})
endif( VISR_SYSTEM_NAME MATCHES "MacOS" AND BUILD_USE_SNDFILE_LIBRARY )
