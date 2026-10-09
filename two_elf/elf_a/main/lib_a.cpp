/*
 * ELF A: implements Base, has a global Base object, exports its symbols to the host.
 *
 * How the symbols get out: with -fvisibility=hidden and --strip-all the ELF has no usable symbol table,
 * so A carries its own: a list of (name, address). The names are plain strings; the addresses are emitted by
 * an asm block (.4byte <mangled symbol>) because C++ cannot take the address of a constructor/destructor.
 * The asm words get RELATIVE relocations like a vtable, so they are valid after esp_elf_relocate().
 */
#include <stdio.h>
#include "lib_a.h"
#include "elf_export.h"
#include "elf_cxx_rt.h"

static int s_live;

Base::Base(int id) : m_id(id) { s_live++; printf("A: Base(%d) ctor, live=%d\n", m_id, s_live); }
Base::~Base() { s_live--; printf("A: Base(%d) dtor, live=%d\n", m_id, s_live); }
int Base::value() const { return m_id; }
int Base::live() { return s_live; }

static Base g_a(42);                // constructed by the loader via .init_array
Base &a_global() { return g_a; }

/* Itanium ABI names, 32-bit target. C1/D1 are aliases of C2/D2; B may reference any of them. */
#define A_EXPORTS(X) \
    X(_ZN4BaseC1Ei) X(_ZN4BaseC2Ei) \
    X(_ZN4BaseD0Ev) X(_ZN4BaseD1Ev) X(_ZN4BaseD2Ev) \
    X(_ZNK4Base5valueEv) X(_ZN4Base4liveEv) X(_Z8a_globalv)

#define AS_ASM_WORD(n)  ".4byte " #n "\n"
#define AS_NAME(n)      { #n, nullptr },

__asm__(".pushsection .data.a_export_addrs,\"aw\"\n"
        ".balign 4\n"
        ".globl a_export_addrs\n"
        ".hidden a_export_addrs\n"
        "a_export_addrs:\n"
        A_EXPORTS(AS_ASM_WORD)
        ".popsection\n");
extern "C" const void *const a_export_addrs[] __attribute__((visibility("hidden")));

static struct elf_export_sym s_table[] = {
    A_EXPORTS(AS_NAME)
    { nullptr, nullptr }
};

__attribute__((destructor)) static void a_fini(void)
{
    printf("A: fini_array, global Base dtor runs via atexit list\n");
    elf_cxx_shutdown();
}

extern "C" int main(int, char **)
{
    for (unsigned i = 0; s_table[i].name; i++) {
        s_table[i].addr = a_export_addrs[i];
    }
    host_export_symbols(s_table);
    printf("A: exported symbols, a_global().id()=%d live=%d\n", a_global().id(), Base::live());
    return 0;
}
