{ lib, fetchFromGitHub }:
let
  gitignoreSrc = fetchFromGitHub {
    owner = "hercules-ci";
    repo = "gitignore.nix";
    rev = "5b9e0ff9d3b551234b4f3eb3983744fa354b17f1";
    sha256 = "sha256-o/BdVjNwcB6jOmzZjOH703BesSkkS5O7ej3xhyO8hAY=";
  };

  inherit (import gitignoreSrc { inherit lib; }) gitignoreFilter;

  filterGitDirs = src:
    let
      srcIgnored = gitignoreFilter src;
    in
    path: type:
      srcIgnored path type
      && builtins.baseNameOf path != ".git";
in
src:
lib.cleanSourceWith {
  filter = filterGitDirs src;
  inherit src;
}
