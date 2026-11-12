{
  description = "StreamDock Python and C++ SDK development environment for NixOS";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { nixpkgs, ... }:
    let
      systems = [ "x86_64-linux" "aarch64-linux" ];
      forAllSystems = nixpkgs.lib.genAttrs systems;
    in {
      devShells = forAllSystems (system:
        let
          pkgs = import nixpkgs { inherit system; };
          python = pkgs.python3.withPackages (ps: with ps; [
            pip
            setuptools
            wheel
            pillow
            pyudev
            # Optional SDK video dependency: provides cv2 for MP4 playback.
            opencv4
          ]);
          libraries = with pkgs; [
            stdenv.cc.cc.lib
            systemd
            libusb1
            hidapi
            giflib
            opencv
          ];
        in {
          default = pkgs.mkShell {
            packages = with pkgs; [
              python
              cmake
              ninja
              pkg-config
              gdb
              git
              usbutils
            ];
            # mkShell supplies the compiler; these also expose headers and
            # library search paths to the C++ SDK's CMake build.
            buildInputs = libraries;

            shellHook = ''
              # Keep SDK imports editable, including when entering from a subdirectory.
              export STREAMDOCK_ROOT="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
              export STREAMDOCK_PYTHON_SDK="$STREAMDOCK_ROOT/Python-SDK"
              export STREAMDOCK_CPP_SDK="$STREAMDOCK_ROOT/CPP-SDK"
              export STREAMDOCK_UDEV_RULES="$STREAMDOCK_PYTHON_SDK/99-streamdock.rules"
              export PYTHONPATH="$STREAMDOCK_PYTHON_SDK/src''${PYTHONPATH:+:$PYTHONPATH}"
              # The SDK loads its bundled transport .so via ctypes. NixOS does
              # not search /usr/lib, so expose its native dependencies here.
              export LD_LIBRARY_PATH="${pkgs.lib.makeLibraryPath libraries}''${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

              echo "StreamDock development environment ready."
              echo "Python: cd \"$STREAMDOCK_PYTHON_SDK\" && python src/main.py"
              echo "C++:    cmake -S \"$STREAMDOCK_CPP_SDK\" -B \"$STREAMDOCK_CPP_SDK/build\" -DCMAKE_BUILD_TYPE=Release"
              echo "        cmake --build \"$STREAMDOCK_CPP_SDK/build\""
              echo "USB access requires host udev rules; the shell does not install them."
            '';
          };
        });
    };
}
