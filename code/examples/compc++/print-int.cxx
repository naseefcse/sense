
#line 1 "print-int.cc"
#include <stdio.h>
#include "compcxx_print-int.h"
class compcxx_B_3;
#line 2 "print-int.cc"
class compcxx_A_2
: public compcxx_component {
public:
    /*outport void out(int)*/;
    /*inport */void print(int);
public:compcxx_B_3* p_compcxx_parent;};

class compcxx_C_4;
#line 14 "print-int.cc"
class compcxx_B_3
: public compcxx_component {
public:
    compcxx_A_2 a;
    /*outport */void out(int i);
    /*inport */void print(int i);
    compcxx_B_3 ()
    { 
        a.p_compcxx_parent=this /*connect a.out, */; 
         /*connect , a.print*/;
    }
public:compcxx_C_4* p_compcxx_parent;};

class compcxx_D_5;
#line 27 "print-int.cc"
/*template <class T> */
#line 27 "print-int.cc"
class compcxx_C_4
: public compcxx_component {
public:
    compcxx_B_3 b;
    /*outport */void out(int i);
    /*inport */void print(int i);
    compcxx_C_4 () 
    { 
        b.p_compcxx_parent=this /*connect b.out, */; 
         /*connect , b.print*/;
    }
public:compcxx_D_5* p_compcxx_D_5;};


#line 40 "print-int.cc"
class compcxx_D_5
: public compcxx_component {
public:
    /*inport */void in(int);
};


#line 9 "print-int.cc"
void compcxx_A_2::print(int i)
{
    (p_compcxx_parent->out(i));
}


#line 18 "print-int.cc"
void compcxx_B_3::out(int i){return (p_compcxx_parent->out(i));}
#line 19 "print-int.cc"
void compcxx_B_3::print(int i){return (a.print(i));}
#line 31 "print-int.cc"
void compcxx_C_4::out(int i){return (p_compcxx_D_5->in(i));}
#line 32 "print-int.cc"
void compcxx_C_4::print(int i){return (b.print(i));}
#line 46 "print-int.cc"
void compcxx_D_5::in(int i)
{
    printf("%d\n", i);
}


#line 51 "print-int.cc"
int main()
{
    compcxx_C_4/*<int> */c;
    compcxx_D_5 d;
    c.p_compcxx_D_5=&d /*connect c.out, d.in*/;
    c.print(5);
    
    return 0;
}
