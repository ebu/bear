{ stdenv, python, fetchFromGitHub }:
python.pkgs.buildPythonPackage rec {
  name = "ear";
  src = fetchFromGitHub {
    owner = "ebu";
    repo = "ebu_adm_renderer";
    rev = "a0e37d33f55ae7080b1aaccbce655680319f92ae";
    sha256 = "sha256-FelQWGcLJbB6ZnadM0/5hrEhZMVGsssqhaPS4xbO2Ws=";
  };
  propagatedBuildInputs = with python.pkgs; [ numpy scipy six attrs multipledispatch lxml ruamel_yaml setuptools ];
  doCheck = false;
  postPatch = ''
    # latest attrs should be fine...
    sed -i "s/'attrs.*'/'attrs'/" setup.py
  '';
}
