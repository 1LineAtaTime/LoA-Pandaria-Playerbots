// #896: a Random Battleground (BATTLEGROUND_RB) and a Rated Battleground (BATTLEGROUND_RATED_10_VS_10) both keep
// their queue type as GetTypeID() and store the real map type in GetTypeID(true) (BattlegroundMgr.cpp:958-959).
#ifndef _PLAYERBOT_BG_REAL_TYPE_H
#define _PLAYERBOT_BG_REAL_TYPE_H

#include "Battleground.h"

inline BattlegroundTypeId RealBgTypeId(Battleground const* bg)
{
    BattlegroundTypeId t = bg->GetTypeID();
    if (t == BATTLEGROUND_RB || t == BATTLEGROUND_RATED_10_VS_10 || t == BATTLEGROUND_RATED_15_VS_15 ||
        t == BATTLEGROUND_RATED_25_VS_25)
        t = bg->GetTypeID(true);
    return t;
}

#endif
