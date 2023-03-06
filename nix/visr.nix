{ stdenv, cmake, python, boost, fetchzip }:
stdenv.mkDerivation rec {
  name = "VISR";
  src = fetchzip {
    url = "https://github.com/ebu/bear/releases/download/v0.0.1-pre/visr-0.13.0-pre-5e13f020.zip";
    sha256 = "1jgrzjlh5kqbq06vz6pnn6yvml1bqjk33rblqfqja7v02b0pzv07";
  };
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
