component print_string 
{
public:
    inport void in(const char*);
};
void print_string::in(const char*s)
{
    printf(s);
}

