# Copyright (c) 2018 Institute of Sound and Vibration Research, University of Southampton.
# All rights reserved.

# Doxygen documentation building
# Even though Doxygen-generated documentation will not be deployed in the long term,
# the Sphinx-generated C++ API documentation uses Doxygen in the background to extract information
# from C++ code and stores it in XML format, which is picked up by the Breathe for Sphinx package

# Define paths needed for sphinx/autodoc documentation

# The path containing extension packages written in Python, for example visr_bst.
file( TO_NATIVE_PATH ${PROJECT_SOURCE_DIR}/src/python/packages VISR_PYTHON_PACKAGE_PATH )
# Path containing the VISR Python externals (in C++)
# Define the separator between multiple directories in environment variables, e.g., PATH
# or PYTHONPATH. We would prefer a CMake-provided solution.
if( WIN32 )
  set( VISR_ENVIRONMENT_PATH_SEPARATOR "$<SEMICOLON>" ) # Apparently semicolons need to be escaped in CMake.
else( WIN32 )
  set( VISR_ENVIRONMENT_PATH_SEPARATOR ":" )
endif( WIN32 )

# Create a variable that contains all environment variable settings for running a Python process (e.g., Sphinx documentation building or Python-based unit testing).
# TODO: Consider moving out of doc/ into a more central location.
if( BUILD_PYTHON_BINDINGS )
  # Include the location of the C++ externals and the pure Python modules.
  # the visrpython target is used because all externals are in the same directory.
  set( VISR_PYTHON_EXEC_ENVIRONMENT PYTHONPATH="$<SHELL_PATH:$<TARGET_FILE_DIR:visrpython>>${VISR_ENVIRONMENT_PATH_SEPARATOR}${VISR_PYTHON_PACKAGE_PATH}" )
else()
  # Only the pure Python packages are used in this case.
  set( VISR_PYTHON_EXEC_ENVIRONMENT PYTHONPATH="{VISR_PYTHON_PACKAGE_PATH}" )
endif( BUILD_PYTHON_BINDINGS )
# Ensure that the shared libraries are found.
if(VISR_SYSTEM_NAME MATCHES "MacOS")
  # On Mac OS, this is required if rpaths are not used at build time.
  # TODO: Are there cases where we have to add the VISR_BUILD_3RD_PARTY_RUNTIME_LIBRARY_DIR as well?
  list( APPEND VISR_PYTHON_EXEC_ENVIRONMENT DYLD_LIBRARY_PATH="$<SHELL_PATH:$<TARGET_FILE_DIR:visr_shared>>" )
endif(VISR_SYSTEM_NAME MATCHES "MacOS")
if(VISR_SYSTEM_NAME MATCHES "Windows")
  # The VISR DLLs as well as the 3rd party libraries must be added to the path.
  list( APPEND VISR_PYTHON_EXEC_ENVIRONMENT PATH="$<SHELL_PATH:$<TARGET_FILE_DIR:visr_shared>>${VISR_ENVIRONMENT_PATH_SEPARATOR}$<SHELL_PATH:${VISR_BUILD_3RD_PARTY_RUNTIME_LIBRARY_DIR}>${VISR_ENVIRONMENT_PATH_SEPARATOR}%PATH%" )
endif(VISR_SYSTEM_NAME MATCHES "Windows")
# No special handling is required on Linux, because the paths of dependent shared libraries are automatically encoded in the binaries.

# Set the dependencies needed to build the documentation. For sphinx.autodoc,
# the Python externals need to be built before the documentation can be extracted.
# We use the meta-target 'pythonexternals' that combines all externals,
# defined in ${CMAKE_SOURCE_DIR}/src/python/CMakeLists.txt
if( BUILD_PYTHON_BINDINGS)
  set( VISR_USERDOC_DEPENDENCIES doxygen-doc pythonexternals )
  set( VISR_APIDOC_DEPENDENCIES doxygen-doc pythonexternals )
else( BUILD_PYTHON_PYTHON_BINDINGS)
  set( VISR_USERDOC_DEPENDENCIES doxygen-doc ) # Used as dummy to prevent error later on if the variable is empty
  set( VISR_APIDOC_DEPENDENCIES doxygen-doc )
endif( BUILD_PYTHON_BINDINGS)

# Debug message:
# message( STATUS "VISR_PYTHON_PATH: " ${VISR_PYTHON_PATH} )

find_package(Doxygen)
if(NOT DOXYGEN_FOUND)
  #TODO add installation line guides for installing Doxygen via website www. ....
  message(FATAL_ERROR "Doxygen is needed to build the documentation.")
endif()

# Latex is used to create PDFs from the sphinx documentation
find_package( LATEX COMPONENTS PDFLATEX )

set( doxyfile_in          ${PROJECT_SOURCE_DIR}/doc/doxygen/Doxyfile.in )
set( doxyfile             ${PROJECT_BINARY_DIR}/doc/doxygen/Doxyfile )
set( doxy_html_index_file ${PROJECT_BINARY_DIR}/doc/doxygen/html/index.html )
set( doxy_output_root     ${PROJECT_BINARY_DIR}/doc/doxygen/ )
set( doxy_input           ${PROJECT_SOURCE_DIR}/src )
set( doxy_extra_files ${PROJECT_SOURCE_DIR}/doc/doxygen )
file( GLOB doxy_extra_file_list ${doxy_extra_files}/*.dox )
# Used for checking dependencies when using Sphinx custom commands
set( doxy_xml_index_file ${PROJECT_BINARY_DIR}/doc/doxygen/xml/index.xml )
# Used as a variable in config.py.in with Sphinx to configure the breathe extension.
set( doxy_xml_index       ${PROJECT_BINARY_DIR}/doc/doxygen/xml/ )

configure_file( ${doxyfile_in} ${doxyfile} @ONLY )

add_custom_command( OUTPUT ${doxy_html_index_file}
		  COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
		  # The following should be ${doxyfile} only but it
		  # will break the dependency.
		  # The optimal solution would be creating a
		  # custom_command for ${doxyfile} generation
		  # but I still have to figure out how...
		  MAIN_DEPENDENCY ${doxyfile} ${doxyfile_in}
		  DEPENDS ${doxy_extra_file_list}
		  COMMENT "Generating Doxygen HTML and XML documentation")

add_custom_target( doxygen-doc DEPENDS ${doxy_html_index_file}  )
set_target_properties( doxygen-doc PROPERTIES FOLDER documentation )

# "Meta-target" to trigger build of all documentation outputs
add_custom_target( doc ALL DEPENDS doxygen-doc )
set_target_properties( doc PROPERTIES FOLDER documentation )

find_package(Sphinx)
if(NOT Sphinx_FOUND)
  message(FATAL_ERROR "Sphinx is required to build documentation. Please set the SPHINX_EXECUTABLE variable.")
endif(NOT Sphinx_FOUND)

# TODO Do we need this ? Look in virtualenv and node_modules directories of the build directory (as
# per the hints in the documentation about installing sphinx, breathe,
# sphinx_js and jsdoc).
set(BUILD_VIRTUALENV_PATH ${PROJECT_BINARY_DIR}/venv/bin/)
list(INSERT CMAKE_PROGRAM_PATH 0 ${BUILD_VIRTUALENV_PATH})


set (userdoc_source_root    ${PROJECT_SOURCE_DIR}/doc/userdoc/)
set (apidoc_source_root    ${PROJECT_SOURCE_DIR}/doc/apidoc/)
set( userdoc_output_root   ${PROJECT_BINARY_DIR}/doc/userdoc )
set( apidoc_output_root   ${PROJECT_BINARY_DIR}/doc/apidoc )

if(NOT DEFINED SPHINX_THEME)
    set(SPHINX_THEME default)
endif()

if(NOT DEFINED SPHINX_THEME_DIR)
    set(SPHINX_THEME_DIR)
endif()

# Function to define a target building HTML documentation from a source directory containing Sphinx rst code.
function( HtmlDocBuild targetName sourceDir outputDir depends )
  configure_file( ${sourceDir}/conf.py.in ${outputDir}/html/conf.py @ONLY )
  # Define an empty file that gets updated when the target is build, such that it can be used as
  # a dependency for other targets.
  set( STAMP ${outputDir}/htmlbuild.stamp )
  # Super-ugly code duplication because the generator expression cannot be pulled out of the function call.
  add_custom_command(
    OUTPUT ${STAMP}
    DEPENDS doxygen-doc ${outputDir}/html/conf.py ${depends}
    COMMAND ${CMAKE_COMMAND} -E env ${VISR_PYTHON_EXEC_ENVIRONMENT}
              ${SPHINX_EXECUTABLE}
               -a
               -q
               -b html
               -c ${outputDir}/html
               -d ${outputDir}/html-cache
                ${sourceDir}
                ${outputDir}/html
     COMMAND ${CMAKE_COMMAND} -E touch ${STAMP}
     WORKING_DIRECTORY ${outputDir}/html
     COMMENT "HTML generation for target ${targetName}-html."
    )
  add_custom_target( ${targetName}-html DEPENDS ${STAMP} )
  set_target_properties( ${targetName}-html PROPERTIES ADDITIONAL_CLEAN_FILES ${outputDir}/html-cache )
  set_target_properties( ${targetName}-html PROPERTIES FOLDER documentation )
  set_target_properties( ${targetName}-html PROPERTIES ADDITIONAL_CLEAN_FILES ${outputDir}/html-cache )
endfunction(HtmlDocBuild)

# Function to define a target building PDF documentation from a source directory containing Sphinx rst code.
function( PdfDocBuild targetName sourceDir outputDir depends )
  configure_file( ${sourceDir}/conf.py.in ${outputDir}/pdf/conf.py @ONLY )
  # Name (with version) of the output file. This is the main dependency of the target.
  set( OUTPUTFILE ${outputDir}/${targetName}-${VISR_MAJOR_VERSION}.${VISR_MINOR_VERSION}.${VISR_PATCH_VERSION}.pdf )
  # Super-ugly code duplication because the generator expression cannot be pulled out of the function call.
  add_custom_command(
    OUTPUT ${OUTPUTFILE}
    DEPENDS doxygen-doc ${outputDir}/pdf/conf.py ${depends}
    COMMAND ${CMAKE_COMMAND} -E env ${VISR_PYTHON_EXEC_ENVIRONMENT}
            ${SPHINX_EXECUTABLE}
               -q
               -b latex
               -c ${outputDir}/pdf
               -d ${outputDir}/pdf-cache
                ${sourceDir}
                ${outputDir}/pdf
    COMMAND ${PDFLATEX_COMPILER} -halt-on-error visr
    COMMAND ${PDFLATEX_COMPILER} -halt-on-error visr
    COMMAND ${PDFLATEX_COMPILER} -halt-on-error visr
    COMMAND ${CMAKE_COMMAND} -E rename ${outputDir}/pdf/visr.pdf ${outputDir}/${targetName}-${VISR_MAJOR_VERSION}.${VISR_MINOR_VERSION}.${VISR_PATCH_VERSION}.pdf
    WORKING_DIRECTORY ${outputDir}/pdf
    COMMENT "PDF compilation for target ${targetName}-pdf."
    )
  add_custom_target( ${targetName}-pdf DEPENDS ${OUTPUTFILE} )
  set_target_properties( ${targetName}-pdf PROPERTIES ADDITIONAL_CLEAN_FILES ${outputDir}/pdf-cache )
  set_target_properties( ${targetName}-pdf PROPERTIES FOLDER documentation )
  set_target_properties( ${targetName}-pdf PROPERTIES ADDITIONAL_CLEAN_FILES ${outputDir}/pdf-cache )
endfunction(PdfDocBuild)

PdfDocBuild( userdoc ${userdoc_source_root} ${userdoc_output_root} "${VISR_USERDOC_DEPENDENCIES}" )
PdfDocBuild( apidoc ${apidoc_source_root} ${apidoc_output_root} "${VISR_APIDOC_DEPENDENCIES}" )

HtmlDocBuild( userdoc ${userdoc_source_root} ${userdoc_output_root} "${VISR_USERDOC_DEPENDENCIES}" )
HtmlDocBuild( apidoc ${apidoc_source_root} ${apidoc_output_root} "${VISR_APIDOC_DEPENDENCIES}" )

# Add the Sphinx-generated targets to the "doc" meta-target
add_dependencies( doc apidoc-html apidoc-pdf userdoc-html userdoc-pdf )

# Include the Doxygen documentation in the package installer
# Note: Doxygen as output format is deprecated, so it won't be externally visible in the future.
install( DIRECTORY ${PROJECT_BINARY_DIR}/doc/doxygen/html
         DESTINATION ${DOCUMENTATION_INSTALL_DIRECTORY}
	 COMPONENT doxygen_documentation_html )

# Place the generated PDF documentation in the installers.
# NOTE: Unfortuanately we need to operate on files, as installing for targets apparently works only for executables and libraries (which have a predefined main file)
# TODO: Remove construction of file name with full version string.
install( FILES  ${userdoc_output_root}/userdoc-${VISR_MAJOR_VERSION}.${VISR_MINOR_VERSION}.${VISR_PATCH_VERSION}.pdf DESTINATION ${DOCUMENTATION_INSTALL_DIRECTORY} COMPONENT user_documentation_pdf )
install( FILES  ${apidoc_output_root}/apidoc-${VISR_MAJOR_VERSION}.${VISR_MINOR_VERSION}.${VISR_PATCH_VERSION}.pdf DESTINATION ${DOCUMENTATION_INSTALL_DIRECTORY} COMPONENT api_documentation_pdf )
