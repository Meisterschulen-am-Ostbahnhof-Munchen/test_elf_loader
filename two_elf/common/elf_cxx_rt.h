/*
 * Minimal C++ runtime for an ELF with global objects (same as cpp_app): the loader provides neither
 * __dso_handle nor __cxa_atexit. Include in exactly one translation unit per ELF.
 * Destructors of globals are run by elf_cxx_shutdown() or, as in this demo, by a destructor attribute.
 */
#pragma once

extern "C" {
void *__dso_handle = &__dso_handle;

#define ELF_MAX_ATEXIT 16
static struct { void (*fn)(void *); void *arg; } s_atexit[ELF_MAX_ATEXIT];
static int s_atexit_cnt;

int __cxa_atexit(void (*fn)(void *), void *arg, void *)
{
    if (s_atexit_cnt >= ELF_MAX_ATEXIT) {
        return -1;
    }
    s_atexit[s_atexit_cnt].fn = fn;
    s_atexit[s_atexit_cnt].arg = arg;
    s_atexit_cnt++;
    return 0;
}
}

static inline void elf_cxx_shutdown(void)
{
    while (s_atexit_cnt > 0) {
        s_atexit_cnt--;
        s_atexit[s_atexit_cnt].fn(s_atexit[s_atexit_cnt].arg);
    }
}
