{
  inputs.flake-utils.url = "github:numtide/flake-utils";
  inputs.nixpkgs.url = "nixpkgs/nixos-22.11";

  inputs.ear_src = {
    type = "git";
    url = "https://github.com/ebu/ebu_adm_renderer.git";
    ref = "master";
    flake = false;
  };

  inputs.libear_src = {
    type = "git";
    url = "https://github.com/ebu/libear.git";
    ref = "master";
    flake = false;
  };

  outputs = { self, nixpkgs, flake-utils, ear_src, libear_src }:
    flake-utils.lib.eachDefaultSystem
      (system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
          python = pkgs.python3;

          src = pkgs.lib.cleanSourceWith {
            filter = (path: type:
              let
                basename = builtins.baseNameOf path;
              in
              basename != "nix" && basename != ".github" && !pkgs.lib.hasPrefix "flake." basename);
            src = ./.;
          };
          visr_bear_src = ./visr_bear;
        in
        rec {
          packages = rec {
            data_files = (pkgs.callPackage ./nix/data_files { inherit src; }).files;

            numpy_quaternion = pkgs.callPackage ./nix/numpy_quaternion.nix { inherit python; };
            xsimd = pkgs.callPackage ./nix/xsimd.nix { };

            ear = pkgs.callPackage ./nix/ear.nix { inherit python; src = ear_src; };
            libear = pkgs.callPackage ./nix/libear.nix { inherit xsimd; src = libear_src; };

            visr = pkgs.callPackage ./nix/visr.nix { inherit python; };
            visr_python = python.pkgs.toPythonModule visr;

            visr_bear = pkgs.callPackage ./nix/visr_bear.nix { inherit python data_files libear visr_python; src = visr_bear_src; };
            bear = pkgs.callPackage ./nix/bear.nix { inherit python src visr_bear ear numpy_quaternion visr_python; };
            bear_docker = pkgs.callPackage ./nix/bear_docker.nix { inherit ear bear; };
          } // {
            # for checking source filter
            src = pkgs.runCommand "src" { } "cp -r ${src} $out";
          };

          devShells = rec {
            visr_bear = packages.visr_bear.overrideAttrs (attrs: {
              nativeBuildInputs = attrs.nativeBuildInputs ++ [
                pkgs.clang-tools
                pkgs.cmake-format
                pkgs.black
              ];
            });
          };
        });
}
