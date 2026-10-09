# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository state

Despite the name `test_elf_loader`, this currently contains only an unmodified copy of the ESP-IDF `hello_world` example (`hello_world/`); no ELF-loader code exists yet. `eclipse-workspace/` is untracked Eclipse IDE metadata (not source).

The project lives in `hello_world/` — run all commands from that directory. It is a standard ESP-IDF project: top-level `CMakeLists.txt` (sets `MINIMAL_BUILD ON`, so only components `main` depends on are built) and one component `main/` (`hello_world_main.c`, requires `spi_flash`).

## Environment

- Target: **esp32s31** (`CONFIG_IDF_TARGET` in `sdkconfig`), ESP-IDF v6.1 at `~/.espressif/v6.1/esp-idf`. Activate the IDF environment (`. $IDF_PATH/export.sh`) before building.
- `build/` and `sdkconfig` are present in the working tree; `build/` is generated.

## Commands

```
idf.py build
idf.py -p PORT flash monitor
idf.py menuconfig
idf.py set-target <chip>      # re-run reconfigures and wipes build/ and sdkconfig target settings
```

Tests are pytest-embedded (`pytest_hello_world.py`): `pytest --target esp32s31` (needs a device or QEMU, plus `pytest-embedded-idf`). Single test: `pytest -k test_hello_world --target <target>`. The file also has `linux` host-target tests (`idf.py --preview set-target linux`).
