#include <stdio.h>

component print_string 
{
public:
    inport void in(const char*);
};
void print_string::in(const char*s)
{
    printf(s);
}


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

int main()
{
    driver<const char*> d;
    print_string s;
    connect d.out, s.in;
    d.run("hello world!\n");
    return 0;
} 
