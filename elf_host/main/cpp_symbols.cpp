/*
 * C++ runtime symbols exported to ELF applications loaded by elf_loader.
 * The built-in loader table only knows C library symbols, so anything a C++ app
 * pulls in from libstdc++/libsupc++ must be provided by the host here.
 * Mangled names assume a 32-bit target (size_t == unsigned int -> 'j').
 */
#include <new>
#include <cstddef>
#include "esp_elf.h"
#include "private/elf_symbol.h"

extern "C" void __cxa_pure_virtual(void);

static void *cpp_new(size_t n) { return ::operator new(n); }
static void *cpp_new_array(size_t n) { return ::operator new[](n); }
static void cpp_delete(void *p) { ::operator delete(p); }
static void cpp_delete_sized(void *p, size_t) { ::operator delete(p); }
static void cpp_delete_array(void *p) { ::operator delete[](p); }
static void cpp_delete_array_sized(void *p, size_t) { ::operator delete[](p); }

static const struct esp_elfsym s_cpp_syms[] = {
    { "_Znwj",   (void *)cpp_new },
    { "_Znaj",   (void *)cpp_new_array },
    { "_ZdlPv",  (void *)cpp_delete },
    { "_ZdlPvj", (void *)cpp_delete_sized },
    { "_ZdaPv",  (void *)cpp_delete_array },
    { "_ZdaPvj", (void *)cpp_delete_array_sized },
    { "__cxa_pure_virtual", (void *)__cxa_pure_virtual },
    ESP_ELFSYM_END
};

extern "C" void cpp_symbols_register(void)
{
    esp_elf_register_symbol(s_cpp_syms);
}
