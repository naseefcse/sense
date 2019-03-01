
#line 1 "hello-incl.cc"
#include <stdio.h>

#line 1 "hello-incl.h"

#line 2 "hello-incl.cc"


#include "compcxx_hello-incl.h"
class compcxx_print_string_3;/*template <class T> */
#line 4 "hello-incl.cc"
class compcxx_driver_2
: public compcxx_component {
public:
    /*outport void out(const char* t)*/;
    void run(const char* t);
public:compcxx_print_string_3* p_compcxx_print_string_3;};


#line 1 "hello-incl.h"
class compcxx_print_string_3 
: public compcxx_component {
public:
    /*inport */void in(const char*);
};

#line 17 "hello-incl.cc"
class compcxx_system_4
: public compcxx_component {
public:
    compcxx_driver_2/*<const char*> */d;
    compcxx_print_string_3 s;
    compcxx_system_4() {
        d.p_compcxx_print_string_3=&s /*connect d.out, s.in*/;
    }
};


#line 11 "hello-incl.cc"

#line 11 "hello-incl.cc"
/*template <class T>
*/void compcxx_driver_2/*<const char* >*/::run(const char* t)
{
    (p_compcxx_print_string_3->in(t));
}


#line 6 "hello-incl.h"
void compcxx_print_string_3::in(const char*s)
{
    printf(s);
}


#line 27 "hello-incl.cc"
int main()
{
    compcxx_system_4 s;
    s.d.run("hello world!\n");
    return 0;
} 
