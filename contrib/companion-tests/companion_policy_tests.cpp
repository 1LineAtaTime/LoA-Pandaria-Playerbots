#include "CompanionPolicy.h"
#include <iostream>
int main()
{
    using namespace CompanionPolicy;
    bool const results[] = {
        !Allowed(1,0,false,60,0,true), !Allowed(59,0,false,60,0,true),
        Allowed(60,0,false,60,0,true), Allowed(90,0,false,60,0,true),
        !Allowed(90,0,false,60,1,true), Allowed(60,1,false,60,1,true),
        Allowed(1,3,false,60,0,true), !Allowed(1,3,false,60,0,false),
        Allowed(1,0,true,60,0,false), !Allowed(60,0,false,-1,0,false),
        Capacity(0,0,39)==39, Capacity(2,2,4)==0, Capacity(2,1,4)==1,
        Capacity(40,0,39)==0, Capacity(0,0,200)==39, Capacity(0,0,-1)==0
    };
    for (unsigned i=0; i<sizeof(results)/sizeof(*results); ++i)
        if (!results[i]) { std::cerr << "Failed case " << i << '\n'; return 1; }
    std::cout << "16 companion access/capacity cases passed\n";
}
