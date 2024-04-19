{ stdenv, python, src }:
python.pkgs.buildPythonPackage rec {
  name = "ear";
  inherit src;
  propagatedBuildInputs = with python.pkgs; [ numpy scipy six attrs multipledispatch lxml pyyaml importlib-resources ];
  nativeBuildInputs = with python.pkgs; [ setuptools ];
  pyproject = true;

  doCheck = false;
}
