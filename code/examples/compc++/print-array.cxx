
#line 1 "print-array.cc"
#include <stdio.h>
#include "compcxx_print-array.h"
class compcxx_SensorNode_4;
#line 2 "print-array.cc"
class compcxx_Net_2
: public compcxx_component {
public:
    /*outport void out(int)*/;
    /*inport */void print();
    int index;
public:compcxx_SensorNode_4* p_compcxx_parent;};


#line 15 "print-array.cc"
class  compcxx_SensorNode_4
: public compcxx_component {
public:
    compcxx_Net_2 a;
    class my_SensorNode_out_f_t:public compcxx_functor<SensorNode_out_f_t>{ public:void  operator() (int i) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(i); return (c[0]->*f[0])(i);};};my_SensorNode_out_f_t out_f;/*outport */void out(int i);
    /*inport */void print();
    compcxx_SensorNode_4 ()
    { 
        a.p_compcxx_parent=this /*connect a.out, */; 
         /*connect , a.print*/;
    }
};


#line 28 "print-array.cc"
class compcxx_Channel_3
: public compcxx_component {
public:
    /*inport */void in(int);
    class my_Channel_print_f_t:public compcxx_functor<Channel_print_f_t>{ public:void  operator() () { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(); return (c[0]->*f[0])();};};compcxx_array<my_Channel_print_f_t > print;/*outportvoid print()*/;
    void activate();
    void config(int i ) { n=i; print.SetSize(i); }
private:
    int n;
};


#line 10 "print-array.cc"
void compcxx_Net_2::print()
{
    (p_compcxx_parent->out(index));
}


#line 19 "print-array.cc"
void compcxx_SensorNode_4::out(int i){return (out_f(i));}
#line 20 "print-array.cc"
void compcxx_SensorNode_4::print(){return (a.print());}
#line 39 "print-array.cc"
void compcxx_Channel_3::activate()
{
    for(int i=0;i<n;i++)
        print[i]();
}

#line 44 "print-array.cc"
void compcxx_Channel_3::in(int i)
{
    printf("%d\n", i);
}


#line 49 "print-array.cc"
int main()
{
    compcxx_Channel_3 c;
    compcxx_array<compcxx_SensorNode_4 >b;

    int n=5;
    b.SetSize(n);
    c.config(n);
    for(int i=0;i<n;i++)
    {
        c.print[i].Connect(b[i],(compcxx_component::Channel_print_f_t)&compcxx_SensorNode_4::print) /*connect c.print[i], b[i].print*/;
        b[i].out_f.Connect(c,(compcxx_component::SensorNode_out_f_t)&compcxx_Channel_3::in) /*connect b[i].out, c.in*/;
        b[i].a.index=i;
    }
    
    c.activate();    
    
    return 0;
}
