#include "GilneasRidePath.h"
#include "GilneasCageRecovery.h"
#include "GilneasCrowleyRoute.h"
#include "CatapultLaunchPolicy.h"
#include <limits>
struct Waypoint { float x,y,z; };
int main()
{
    Waypoint* one = new Waypoint{1,2,3};
    Waypoint* two = new Waypoint{4,5,6};
    std::map<unsigned,Waypoint*> original{{1,one},{2,two}};
    auto saved = GilneasRidePath::Copy(original);
    delete one; delete two; original.clear(); // model full DB path reload
    if (saved.size()!=2 || saved.at(1).x!=1 || saved.at(2).z!=6) return 1;
    Waypoint valid{1,2,3}, bad{std::numeric_limits<float>::quiet_NaN(),2,3};
    if (!GilneasRidePath::Copy(std::map<unsigned,Waypoint*>{{1,&valid},{999999,&valid}}).empty()) return 2;
    if (!GilneasRidePath::Copy(std::map<unsigned,Waypoint*>{{1,&valid},{2,&bad}}).empty()) return 3;
    if (!GilneasRidePath::Copy(std::map<unsigned,Waypoint*>{{1,nullptr}}).empty()) return 4;
    if (!GilneasRidePath::Copy(std::map<unsigned,Waypoint*>{{0,&valid}}).empty()) return 5;
    using CatapultLaunchPolicy::Valid;
    if (!Valid(true,1,2,3,10,5) || !Valid(true,1,2,3,10,-5)) return 6;
    if (Valid(false,1,2,3,10,5) || Valid(true,1,2,3,0,5) || Valid(true,1,2,3,-10,5)) return 7;
    float nan=std::numeric_limits<float>::quiet_NaN(), inf=std::numeric_limits<float>::infinity();
    if (Valid(true,nan,2,3,10,5) || Valid(true,1,2,3,inf,5) || Valid(true,1,2,3,10,nan)) return 8;
    GilneasCrowleyRoute::Progress progress;
    unsigned jumps=0;
    for (unsigned point=1;point<=33;++point)
    {
        if (progress.Point()!=point || progress.Finished()) return 9;
        bool jump=point==2 || point==19;
        if (progress.Jump()!=jump) return 10;
        if (progress.Arrive(point+1,jump) || progress.Arrive(point,!jump)) return 11;
        if (!progress.Arrive(point,jump) || progress.Arrive(point,jump)) return 12;
        jumps+=jump;
    }
    if (!progress.Finished() || jumps!=2 || progress.Arrive(34,false)) return 13;
    using GilneasCageRecovery::Needed;
    if (!Needed(22,654,true,false,true,false,false,-1818.41f,2294.27f,0.5f)) return 14;
    if (Needed(22,654,true,false,true,false,false,-1818.41f,2294.27f,42.21f)) return 15;
    if (Needed(22,654,true,true,true,false,false,-1818.41f,2294.27f,0.5f)) return 16;
    if (Needed(22,654,false,false,true,false,false,-1818.41f,2294.27f,0.5f)) return 17;
    if (Needed(1,654,true,false,true,false,false,-1818.41f,2294.27f,0.5f)) return 18;
    if (Needed(22,0,true,false,true,false,false,-1818.41f,2294.27f,0.5f)) return 19;
    if (Needed(22,654,true,false,false,false,false,-1818.41f,2294.27f,0.5f)) return 20;
    if (Needed(22,654,true,false,true,true,false,-1818.41f,2294.27f,0.5f)) return 21;
    if (Needed(22,654,true,false,true,false,true,-1818.41f,2294.27f,0.5f)) return 22;
    if (Needed(22,654,true,false,true,false,false,100,100,0.5f)) return 23;
    if (Needed(22,654,true,false,true,false,false,-1818.41f,2294.27f,nan)) return 24;
    return 0;
}
