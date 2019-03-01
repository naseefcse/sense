#include <stdio.h>
component Net
{
public:
    outport void out(int);
    inport void print();
    int index;
};

void Net::print()
{
    out(index);
}

component  SensorNode
{
public:
    Net a;
    outport void out(int i);
    inport void print();
    SensorNode ()
    { 
        connect a.out, out; 
        connect print, a.print;
    }
};

component Channel
{
public:
    inport void in(int);
    outport[] void print();
    void activate();
    void config(int i ) { n=i; print.SetSize(i); }
private:
    int n;
};

void Channel::activate()
{
    for(int i=0;i<n;i++)
        print[i]();
}
void Channel::in(int i)
{
    printf("%d\n", i);
}

int main()
{
    Channel c;
    SensorNode[] b;

    int n=5;
    b.SetSize(n);
    c.config(n);
    for(int i=0;i<n;i++)
    {
        connect c.print[i], b[i].print;
        connect b[i].out, c.in;
        b[i].a.index=i;
    }
    
    c.activate();    
    
    return 0;
}
