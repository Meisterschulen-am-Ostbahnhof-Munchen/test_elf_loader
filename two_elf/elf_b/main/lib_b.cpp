/*
 * ELF B: uses and derives from Base without containing it. All Base symbols stay undefined
 * and are resolved by the loader against the table that ELF A exported.
 */
#include <stdio.h>
#include "lib_a.h"

class Derived : public Base {
public:
    explicit Derived(int id) : Base(id) {}
    int value() const override { return Base::value() + 100; }
};

extern "C" int main(int, char **)
{
    int fail = 0;
    printf("B: a_global().id()=%d (expect 42), live=%d (expect 1)\n", a_global().id(), Base::live());
    fail |= a_global().id() != 42 || Base::live() != 1;

    Base local(1);
    Base *d = new Derived(7);
    printf("B: virtual value(): local=%d derived=%d global=%d (expect 1 107 42), live=%d (expect 3)\n",
           local.value(), d->value(), a_global().value(), Base::live());
    fail |= local.value() != 1 || d->value() != 107 || a_global().value() != 42 || Base::live() != 3;

    delete d;                       // virtual dtor: Derived::D0 in B -> Base::D2 in A
    printf("B: after delete live=%d (expect 2)\n", Base::live());
    fail |= Base::live() != 2;

    printf(fail ? "B: FAILED\n" : "B: PASSED\n");
    return fail;
}
