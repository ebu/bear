{ callPackage, src, python, visr_python, ear, numpy_quaternion }: rec {
  data_files = (callPackage ./data_files.nix { inherit src; }).files;
  visr_bear = callPackage ./visr_bear.nix { inherit python src data_files visr_python; };
  bear = callPackage ./bear.nix { inherit python visr_python src visr_bear ear numpy_quaternion; };
}
