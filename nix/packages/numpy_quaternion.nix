{ python }:
python.pkgs.buildPythonPackage rec {
  pname = "numpy-quaternion";
  version = "2021.11.4.15.26.3";

  src = python.pkgs.fetchPypi {
    inherit pname version;
    sha256 = "b0dc670b2adc8ff2fb8d6105a48769873f68d6ccbe20af6a19e899b1e8d48aaf";
  };

  propagatedBuildInputs = with python.pkgs; [ numpy ];
  doCheck = false;
}
