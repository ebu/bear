{ src, python, ear, visr_python, visr_bear, numpy_quaternion }:
python.pkgs.buildPythonPackage rec {
  name = "bear";
  inherit src;
  propagatedBuildInputs = with python.pkgs; [ numpy ear setuptools visr_python visr_bear ];
  checkInputs = [ python.pkgs.pytest numpy_quaternion ];
  checkPhase = ''
    pytest visr_bear/test
  '';
}
