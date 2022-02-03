{ stdenv, src, cmake, boost, python, visr_python, data_files }:
stdenv.mkDerivation rec {
  name = "visr_bear";
  inherit src;
  sourceRoot = "source/visr_bear";
  nativeBuildInputs = [ cmake boost python.buildEnv visr_python ];
  cmakeFlags = [
    "-DBEAR_PACKAGE_AND_INSTALL=false"
    "-DBEAR_UNIT_TESTS=false"
    "-DBEAR_DATA_PATH_DEFAULT=${data_files.default.file}"
    "-DBEAR_DATA_PATH_DEFAULT_SMALL=${data_files.default_small.file}"
    "-DBEAR_SYMLINK_DATA_FILES=true"
  ];
  preConfigure = ''cmakeFlags="$cmakeFlags -DBEAR_PYTHON_SITE_PACKAGES=$out/${python.sitePackages}"'';
}
