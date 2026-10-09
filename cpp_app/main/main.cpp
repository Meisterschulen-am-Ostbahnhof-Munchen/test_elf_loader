/*
 * Test: do C++ classes work inside an ELF loaded by elf_loader?
 * Covers: non-virtual methods, virtual dispatch (vtable), heap objects (new/delete),
 * stack objects, and a global object with a constructor/destructor (.init_array).
 */
#include <stdio.h>
#include <unistd.h>

#ifndef TEST_GLOBALS
#define TEST_GLOBALS 0   // global ctor/dtor needs __dso_handle, __cxa_atexit and .init_array
#endif

class Shape {
public:
    explicit Shape(const char *name) : m_name(name) { printf("Shape ctor: %s\n", m_name); }
    virtual ~Shape() { printf("Shape dtor: %s\n", m_name); }
    virtual int area() const = 0;
    const char *name() const { return m_name; }
private:
    const char *m_name;
};

class Rect : public Shape {
public:
    Rect(int w, int h) : Shape("Rect"), m_w(w), m_h(h) {}
    int area() const override { return m_w * m_h; }
private:
    int m_w, m_h;
};

class Square : public Rect {
public:
    explicit Square(int s) : Rect(s, s) {}
};

class Counter {
public:
    Counter() : m_value(42) { printf("Global Counter ctor (init_array ran)\n"); }
    ~Counter() { printf("Global Counter dtor\n"); }
    int next() { return m_value++; }
private:
    int m_value;
};

#if TEST_GLOBALS
static Counter s_counter;   // needs .init_array to be executed by the loader
#endif

extern "C" int main(int argc, char *argv[])
{
#if TEST_GLOBALS
    printf("s_counter.next() = %d (expected 42 if constructor ran)\n", s_counter.next());
#endif

    Rect r(3, 4);                       // stack object
    printf("%s area = %d (expected 12)\n", r.name(), r.area());

    Shape *s = new Square(5);           // heap object + virtual dispatch
    printf("%s area = %d (expected 25)\n", s->name(), s->area());
    delete s;

    printf("C++ test done\n");
    return 0;
}
