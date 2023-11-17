{ stdenv, python, fetchFromGitHub, src }:
python.pkgs.buildPythonPackage rec {
  name = "ear";
  inherit src;
  propagatedBuildInputs = with python.pkgs; [ numpy scipy six attrs multipledispatch lxml pyyaml setuptools ];
  doCheck = false;
}
