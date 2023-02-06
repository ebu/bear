{ stdenv, python, fetchFromGitHub }:
python.pkgs.buildPythonPackage rec {
  name = "ear";
  src = fetchFromGitHub {
    owner = "ebu";
    repo = "ebu_adm_renderer";
    rev = "ef2189021203101eab323e1eccdd2527b32a5024";
    sha256 = "wkzfmqqnY5Nzw3Rr5kpnpIZ5kuQygUxHidMN9QZEI/Q=";
  };
  propagatedBuildInputs = with python.pkgs; [ numpy scipy six attrs multipledispatch lxml ruamel_yaml setuptools ];
  doCheck = false;
  postPatch = ''
    # latest attrs should be fine...
    sed -i "s/'attrs.*'/'attrs'/" setup.py
  '';
}
