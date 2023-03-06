{ stdenv, cmake, ninja, boost, fetchFromGitHub, eigen, xsimd, src }:
stdenv.mkDerivation {
  name = "libear";
  inherit src;
  nativeBuildInputs = [ cmake ninja boost eigen xsimd ];
  cmakeFlags = [ "-DEAR_UNIT_TESTS=OFF" "-DEAR_EXAMPLES=OFF" "-DEAR_USE_INTERNAL_EIGEN=OFF" "-DEAR_USE_INTERNAL_XSIMD=OFF" ];
}
