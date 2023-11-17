# Copyright Institute of Sound and Vibration Research - All rights reserved

# Package generation configuration

SET( CPACK_DEBIAN_PACKAGE_MAINTAINER "Andreas Franck A.Franck@soton.ac.uk" )
SET( CPACK_PACKAGE_NAME "VISR" )
SET( CPACK_PACKAGE_VENDOR "ISVR")

# Use the version numbers centrally defined for the VISR project
set( CPACK_PACKAGE_VERSION_MAJOR ${VISR_MAJOR_VERSION} )
set( CPACK_PACKAGE_VERSION_MINOR ${VISR_MINOR_VERSION} )
set( CPACK_PACKAGE_VERSION_PATCH ${VISR_PATCH_VERSION} )

set( PKG_FILE_NAME ${VISR_VERSIONED_NAME}-${CMAKE_SYSTEM_NAME})

# Encode the Python major/minor version in the package file name if Python is enabled.
if( BUILD_PYTHON_BINDINGS )
  set( PYTHON_MAJOR_MINOR
    ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR} )
  set( PKG_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-Python${PYTHON_MAJOR_MINOR}-${CMAKE_SYSTEM_NAME}" )
else( BUILD_PYTHON_BINDINGS )
  set( PKG_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${CMAKE_SYSTEM_NAME}" )
endif( BUILD_PYTHON_BINDINGS )

set( CPACK_PACKAGE_FILE_NAME ${PKG_FILE_NAME} )
set( CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)

if( WIN32 )
  set( CPACK_GENERATOR NSIS )
  set( CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
  set( CPACK_PACKAGE_INSTALL_DIRECTORY ${VISR_VERSIONED_NAME} )
  if( BUILD_AUDIOINTERFACES_PORTAUDIO )
    get_target_property( Portaudio_IMPORT_LIBRARY Portaudio::portaudio IMPORTED_IMPLIB )
    get_target_property( Portaudio_RUNTIME_LIBRARY Portaudio::portaudio IMPORTED_LOCATION )
    install( FILES ${Portaudio_IMPORT_LIBRARY} ${Portaudio_RUNTIME_LIBRARY}
             DESTINATION ${THIRDPARTY_LIBRARY_INSTALL_DIRECTORY} COMPONENT thirdparty_libraries )

    if( BUILD_INSTALL_STATIC_LIBRARIES OR BUILD_INSTALL_STATIC_PIC_LIBRARIES )
      install( FILES ${Portaudio_INCLUDE_DIRS}/portaudio.h
        DESTINATION "${VISR_TOPLEVEL_INSTALL_DIRECTORY}/3rd/include/"
        COMPONENT development_files )
    endif()
  endif( BUILD_AUDIOINTERFACES_PORTAUDIO )

  if( BUILD_USE_SNDFILE_LIBRARY )
    get_target_property( SndFile_IMPORT_LIBRARY SndFile::sndfile IMPORTED_IMPLIB )
    get_target_property( SndFile_RUNTIME_LIBRARY SndFile::sndfile IMPORTED_LOCATION )
    install( FILES ${SndFile_IMPORT_LIBRARY} ${SndFile_RUNTIME_LIBRARY}
             DESTINATION ${THIRDPARTY_LIBRARY_INSTALL_DIRECTORY} COMPONENT thirdparty_libraries )

    if( BUILD_INSTALL_STATIC_LIBRARIES OR BUILD_INSTALL_STATIC_PIC_LIBRARIES )
      install( FILES ${SndFile_INCLUDE_DIRS}/sndfile.h
        DESTINATION "${VISR_TOPLEVEL_INSTALL_DIRECTORY}/3rd/include/"
        COMPONENT development_files
    )
    endif()
  endif( BUILD_USE_SNDFILE_LIBRARY )

  # Boost
  if( NOT Boost_USE_STATIC_LIBS )
    foreach( BOOSTLIB ${VISR_BOOST_INSTALL_LIBRARIES} )
      # TODO: Use generator expression to select the right file depending on the config.
      get_target_property( BOOSTLIBPATH Boost::${BOOSTLIB} IMPORTED_LOCATION_RELEASE )
      get_filename_component( BOOSTLIBNAME ${BOOSTLIBPATH} NAME_WE )
      get_filename_component( BOOSTLIBDIR ${BOOSTLIBPATH} DIRECTORY )
      install( FILES ${BOOSTLIBDIR}/${BOOSTLIBNAME}.dll
               DESTINATION ${THIRDPARTY_LIBRARY_INSTALL_DIRECTORY} COMPONENT thirdparty_libraries)
    endforeach()
  endif( NOT Boost_USE_STATIC_LIBS )
endif( WIN32 )

if( VISR_SYSTEM_NAME MATCHES "Linux" )
  set( CPACK_GENERATOR DEB )
  set( CPACK_DEBIAN_HOMEPAGE "http://www.s3a-spatialaudio.org" )
  set( CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
  set( CPACK_DEBIAN_ARCHIVE_TYPE gnutar )
endif(VISR_SYSTEM_NAME MATCHES "Linux")

if( VISR_SYSTEM_NAME MATCHES "MacOS" )
  set( CPACK_GENERATOR productbuild )

  set( CPACK_PACKAGE_NAME ${PKG_FILE_NAME}.pkg )

  set( CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" )

  if( BUILD_AUDIOINTERFACES_PORTAUDIO )
    get_target_property( PORTAUDIO_LIBPATH Portaudio::portaudio
                         IMPORTED_LOCATION )
    install( FILES ${PORTAUDIO_LIBPATH}
             DESTINATION ${THIRDPARTY_LIBRARY_INSTALL_DIRECTORY} COMPONENT thirdparty_libraries)
  endif( BUILD_AUDIOINTERFACES_PORTAUDIO )
  if( BUILD_USE_SNDFILE_LIBRARY )
    get_target_property( SNDFILE_LIBPATH SndFile::sndfile
                         IMPORTED_LOCATION )
    install( FILES ${SNDFILE_LIBPATH}
             DESTINATION ${THIRDPARTY_LIBRARY_INSTALL_DIRECTORY} COMPONENT thirdparty_libraries)
    install( FILES ${FLAC_LIBRARY}
             DESTINATION ${THIRDPARTY_LIBRARY_INSTALL_DIRECTORY} COMPONENT thirdparty_libraries)
    install( FILES ${OGG_LIBRARY}
             DESTINATION ${THIRDPARTY_LIBRARY_INSTALL_DIRECTORY} COMPONENT thirdparty_libraries)
    install( FILES ${VORBIS_LIBRARY}
             DESTINATION ${THIRDPARTY_LIBRARY_INSTALL_DIRECTORY} COMPONENT thirdparty_libraries)
    install( FILES ${VORBISENC_LIBRARY}
             DESTINATION ${THIRDPARTY_LIBRARY_INSTALL_DIRECTORY} COMPONENT thirdparty_libraries)
  endif( BUILD_USE_SNDFILE_LIBRARY )
  if( NOT Boost_USE_STATIC_LIBS )
    foreach( BOOSTLIB ${VISR_BOOST_INSTALL_LIBRARIES} )
      get_target_property( BOOSTLIBPATH Boost::${BOOSTLIB} IMPORTED_LOCATION )
      install( FILES ${BOOSTLIBPATH}
               DESTINATION ${THIRDPARTY_LIBRARY_INSTALL_DIRECTORY} COMPONENT thirdparty_libraries)
    endforeach()
  endif( NOT Boost_USE_STATIC_LIBS )

  # Prepare a example launchagent file
  set( LAUNCHAGENT_PLIST_FILENAME ${VISR_VERSIONED_NAME}.plist )
  set( LAUNCHAGENT_PLIST_FILEPATH ${PROJECT_BINARY_DIR}/package_resources/${LAUNCHAGENT_PLIST_FILENAME} )
  configure_file( ${PROJECT_SOURCE_DIR}/cmake_modules/package_resources/VISR-launchagent.plist.in
                  ${LAUNCHAGENT_PLIST_FILEPATH}	@ONLY )
  # Hack: we treat the launchagents file as part of shared_libraries because we want to use the same script.
  install( FILES ${LAUNCHAGENT_PLIST_FILEPATH}
           DESTINATION ${VISR_TOPLEVEL_INSTALL_DIRECTORY}/etc COMPONENT shared_libraries )

  configure_file(${PROJECT_SOURCE_DIR}/cmake_modules/package_resources/productbuild_postscript.sh.in
  		       "${PROJECT_BINARY_DIR}/package_resources/productbuild_postscript.sh" @ONLY)
  set( CPACK_POSTFLIGHT_SHARED_LIBRARIES_SCRIPT
       ${PROJECT_BINARY_DIR}/package_resources/productbuild_postscript.sh)
endif( VISR_SYSTEM_NAME MATCHES "MacOS" )

install( DIRECTORY config DESTINATION ${VISR_TOPLEVEL_INSTALL_DIRECTORY} COMPONENT loudspeaker_configs )
install( FILES ${PROJECT_SOURCE_DIR}/LICENSE.md
               ${PROJECT_SOURCE_DIR}/Contributors.md
               ${PROJECT_SOURCE_DIR}/Readme.md
               ${PROJECT_SOURCE_DIR}/ChangeLog.txt
               DESTINATION ${VISR_TOPLEVEL_INSTALL_DIRECTORY} COMPONENT base )
# Allow for text substitution in the files and change names from .md to .txt (the former is not currently supported by CMake).
configure_file (${PROJECT_SOURCE_DIR}/LICENSE.md
        "${PROJECT_BINARY_DIR}/package_resources/LICENSE.txt" @ONLY)
set( CPACK_RESOURCE_FILE_LICENSE ${PROJECT_BINARY_DIR}/package_resources/LICENSE.txt )
configure_file (${PROJECT_SOURCE_DIR}/Readme.md
        "${PROJECT_BINARY_DIR}/package_resources/Readme.txt" @ONLY)
set( CPACK_RESOURCE_FILE_README ${PROJECT_BINARY_DIR}/package_resources/Readme.txt )
configure_file (${PROJECT_SOURCE_DIR}/cmake_modules/package_resources/welcome.txt.in
        "${PROJECT_BINARY_DIR}/package_resources/welcome.txt" @ONLY)
set(CPACK_RESOURCE_FILE_WELCOME ${PROJECT_BINARY_DIR}/package_resources/welcome.txt)


# Install Python example scripts and templates
if( BUILD_PYTHON_BINDINGS )
  install( DIRECTORY src/python/templates
           DESTINATION ${PYTHON_MODULE_INSTALL_DIRECTORY}
           COMPONENT python_templates
           PATTERN __pycache__ EXCLUDE )
  install( DIRECTORY src/python/packages/visr_bst
           DESTINATION ${PYTHON_MODULE_INSTALL_DIRECTORY}
           COMPONENT python_package_bst
           PATTERN __pycache__ EXCLUDE )
  install( DIRECTORY src/python/packages/metadapter
           DESTINATION ${PYTHON_MODULE_INSTALL_DIRECTORY}
           COMPONENT python_package_metadapter
           PATTERN __pycache__ EXCLUDE )

  # Copy the pybind11 directory into the installation tree
  # In this way the same pybind11 version can be used by dependent projects.
  # We omit the tests/ and docs/ subdirectries to save space.
  install( DIRECTORY ${PYBIND11_DIR}/
           DESTINATION "${VISR_TOPLEVEL_INSTALL_DIRECTORY}/3rd/include/pybind11"
           COMPONENT development_files
           PATTERN tests EXCLUDE
           PATTERN docs EXCLUDE )
endif( BUILD_PYTHON_BINDINGS )

# TODO: Decide whether this shall go into a separate component.
install( DIRECTORY src/python/packages/loudspeakerconfig
         DESTINATION ${PYTHON_MODULE_INSTALL_DIRECTORY}
         COMPONENT base
	 PATTERN __pycache__ EXCLUDE )

# CPack must be included after all install directives and CPACK_ variable definitions.
include( CPack )

# Define components and component groups for the component-enabled installers.
# This must happen after include( CPack )

# Installation types are ATM only supported by NSIS.
cpack_add_install_type( default DISPLAY_NAME "Default" )
cpack_add_install_type( developer DISPLAY_NAME "Developer" )
cpack_add_install_type( full DISPLAY_NAME "Full" )
if( BUILD_PYTHON_BINDINGS )
  cpack_add_install_type( python DISPLAY_NAME "Python developer" )
endif( BUILD_PYTHON_BINDINGS )

cpack_add_component( base
                     DISPLAY_NAME "Base"
                     DESCRIPTION "Basic required files."
                     REQUIRED HIDDEN
                   )

cpack_add_component( thirdparty_libraries
                     DISPLAY_NAME "Third-party libraries"
                     DESCRIPTION "Shared 3rd-party libraries."
                     REQUIRED HIDDEN
                   )

if( BUILD_INSTALL_SHARED_LIBRARIES )
  cpack_add_component(shared_libraries
                      DISPLAY_NAME "Shared Libraries"
                      DESCRIPTION "Core VISR libraries (shared)"
                      REQUIRED
                     )
endif( BUILD_INSTALL_SHARED_LIBRARIES )

if( BUILD_INSTALL_STATIC_LIBRARIES )
  cpack_add_component(static_libraries
                      DISPLAY_NAME "Static Libraries"
                      DESCRIPTION "Core VISR libraries (static)"
                      INSTALL_TYPES full developer
                      DISABLED
                     )
endif( BUILD_INSTALL_STATIC_LIBRARIES )

if( BUILD_INSTALL_STATIC_PIC_LIBRARIES )
  cpack_add_component(static_pic_libraries
                      DISPLAY_NAME "Static Libraries with position-independent code"
                      DESCRIPTION "Core VISR libraries (static and position-independent, for building loadable modules)"
                      INSTALL_TYPES full developer
                      DISABLED
                     )
endif( BUILD_INSTALL_STATIC_PIC_LIBRARIES )

cpack_add_component( standalone_applications
                    DISPLAY_NAME "Standalone applications"
                    DESCRIPTION "Standalone command-line applications"
                    INSTALL_TYPES default full
                    DEPENDS shared_libraries
                   )

cpack_add_component( standalone_applications
                    DISPLAY_NAME "Standalone applications"
                    DESCRIPTION "Standalone command-line applications"
                    INSTALL_TYPES default full
                    DEPENDS shared_libraries
                   )

if( BUILD_INSTALL_STATIC_LIBRARIES )
  cpack_add_component(development_files
                      DISPLAY_NAME "Development files"
                      DESCRIPTION "Header files and CMake support"
                      INSTALL_TYPES developer full
                      DEPENDS static_libraries
                      # DISABLED # Enable by default.
                     )
else( BUILD_INSTALL_STATIC_LIBRARIES )
  cpack_add_component(development_files
                      DISPLAY_NAME "Development files"
                      DESCRIPTION "Header files and CMake support"
                      INSTALL_TYPES developer full
                      # DISABLED # Enable by default
                     )
endif( BUILD_INSTALL_STATIC_LIBRARIES )

if( BUILD_PYTHON_BINDINGS )
cpack_add_component( python_externals
                    DISPLAY_NAME "Python Externals"
                    DESCRIPTION "Python modules to access the VISR functionality from Python"
                    INSTALL_TYPES default python full
                   )

cpack_add_component_group( python_packages
                          DISPLAY_NAME "Python packages"
                          DESCRIPTION "VISR extension packages implemented in Python"
                          )

cpack_add_component( python_package_bst
                    DISPLAY_NAME "Binaural Synthesis Toolkit"
                    DESCRIPTION "Toolbox for binaural synthesis"
                    INSTALL_TYPES default python full
                    DEPENDS python_externals
                    GROUP python_packages
                   )

cpack_add_component( python_package_metadapter
                    DISPLAY_NAME "Metadapter"
                    DESCRIPTION "Extensible system for object metadata adaptation"
                    INSTALL_TYPES default python full
                    DEPENDS python_externals
                    GROUP python_packages
                   )

cpack_add_component( python_templates
                    DISPLAY_NAME "Python Templates"
                    DESCRIPTION "Python template files"
                    DEPENDS python_externals
                    INSTALL_TYPES python full
                   )
endif( BUILD_PYTHON_BINDINGS )

if( BUILD_MAX_MSP_EXTERNALS )
cpack_add_component( max_externals
                    DISPLAY_NAME "Max/MSP externals"
                    DESCRIPTION "Multichannel DSP externals for Max/MSP"
                    INSTALL_TYPES full
                    DEPENDS shared_libraries
                   )
endif( BUILD_MAX_MSP_EXTERNALS )

cpack_add_component_group( documentation
                          DISPLAY_NAME "Documentation"
                          DESCRIPTION "User and API documentation"
                          )

cpack_add_component( user_documentation_pdf
                    DISPLAY_NAME "User documentation (PDF)"
                    DESCRIPTION "User documentation"
                    INSTALL_TYPES default developer full
                    GROUP documentation
                   )

cpack_add_component( api_documentation_pdf
                    DISPLAY_NAME "API documentation (PDF)"
                    DESCRIPTION "Code reference documentation"
                    INSTALL_TYPES developer full
                    GROUP documentation
                   )

cpack_add_component( doxygen_documentation_html
                    DISPLAY_NAME "Doxygen code documentation"
                    DESCRIPTION "Code reference documentation in Doxygen format (deprecated)"
                    INSTALL_TYPES full
                    DISABLED
                    GROUP documentation
                   )

cpack_add_component( loudspeaker_configs
                    DISPLAY_NAME "Loudspeaker configuration files"
                    DESCRIPTION "Example configuration files for the loudspeaker renderers."
                    INSTALL_TYPES default developer full
                   )
