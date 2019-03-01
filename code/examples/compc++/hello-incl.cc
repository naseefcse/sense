#include <stdio.h>
#include "hello-incl.h"

template <class T> component driver
{
public:
    outport void out(T t);
    void run(T t);
};

template <class T>
void driver<T>::run(T t)
{
    out(t);
}

component system
{
public:
    driver<const char*> d;
    print_string s;
    system() {
        connect d.out, s.in;
    }
};

int main()
{
    system s;
    s.d.run("hello world!\n");
    return 0;
} 
