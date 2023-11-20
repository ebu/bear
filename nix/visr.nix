{ stdenv, cmake, python, boost, fetchzip, src }:
stdenv.mkDerivation rec {
  name = "VISR";
  inherit src;
  nativeBuildInputs = [ cmake boost python.buildEnv ];
  cmakeFlags = [
    "-DBUILD_AUDIOINTERFACES_PORTAUDIO=FALSE"
    "-DBUILD_USE_SNDFILE_LIBRARY=FALSE"
    "-DBUILD_DOCUMENTATION=FALSE"
    "-DBUILD_STANDALONE_APPLICATIONS=false"
  ];
  postPatch = ''
    sed -i "s%set( PYTHON_MODULE_INSTALL_DIRECTORY .*$%set( PYTHON_MODULE_INSTALL_DIRECTORY \"$out/${python.sitePackages}\")%" CMakeLists.txt
    sed -i 's/ADD_SUBDIRECTORY( apps )//' src/CMakeLists.txt
  '';
}
