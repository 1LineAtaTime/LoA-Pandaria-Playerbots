#include "PartyMemberSnaredTargetValue.h"

#include "Group.h"
#include "Playerbots.h"

// #163 S8: same movement-impaired check as PlayerbotAI::IsMovementImpaired on AC
// (Bot/PlayerbotAI.cpp:2109-2112); not added to PlayerbotAI.h/.cpp on this fork (duplicated in
// PaladinActions.cpp too - both are small enough that sharing isn't worth a new header).
static bool IsSnared(Unit* unit)
{
    // 5.4.8: Unit::IsRooted() doesn't exist on this fork; HasUnitState(UNIT_STATE_ROOT) is the
    // equivalent (root effects apply this state regardless of source).
    return unit &&
        (unit->HasAuraType(SPELL_AURA_MOD_ROOT) || unit->HasUnitState(UNIT_STATE_ROOT) || unit->GetSpeedRate(MOVE_RUN) < 1.0f);
}

bool PartyMemberSnaredTargetValue::Check(Unit* player)
{
    return PartyMemberValue::Check(player) && IsSnared(player);
}

Unit* PartyMemberSnaredTargetValue::Calculate()
{
    Group* group = bot->GetGroup();
    if (!group)
        return IsSnared(bot) ? bot : nullptr;

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->GetSource();
        if (player && player->IsAlive() && Check(player))
            return player;
    }

    return nullptr;
}
