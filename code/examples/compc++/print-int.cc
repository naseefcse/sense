#include <stdio.h>
component A
{
public:
    outport void out(int);
    inport void print(int);
};

void A::print(int i)
{
    out(i);
}

component B
{
public:
    A a;
    outport void out(int i);
    inport void print(int i);
    B ()
    { 
        connect a.out, out; 
        connect print, a.print;
    }
};

template <class T> component C
{
public:
    B b;
    outport void out(T i);
    inport void print(int i);
    C () 
    { 
        connect b.out, out; 
        connect print, b.print;
    }
};

component D
{
public:
    inport void in(int);
};

void D::in(int i)
{
    printf("%d\n", i);
}

int main()
{
    C<int> c;
    D d;
    connect c.out, d.in;
    c.print(5);
    
    return 0;
}
