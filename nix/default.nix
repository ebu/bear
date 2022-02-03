{ pkgs ? import
    (fetchTarball {
      url = "https://github.com/NixOS/nixpkgs/archive/ceaca998029d4b1ac7fd2bee985a3b0f37a786b5.tar.gz";
      sha256 = "02wa4bw6s800b9inn8hznr56v4v8x3y44sj9lwmkw9zbxzw6mi7s";
    })
    { }
}: with pkgs; callPackage ./packages { python = python3; }
