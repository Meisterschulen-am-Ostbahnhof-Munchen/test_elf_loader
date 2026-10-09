# Two ELFs: B uses C++ symbols from A

Question: ELF B needs constructors/virtual methods that only ELF A defines. The loader resolves open symbols only
against the host tables (`esp_elf_register_symbol`), never against another loaded ELF.

## Design (no loader change needed)
- A (`elf_a/main/lib_a.cpp`) keeps its own export table: `{mangled name, address}`. Names are strings, addresses come from an asm
  block (`.4byte _ZN4BaseC1Ei`, C++ cannot take a ctor address). With `-fvisibility=hidden` and `--strip-all` there is no
  `.symtab`/`.dynsym` to read, so an explicit table is the only way; it costs ~8 bytes per symbol.
- A calls `host_export_symbols(table)` from `main()`. That happens after the loader ran A's `.init_array`.
- The host (`elf_host/main/two_elf_host.cpp`) registers the table with `esp_elf_register_symbol()`. Because the table lives in A's
  `.data` its pointers are valid only after A's `esp_elf_relocate()` (RELATIVE relocs), which is the case in `main()`.
- B (`elf_b/main/lib_b.cpp`) only includes `common/lib_a.h`. Its undefined `_ZN4BaseC2Ei` etc. are resolved at B's `esp_elf_relocate()`.
- Unload: the host unregisters A's table, then `esp_elf_deinit(A)` (runs A's `.fini_array`). The host must unload B before A:
  B holds raw pointers into A's code and vtables, the loader tracks no dependencies. Loading B after A is gone fails in relocate (-ENOSYS), B never runs.

Run: `source ./make_two_elf.sh [esp32s31|esp32s3]`, then in the shell `twoelf ok <arch>/lib_a.elf <arch>/lib_b.elf`
and `twoelf a-first ...`. Status: written, NOT built/tested yet.
