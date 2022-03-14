{ lib, callPackage, python }: rec {
  numpy_quaternion = callPackage ./numpy_quaternion.nix { inherit python; };

  visr = callPackage ./visr.nix { inherit python; };
  visr_python = python.pkgs.toPythonModule visr;

  cleanSourceGit = callPackage ./gitignore.nix { };

  ear = callPackage ./ear.nix { inherit python; };

  bear_src = cleanSourceGit ../..;
  # drop irrelevant directories to save some rebuilds
  bear_src_tidy = lib.cleanSourceWith {
    filter = (path: type:
      let
        basename = builtins.baseNameOf path;
      in
      basename != "nix" && basename != ".github");
    src = bear_src;
  };

  bear_pkgs = callPackage ./bear { inherit python visr_python ear numpy_quaternion; src = bear_src_tidy; };
  inherit (bear_pkgs) data_files visr_bear bear bear_docker;
}
