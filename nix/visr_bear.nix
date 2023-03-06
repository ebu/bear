{ stdenv, src, cmake, boost, python, visr_python, data_files, eigen, libear, rapidjson }:
stdenv.mkDerivation rec {
  name = "visr_bear";
  inherit src;
  nativeBuildInputs = [ cmake boost python.buildEnv visr_python eigen libear rapidjson ];
  cmakeFlags = [
    "-DBEAR_PACKAGE_AND_INSTALL=false"
    "-DBEAR_UNIT_TESTS=true"
    "-DBEAR_DATA_PATH_DEFAULT=${data_files.default.file}"
    "-DBEAR_DATA_PATH_DEFAULT_SMALL=${data_files.default_small.file}"
    "-DBEAR_SYMLINK_DATA_FILES=true"
    "-DBEAR_USE_INTERNAL_LIBEAR=false"
    "-DBEAR_USE_INTERNAL_RAPIDJSON=false"
  ];
  preConfigure = ''cmakeFlags="$cmakeFlags -DBEAR_PYTHON_SITE_PACKAGES=$out/${python.sitePackages}"'';

  doCheck = true;
  checkPhase = "make test";
}
