{
  description = "JustManage manage your system actions";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }: let
    system = "x86_64-linux";
    pkgs = nixpkgs.legacyPackages.${system};
  in {
    packages.${system}.default = pkgs.stdenv.mkDerivation {
      pname = "jm";
      version = "0.1.0";
      src = self;
      buildInputs = [ pkgs.stdenv.cc ];
      buildPhase = "make";
      installPhase = ''
        runHook preInstall
        install -Dm755 jm $out/bin/jm
        runHook postInstall
      '';
    };
  };
}
