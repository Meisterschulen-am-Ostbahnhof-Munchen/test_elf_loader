/*
 * Host side of the two-ELF demo (two_elf/): ELF B uses symbols defined in ELF A.
 *
 * Shell command:  twoelf <scenario> <a.elf> <b.elf>
 *   ok       load A, run A (global ctor, export table), load B (resolves against A), run B,
 *            try to unload A while B is loaded (refused), unload B, then A
 *   a-first  load A, run A, unload A, then try to load B: relocation fails, B never runs
 *
 * The host does the bookkeeping the loader does not: it registers A's export table after A ran,
 * unregisters it before A is unloaded, and refuses to unload A while B (which holds pointers into A's
 * code and vtables) is still loaded.
 */
#include <stdio.h>
#include <string.h>
#include "esp_console.h"
#include "esp_elf.h"
#include "private/elf_symbol.h"

struct lib {
    const char *tag;
    elf_file_t file;
    esp_elf_t elf;
    bool loaded;
    esp_elf_symbol_table_t *table;      // export table registered for this lib (NULL if none)
};

static lib s_a = { "A", {}, {}, false, nullptr };
static lib s_b = { "B", {}, {}, false, nullptr };
static lib *s_current;                  // lib whose main() is running; receives host_export_symbols()

extern "C" void host_export_symbols(const void *table)
{
    esp_elf_symbol_table_t *t = (esp_elf_symbol_table_t *)table;
    int n = 0;
    for (const struct esp_elfsym *s = t; s->name; s++) {
        n++;
    }
    int ret = esp_elf_register_symbol(t);
    printf("host: %s exported %d symbols, register -> %d\n", s_current->tag, n, ret);
    if (ret == 0) {
        s_current->table = t;
    }
}

static int lib_load(lib *l, const char *path)
{
    int ret = esp_elf_open(&l->file, path);
    if (ret < 0) {
        printf("host: %s: open %s failed %d\n", l->tag, path, ret);
        return ret;
    }
    ret = esp_elf_init(&l->elf);
    if (ret < 0) {
        esp_elf_close(&l->file);
        return ret;
    }
    ret = esp_elf_relocate(&l->elf, l->file.payload);
    if (ret < 0) {
        printf("host: %s: relocate failed %d (unresolved symbol?)\n", l->tag, ret);
        esp_elf_deinit(&l->elf);
        esp_elf_close(&l->file);
        return ret;
    }
    l->loaded = true;
    return 0;
}

static int lib_run(lib *l)
{
    s_current = l;
    int ret = esp_elf_request(&l->elf, 0, 0, NULL);
    printf("host: %s: request -> %d\n", l->tag, ret);
    return ret;
}

static void lib_unload(lib *l)
{
    if (l->table) {                     // drop the export table while the lib's memory is still valid
        esp_elf_unregister_symbol(l->table);
        l->table = nullptr;
    }
    esp_elf_deinit(&l->elf);            // runs the lib's fini_array
    esp_elf_close(&l->file);
    l->loaded = false;
    printf("host: %s unloaded\n", l->tag);
}

static int twoelf_main(int argc, char **argv)
{
    if (argc != 4) {
        printf("usage: twoelf <ok|a-first> <a.elf> <b.elf>\n");
        return -1;
    }
    const char *scenario = argv[1];

    if (lib_load(&s_a, argv[2]) < 0 || lib_run(&s_a) < 0) {
        return -1;
    }

    if (!strcmp(scenario, "a-first")) {
        printf("--- unload A first, then try to load B\n");
        lib_unload(&s_a);
        int ret = lib_load(&s_b, argv[3]);
        printf("host: load B after A is gone -> %d (expected: failure, B did not run)\n", ret);
        if (ret == 0) {
            lib_unload(&s_b);
        }
        return 0;
    }

    if (lib_load(&s_b, argv[3]) < 0) {
        lib_unload(&s_a);
        return -1;
    }
    int bret = lib_run(&s_b);
    printf("host: B %s\n", bret == 0 ? "PASSED" : "FAILED");

    printf("--- try to unload A while B is loaded\n");
    if (s_b.loaded) {
        printf("host: refused, B depends on A. Unloading B first.\n");
        lib_unload(&s_b);
    }
    lib_unload(&s_a);
    return 0;
}

static const struct esp_elfsym s_host_syms[] = {
    { "host_export_symbols", (void *)host_export_symbols },
    ESP_ELFSYM_END
};

extern "C" void two_elf_register(void)
{
    esp_elf_register_symbol(s_host_syms);

    esp_console_cmd_t cmd = {};
    cmd.command = "twoelf";
    cmd.help = "Two-ELF demo: twoelf <ok|a-first> <a.elf> <b.elf>";
    cmd.func = &twoelf_main;
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}
