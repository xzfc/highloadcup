{ nixpkgs ? import <nixpkgs> {} }:
let
  overrideCC = nixpkgs.overrideCC;
  stdenv = overrideCC nixpkgs.stdenv nixpkgs.gcc6;
  boost = nixpkgs.boost.override { stdenv = stdenv; };
in rec {
  myProject = stdenv.mkDerivation {
    name = "my-project";
    nativeBuildInputs = [
      boost
    ];
  };
}
