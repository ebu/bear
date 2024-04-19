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

  preCheck = ''
    export LD_LIBRARY_PATH=$(pwd)/lib
    export DYLD_LIBRARY_PATH=$(pwd)/lib
  '';

  nativeCheckInputs = lib.optionals enable_python [
    python.pkgs.numpy
    python.pkgs.scipy
    python.pkgs.pytest
  ];

  doCheck = true;
}
