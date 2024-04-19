{ lib, src, python, ear, visr_python ? null, numpy_quaternion, visr_bear ? null }:
python.pkgs.buildPythonPackage rec {
  name = "bear";
  inherit src;
  propagatedBuildInputs = with python.pkgs; [ numpy ear importlib-resources visr_python visr_bear ];
  nativeBuildInputs = with python.pkgs; [ setuptools ];
  nativeCheckInputs = [ python.pkgs.pytest numpy_quaternion ];
  checkPhase = lib.optionalString (visr_bear != null) ''
    pytest visr_bear/test
  '';
}
