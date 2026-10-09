/*
 * Contract between an ELF that exports symbols and the host (two_elf_host.cpp):
 * the ELF calls host_export_symbols(table) once from main(); the host registers the table with
 * esp_elf_register_symbol() so that ELFs loaded afterwards can resolve against it,
 * and unregisters it again before the exporting ELF is unloaded.
 * Layout is identical to struct esp_elfsym of the loader.
 */
#pragma once

struct elf_export_sym {
    const char *name;
    const void *addr;
};

extern "C" void host_export_symbols(const struct elf_export_sym *table);
