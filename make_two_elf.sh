#!/bin/bash
# Baut die Zwei-ELF-Demo: two_elf/elf_a (lib_a) + two_elf/elf_b (lib_b), kopiert beide ins LittleFS-Image von elf_host
# und baut den Host. ELF B nutzt Symbole aus ELF A (siehe two_elf/README.md).
# Aufruf: source ./make_two_elf.sh [target]   (esp32s31 = Default/RISC-V, esp32s3 = Xtensa)
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

TARGET="${1:-esp32s31}"
case "$TARGET" in
	esp32|esp32s2|esp32s3) ARCH=xtensa ;;
	*) ARCH=riscv ;;
esac
if [ "$TARGET" = "esp32s31" ]; then
	APP_ARGS=(); HOST_ARGS=(); BUILD_DIR="build"
else
	APP_ARGS=(-B "build_$TARGET" -D "SDKCONFIG=sdkconfig_$TARGET")
	HOST_ARGS=(-B "build_$TARGET" -D "SDKCONFIG=sdkconfig_$TARGET" -D "SDKCONFIG_DEFAULTS=sdkconfig.defaults;sdkconfig.defaults.$TARGET")
	BUILD_DIR="build_$TARGET"
fi

source "$HOME/.espressif/tools/activate_idf_v6.1.sh"

mkdir -p elf_host/main/fs_image/$ARCH
for L in a b; do
	(cd two_elf/elf_$L
	 [ -f $BUILD_DIR/CMakeCache.txt ] || idf.py --preview "${APP_ARGS[@]}" -G 'Unix Makefiles' set-target "$TARGET"
	 idf.py --preview "${APP_ARGS[@]}" elf)
	cp two_elf/elf_$L/$BUILD_DIR/lib_$L.app.elf elf_host/main/fs_image/$ARCH/lib_$L.elf
done

(cd elf_host
 [ -f $BUILD_DIR/CMakeCache.txt ] || idf.py --preview "${HOST_ARGS[@]}" set-target "$TARGET"
 idf.py --preview "${HOST_ARGS[@]}" build)

echo "----------------------------------------------------------------------------"
echo " DONE ($TARGET). to flash type:"
echo "cd elf_host && idf.py --preview ${HOST_ARGS[*]} flash monitor"
echo "Dann in der Shell:  twoelf ok $ARCH/lib_a.elf $ARCH/lib_b.elf"
echo "                    twoelf a-first $ARCH/lib_a.elf $ARCH/lib_b.elf"
echo "----------------------------------------------------------------------------"
