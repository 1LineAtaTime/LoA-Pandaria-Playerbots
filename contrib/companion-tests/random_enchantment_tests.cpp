#include "RandomEnchantmentSelection.h"
#include <vector>
#include <limits>
#include <iostream>
#include <array>
struct Entry { unsigned ench; float chance; };
int main()
{
 using RandomEnchantmentSelection::Choose;
 std::vector<Entry> pool{{10,5.f},{20,15.f},{30,30.f}};
 std::array<unsigned,3> count{};
 for(unsigned i=0;i<10000;++i){auto value=Choose(pool,(i+0.5)/10000.0);if(value==10)++count[0];else if(value==20)++count[1];else if(value==30)++count[2];else return 1;}
 if(count!=std::array<unsigned,3>{1000,3000,6000})return 2;
 if(Choose(pool,0)!=10||Choose(pool,1)!=30)return 3;
 pool={{1,0.1f},{2,0.1f}};if(Choose(pool,0.25)!=1||Choose(pool,0.75)!=2||Choose(pool,1)!=2)return 4;
 pool={{1,100.f},{2,100.f}};if(Choose(pool,0.25)!=1||Choose(pool,0.75)!=2)return 5;
 auto nan=std::numeric_limits<double>::quiet_NaN();
 if(Choose(pool,nan)||Choose(pool,-0.1)||Choose(pool,1.1))return 6;
 pool={{0,10.f},{1,0.f},{2,-1.f},{3,std::numeric_limits<float>::quiet_NaN()}};if(Choose(pool,0.5))return 7;
 pool={{1,0.f},{2,1.f},{3,0.f}};if(Choose(pool,0)!=2||Choose(pool,1)!=2)return 8;
 std::cout<<"PASS: proportional distribution, fractional/overfull pools, endpoints, invalid weights and no empty valid draw\n";
}
