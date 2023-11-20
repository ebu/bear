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
    substituteInPlace CMakeLists.txt \
      --replace "\''${VISR_TOPLEVEL_INSTALL_DIRECTORY}/python" "$out/${python.sitePackages}"
  '';
}
