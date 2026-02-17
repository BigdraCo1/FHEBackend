{
  description = "FHEbackend – Drogon + OpenFHE + OpenCV dev environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };

        # ── Custom OpenFHE derivation (not in nixpkgs) ──────────────────────
        openfhe = pkgs.stdenv.mkDerivation rec {
          pname = "openfhe";
          version = "1.4.2";

          src = pkgs.fetchgit {
            url = "https://github.com/openfheorg/openfhe-development.git";
            rev = "v${version}";
            hash = "sha256-AWctCovIU1+lLOCpN2SKJAdToSwV8cPok9sBb7e+wCc=";
            fetchSubmodules = true;
          };

          nativeBuildInputs = with pkgs; [ cmake ];
          buildInputs = with pkgs;
            [ llvmPackages.openmp ]
            ++ pkgs.lib.optionals pkgs.stdenv.hostPlatform.isLinux [ gmp ];

          cmakeFlags = [
            "-DCMAKE_BUILD_TYPE=Release"
            "-DBUILD_EXAMPLES=OFF"
            "-DBUILD_BENCHMARKS=OFF"
            "-DBUILD_UNITTESTS=OFF"
            "-DWITH_OPENMP=ON"
          ];

          meta = with pkgs.lib; {
            description = "Open-source FHE library";
            homepage = "https://www.openfhe.org";
            license = licenses.bsd2;
            platforms = platforms.unix;
          };
        };

        # ── Platform-specific packages ──────────────────────────────────────
        isLinux = pkgs.stdenv.hostPlatform.isLinux;
        isDarwin = pkgs.stdenv.hostPlatform.isDarwin;

        linuxOnlyPkgs = with pkgs; pkgs.lib.optionals isLinux [
          valgrind
          gdb
          util-linux  # libuuid
        ];

        darwinOnlyPkgs = with pkgs; pkgs.lib.optionals isDarwin [
          lldb
        ];

      in
      {
        devShells.default = pkgs.mkShell {
          name = "fhebackend-dev";

          packages = with pkgs; [
            # ── Build tools ─────────────────────────────────────────────────
            cmake
            gnumake
            pkg-config
            gcc
            clang-tools          # clang-format, clang-tidy

            # ── Core libraries ──────────────────────────────────────────────
            openfhe
            (drogon.override { withYaml = true; })
            opencv

            # ── Drogon transitive deps ──────────────────────────────────────
            jsoncpp
            openssl
            zlib
            c-ares
            brotli
            hiredis
            yaml-cpp
            llvmPackages.openmp

          ] ++ linuxOnlyPkgs ++ darwinOnlyPkgs;

          shellHook = ''
            echo "🔐 FHEbackend dev shell"
            echo "   OpenFHE ${openfhe.version} | Drogon | OpenCV"
            echo "   System: ${system}"
            echo ""
            echo "   Build:  cmake -S . -B build && cmake --build build -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)"
            echo "   Test:   cd build && ctest"
          '';
        };
      }
    );
}
