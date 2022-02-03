#! /usr/bin/env nix-shell
#! nix-shell --pure -i bash -p nixpkgs-fmt
cd $(dirname $0)
nixpkgs-fmt .
