
#line 1 "hello-system.cc"
#include <stdio.h>

#include "compcxx_hello-system.h"

#line 3 "hello-system.cc"
class compcxx_print_string_4 
: public compcxx_component {
public:
    /*inport */const char* in(const char*);
};
class compcxx_Wrapper_3;
#line 14 "hello-system.cc"
/*template <class T> */
#line 14 "hello-system.cc"
class compcxx_driver_2
: public compcxx_component {
public:
    /*outport const char* out(const char* t)*/;
    /*inport */const char* run(const char* t);
public:compcxx_Wrapper_3* p_compcxx_parent;};

class compcxx_print_string_4;
#line 27 "hello-system.cc"
/*template <class T1> */
#line 27 "hello-system.cc"
class compcxx_Wrapper_3
: public compcxx_component {
public:
    compcxx_driver_2/*<const char* > */d;
    /*outport */const char* out(const char* t);
    /*inport */const char* run(const char* t);
    
    compcxx_Wrapper_3() { 
         /*connect , d.run*/; 
        d.p_compcxx_parent=this /*connect d.out, */;
    }    
public:compcxx_print_string_4* p_compcxx_print_string_4;};


#line 40 "hello-system.cc"
class compcxx_system_5
: public compcxx_component {
public:
    compcxx_Wrapper_3/*<const char*> */w;
    compcxx_print_string_4 s;
    compcxx_system_5() {
        w.p_compcxx_print_string_4=&s /*connect w.out, s.in*/;
    }
};


#line 8 "hello-system.cc"
const char* compcxx_print_string_4::in(const char*s)
{
    printf(s);
    return s;
}


#line 21 "hello-system.cc"

#line 21 "hello-system.cc"
/*template <class T>
*/const char* compcxx_driver_2/*<const char* >*/::run(const char* t)
{
    return (p_compcxx_parent->out(t));
}


#line 31 "hello-system.cc"
const char* compcxx_Wrapper_3::out(const char* t){return (p_compcxx_print_string_4->in(t));}
#line 32 "hello-system.cc"
const char* compcxx_Wrapper_3::run(const char* t){return (d.run(t));}
#line 50 "hello-system.cc"
int main()
{
    compcxx_system_5 s;
    s.w.run(s.w.run("hello world!\n"));
    return 0;
} 
