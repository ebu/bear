include(ExternalProject)

# data file handling

# The list of available data files and their details (url, hash) in a separate
# file so that they can be more easily used by external tools. Ideally this
# would be JSONor similar, but that requires quite a modern cmake version

# do_download sets up a download target using ExternalProject_Add; this is used
# if BEAR_DOWNLOAD_DATA_${name} is set and an external path is not provided with
# BEAR_DATA_PATH_${name}. Either way, BEAR_DATA_OUTPUT_PATH_${name} is set to
# the path to the file, for use in other projects

include(files.txt)

function(do_download name)
  ExternalProject_Add(
    tensorfile_${name}
    PREFIX "tensorfile_${name}"
    URL "${${name}_url}"
    URL_HASH SHA256=${${name}_sha256}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DOWNLOAD_NO_EXTRACT true
    DOWNLOAD_NAME "${${name}_fname}")
  ExternalProject_Get_Property(tensorfile_${name} DOWNLOADED_FILE)
  set(DOWNLOADED_FILE
      ${DOWNLOADED_FILE}
      PARENT_SCOPE)
endfunction()

if(BEAR_DOWNLOAD_DATA_DEFAULT AND (NOT BEAR_DATA_PATH_DEFAULT))
  do_download(default)
  set(BEAR_DATA_OUTPUT_PATH_DEFAULT
      ${DOWNLOADED_FILE}
      CACHE INTERNAL "BEAR data file location, for use in other projects")
else()
  set(BEAR_DATA_OUTPUT_PATH_DEFAULT
      ${BEAR_DATA_PATH_DEFAULT}
      CACHE INTERNAL "BEAR data file location, for use in other projects")
endif()

if(BEAR_DOWNLOAD_DATA_DEFAULT_SMALL AND (NOT BEAR_DATA_PATH_DEFAULT_SMALL))
  do_download(default_small)
  set(BEAR_DATA_OUTPUT_PATH_DEFAULT_SMALL
      ${DOWNLOADED_FILE}
      CACHE INTERNAL "small BEAR data file location, for use in other projects")
else()
  set(BEAR_DATA_OUTPUT_PATH_DEFAULT_SMALL
      ${BEAR_DATA_PATH_DEFAULT_SMALL}
      CACHE INTERNAL "small BEAR data file location, for use in other projects")
endif()
