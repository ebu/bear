{ pkgs ? import
    (fetchTarball {
      url = "https://github.com/NixOS/nixpkgs/archive/cff83d5032a21aad4f69bf284e95b5f564f4a54e.tar.gz";
      sha256 = "0gcm6k52mfazykhs5s4548qxa25sgf3vafb8jy051ayi9i7076wa";
    })
    { }
}: with pkgs; callPackage ./packages { python = python3; }
