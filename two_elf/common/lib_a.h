/*
 * Interface of "lib A": a small C++ class that lives in ELF A only.
 * ELF B includes this header but must NOT link lib_a.cpp: every out-of-line
 * member below is an undefined symbol in B, resolved by the loader at relocation time
 * from the symbol table that ELF A exported to the host.
 */
#pragma once

class Base {
public:
    explicit Base(int id);          // _ZN4BaseC1Ei / C2Ei
    virtual ~Base();                // _ZN4BaseD0Ev / D1Ev / D2Ev
    virtual int value() const;      // via vtable (vtable lives in A)
    int id() const { return m_id; }
    static int live();              // number of live Base objects (state lives in A)
protected:
    int m_id;
};

Base &a_global();                   // the global object constructed by A's .init_array
