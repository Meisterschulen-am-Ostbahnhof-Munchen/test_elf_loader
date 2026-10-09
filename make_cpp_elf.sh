#!/bin/bash
# Baut die C++-ELF-App (cpp_app) und den Host (elf_host), der sie per LittleFS laedt.
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

source "$HOME/.espressif/tools/activate_idf_v6.1.sh"

# esp32s31 ist noch Preview -> --preview
(cd cpp_app
 [ -f build/CMakeCache.txt ] || idf.py --preview -G 'Unix Makefiles' set-target esp32s31
 idf.py --preview elf)
mkdir -p elf_host/main/fs_image/riscv
cp cpp_app/build/hello_world.app.elf elf_host/main/fs_image/riscv/cpp_app.elf

(cd elf_host
 [ -f build/CMakeCache.txt ] || idf.py --preview set-target esp32s31
 idf.py --preview build)

echo "----------------------------------------------------------------------------"
echo " DONE. to flash type:"
echo "cd elf_host && idf.py --preview flash storage-flash monitor"
echo "Dann in der Shell: exec /storage/riscv/cpp_app.elf"
echo "----------------------------------------------------------------------------"
