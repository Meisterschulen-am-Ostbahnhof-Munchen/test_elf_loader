#!/bin/bash
# Baut die C++-ELF-App (cpp_app) und den Host (elf_host), der sie per LittleFS laedt.
# Aufruf: source ./make_cpp_elf.sh [target]   (das IDF-Activate-Skript verlangt source)
#   target: esp32s31 (Default, RISC-V) oder z.B. esp32s3 / esp32 (Xtensa)
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

TARGET="${1:-esp32s31}"
case "$TARGET" in
	esp32|esp32s2|esp32s3) ARCH=xtensa ;;
	*) ARCH=riscv ;;
esac
# esp32s31 behaelt die bisherigen Ordner (build, sdkconfig); andere Targets bekommen eigene
if [ "$TARGET" = "esp32s31" ]; then
	APP_ARGS=()
	HOST_ARGS=()
else
	# cpp_app hat nur sdkconfig.defaults, elf_host zusaetzlich sdkconfig.defaults.<target>
	APP_ARGS=(-B "build_$TARGET" -D "SDKCONFIG=sdkconfig_$TARGET")
	HOST_ARGS=(-B "build_$TARGET" -D "SDKCONFIG=sdkconfig_$TARGET" -D "SDKCONFIG_DEFAULTS=sdkconfig.defaults;sdkconfig.defaults.$TARGET")
fi
BUILD_DIR="build"; [ "$TARGET" = "esp32s31" ] || BUILD_DIR="build_$TARGET"

source "$HOME/.espressif/tools/activate_idf_v6.1.sh"

# esp32s31 ist noch Preview -> --preview
(cd cpp_app
 [ -f $BUILD_DIR/CMakeCache.txt ] || idf.py --preview "${APP_ARGS[@]}" -G 'Unix Makefiles' set-target "$TARGET"
 idf.py --preview "${APP_ARGS[@]}" elf)
mkdir -p elf_host/main/fs_image/$ARCH
cp cpp_app/$BUILD_DIR/hello_world.app.elf elf_host/main/fs_image/$ARCH/cpp_app.elf

(cd elf_host
 [ -f $BUILD_DIR/CMakeCache.txt ] || idf.py --preview "${HOST_ARGS[@]}" set-target "$TARGET"
 idf.py --preview "${HOST_ARGS[@]}" build)

echo "----------------------------------------------------------------------------"
echo " DONE ($TARGET). to flash type:"
echo "cd elf_host && idf.py --preview ${HOST_ARGS[*]} flash monitor"
echo "Dann in der Shell: exec $ARCH/cpp_app.elf"
echo "----------------------------------------------------------------------------"
