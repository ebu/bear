{ lib, stdenv, cmake, ninja, python, boost, fetchzip, src, enable_python ? true }:
stdenv.mkDerivation rec {
  name = "VISR";
  inherit src;
  nativeBuildInputs = [ cmake ninja boost ] ++ lib.optional enable_python python;
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
