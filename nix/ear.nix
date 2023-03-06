{ stdenv, python, fetchFromGitHub, src }:
python.pkgs.buildPythonPackage rec {
  name = "ear";
  inherit src;
  propagatedBuildInputs = with python.pkgs; [ numpy scipy six attrs multipledispatch lxml ruamel_yaml setuptools ];
  doCheck = false;
  postPatch = ''
    # latest attrs should be fine...
    sed -i "s/'attrs.*'/'attrs'/" setup.py
  '';
}
