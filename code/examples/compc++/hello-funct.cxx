
#line 1 "hello-funct.cc"
#include <stdio.h>

#include "compcxx_hello-funct.h"
class compcxx_print_string_3;
#line 14 "hello-funct.cc"
/*template <class T> */
#line 14 "hello-funct.cc"
class compcxx_driver_2
: public compcxx_component {
public:
    /*outport void out(const char* t)*/;
    void run(const char* t);
public:compcxx_print_string_3* p_compcxx_print_string_3;};


#line 3 "hello-funct.cc"
class compcxx_print_string_3 
: public compcxx_component {
public:
    /*inport */void in(const char*);
};

#line 21 "hello-funct.cc"

#line 21 "hello-funct.cc"
/*template <class T>
*/void compcxx_driver_2/*<const char* >*/::run(const char* t)
{
    (p_compcxx_print_string_3->in(t));
}


#line 8 "hello-funct.cc"
void compcxx_print_string_3::in(const char*s)
{
    printf(s);
}



#line 27 "hello-funct.cc"
int main()
{
    compcxx_driver_2/*<const char*> */d;
    compcxx_print_string_3 s;
    d.p_compcxx_print_string_3=&s /*connect d.out, s.in*/;
    d.run("hello world!\n");
    return 0;
} 
