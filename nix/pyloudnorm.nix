{ python, fetchFromGitHub }:
python.pkgs.buildPythonPackage rec {
  pname = "pyloudnorm";
  version = "master";

  src = fetchFromGitHub {
    owner = "csteinmetz1";
    repo = "pyloudnorm";
    rev = "1fb914693e07f4bea06fdb2e4bd2d6ddc1688a9e";
    sha256 = "sha256-eDQt3OmXir5xaSUmXdvItk1d2PWigV8QOZAqpr6EaE8=";
  };

  propagatedBuildInputs = with python.pkgs; [ numpy scipy ];
  nativeCheckInputs = with python.pkgs; [ pytestCheckHook soundfile ];
}
