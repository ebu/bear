{ lib, stdenv, cmake, python, boost, fetchzip, src, enable_python ? true }:
stdenv.mkDerivation rec {
  name = "VISR";
  inherit src;
  nativeBuildInputs = [ cmake boost ] ++ lib.optional enable_python python.buildEnv;
  cmakeFlags = [
    "-DBUILD_AUDIOINTERFACES_PORTAUDIO=FALSE"
    "-DBUILD_USE_SNDFILE_LIBRARY=FALSE"
    "-DBUILD_DOCUMENTATION=FALSE"
    "-DBUILD_STANDALONE_APPLICATIONS=false"
  ] ++ lib.optional (!enable_python) "-DBUILD_PYTHON_BINDINGS=false";
  postPatch = ''
    substituteInPlace CMakeLists.txt \
      --replace "\''${VISR_TOPLEVEL_INSTALL_DIRECTORY}/python" "$out/${python.sitePackages}"
  '';
}
