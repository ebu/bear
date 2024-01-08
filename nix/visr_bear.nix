{ lib, stdenv, src, cmake, ninja, boost, python, visr ? null, visr_python ? null, data_files, eigen, libear, rapidjson, enable_python ? true }:
stdenv.mkDerivation rec {
  name = "visr_bear";
  inherit src;
  nativeBuildInputs = [
    cmake
    ninja
    boost
    eigen
    libear
    rapidjson
  ] ++ (if enable_python then [ python.buildEnv visr_python ] else [ visr ]);
  cmakeFlags = [
    "-DBEAR_UNIT_TESTS=true"
    "-DBEAR_DATA_PATH_DEFAULT=${data_files.default.file}"
    "-DBEAR_DATA_PATH_DEFAULT_SMALL=${data_files.default_small.file}"
    "-DBEAR_SYMLINK_DATA_FILES=true"
    "-DBEAR_USE_INTERNAL_LIBEAR=false"
    "-DBEAR_USE_INTERNAL_RAPIDJSON=false"
  ] ++ lib.optional (!enable_python) "-DBUILD_PYTHON_BINDINGS=false";
  preConfigure = ''cmakeFlags="$cmakeFlags -DBEAR_PYTHON_SITE_PACKAGES=$out/${python.sitePackages}"'';

  doCheck = true;
}
