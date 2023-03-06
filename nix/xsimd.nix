# adapted from https://github.com/NixOS/nixpkgs/blob/master/pkgs/development/libraries/xsimd/default.nix
# MIT licensed
{ lib, stdenv, fetchFromGitHub, cmake, doctest }:
stdenv.mkDerivation rec {
  pname = "xsimd";
  version = "10.0.0";
  src = fetchFromGitHub {
    owner = "xtensor-stack";
    repo = "xsimd";
    rev = version;
    sha256 = "sha256-+ewKbce+rjNWQ0nQzm6O4xSwgzizSPpDPidkQYuoSTU=";
  };

  nativeBuildInputs = [ cmake doctest ];

  cmakeFlags = [ "-DBUILD_TESTS=ON" ];

  doCheck = true;
  checkTarget = "xtest";
  GTEST_FILTER =
    let
      # Upstream Issue: https://github.com/xtensor-stack/xsimd/issues/456
      filteredTests = lib.optionals stdenv.hostPlatform.isDarwin [
        "error_gamma_test/*"
      ];
    in
    "-${builtins.concatStringsSep ":" filteredTests}";
}
