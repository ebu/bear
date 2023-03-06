{ src, runCommand, cmake, fetchurl }:
let
  # use a cmake script to extract the file names, hashes and urls as a nix
  # expression
  script = ./extract_files.txt;
  files_data_nix = runCommand "list_files" { inherit src; } ''
    ${cmake}/bin/cmake -P ${script}
  '';
  files_data = (import files_data_nix);
in
{
  # add file attributes with the downloaded file
  files = builtins.mapAttrs
    (name: data@{ url, sha256, fname, ... }:
      data // {
        file = fetchurl {
          inherit url sha256;
          name = fname;
        };
      })
    files_data;
}
