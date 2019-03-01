#include <stdio.h>

component print_string 
{
public:
    inport const char* in(const char*);
};
const char* print_string::in(const char*s)
{
    printf(s);
    return s;
}

template <class T> component driver
{
public:
    outport T out(T t);
    inport T run(T t);
};

template <class T>
T driver<T>::run(T t)
{
    return out(t);
}

template <class T1> component Wrapper
{
public:
    driver<T1> d;
    outport T1 out(T1 t);
    inport T1 run(T1 t);
    
    Wrapper() { 
        connect run, d.run; 
        connect d.out, out;
    }    
};

component system
{
public:
    Wrapper<const char*> w;
    print_string s;
    system() {
        connect w.out, s.in;
    }
};

int main()
{
    system s;
    s.w.run(s.w.run("hello world!\n"));
    return 0;
} 
