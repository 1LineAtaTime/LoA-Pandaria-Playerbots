#include "DruidCatActions.h"

#include "Playerbots.h"

// #363 (AC parity, Ai/Class/Druid/Action/DruidCatActions.h:117-124).
bool CastMaimAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target || !target->ToPlayer())
        return false;

    return CastMeleeSpellAction::isUseful();
}

bool CastTigersFuryAction::isUseful()
{
    return !bot->HasAura(106951) && CastBuffSpellAction::isUseful();  // 106951 = Berserk (cat), see the header
}

bool CastCowerAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return CastBuffSpellAction::isUseful() && !(target && target->IsPlayer());  // players have no threat
}
