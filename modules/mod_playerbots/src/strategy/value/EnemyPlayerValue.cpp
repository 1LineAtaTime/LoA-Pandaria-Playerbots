#include "EnemyPlayerValue.h"

#include "CombatManager.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "Vehicle.h"

namespace
{
    // #572 (MoP-only, beyond AC): with Battleground.IgnoreFaction = 1 (the 5.4.8 dist default,
    // BattlegroundQueue.cpp:190-240) a bot can play for the other faction's side, and the core gives it
    // that side's faction (Battleground.cpp:1605/1682). Inside a battleground "opposing" therefore means
    // the other BG team - the rule BGTactics::BgTeamIdOf already applies to the tactics layer. Outside a
    // battleground this is AC's race rule (PlayerbotAI::IsOpposing) unchanged.
    bool IsOpposingSide(PlayerbotAI* botAI, Player* bot, Player* other)
    {
        if (bot->InBattleground() && other->InBattleground() &&
            bot->GetBattlegroundId() == other->GetBattlegroundId())
            return bot->GetBGTeam() != other->GetBGTeam();

        return botAI->IsOpposing(other);
    }

    // #572: AC tests the flag aura against the race team (EnemyPlayerValue.cpp:66-67): a Horde bot hunts
    // the holder of 23333 (the Horde flag), an Alliance bot the holder of 23335. Same test, side taken
    // from the BG team inside a battleground.
    bool CarriesOurFlag(Player* bot, Unit* target)
    {
        bool const hordeSide = bot->InBattleground() ? bot->GetBGTeam() == HORDE : bot->GetTeamId() == TEAM_HORDE;
        return hordeSide ? target->HasAura(23333) : target->HasAura(23335);
    }
}

bool NearestEnemyPlayersValue::AcceptUnit(Unit* unit)
{
    // #571, AC EnemyPlayerValue.cpp:15-17: apply the parent's filtering first (hostility, visibility,
    // PvP-prohibited zones, unflagged players - AttackersValue::IsPossibleTarget).
    if (!PossibleTargetsValue::AcceptUnit(unit))
        return false;

    bool inCannon = botAI->IsInVehicle(false, true);
    Player* enemy = dynamic_cast<Player*>(unit);
    if (enemy && IsOpposingSide(botAI, bot, enemy) && enemy->IsPvP() &&
        !sPlayerbotAIConfig->IsPvpProhibited(enemy->GetZoneId(), enemy->GetAreaId()) &&
        !enemy->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NON_ATTACKABLE_2) &&
        ((inCannon || !enemy->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))) &&
        /*!enemy->HasStealthAura() && !enemy->HasInvisibilityAura()*/ (bot->InBattleground() ? bot->CanSeeOrDetect(enemy) : enemy->CanSeeOrDetect(bot)) &&
        !(enemy->HasSpiritOfRedemptionAura()))
    {
        // #571, AC EnemyPlayerValue.cpp:27-31: with a master, only attack if the master is PvP flagged.
        Player* master = botAI->GetMaster();
        if (master && !master->IsPvP() && !master->IsFFAPvP())
            return false;

        return true;
    }

    return false;
}

Unit* EnemyPlayerValue::Calculate()
{
    bool controllingCannon = false;
    bool controllingVehicle = false;
    bool passenger = false;
    if (Vehicle* vehicle = bot->GetVehicle())
    {
        // #693, AC mod-playerbots#2855 (EnemyPlayerValue.cpp:43-55): a passenger in an attack seat fights too.
        VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(bot);
        if (!seat || (!seat->CanControl() && !(seat->m_flags & VEHICLE_SEAT_FLAG_CAN_ATTACK)))
            return nullptr;  // a passenger seat that can't attack
        VehicleEntry const* vi = vehicle->GetVehicleInfo();
        if (!seat->CanControl())
            passenger = true;  // fights from its seat: it can't close in
        else if (vi && vi->m_flags & VEHICLE_FLAG_FIXED_POSITION)
            controllingCannon = true;
        else
            controllingVehicle = true;
    }

    // 1. Check units we are currently in PvP combat with.
    // #571, AC EnemyPlayerValue.cpp:55-70: AC walks the PvP combat references. This fork walked
    // GetThreatenedByMeList(), which only holds the threat lists of creatures (players have none), so
    // after the IsPlayer() filter this step could never return a player that was already fighting the bot.
    std::vector<Unit*> targets;
    Unit* pVictim = bot->GetVictim();
    for (auto const& [guid, combatRef] : bot->GetCombatManager().GetPvPCombatRefs())
    {
        Unit* pTarget = combatRef->GetOther(bot);
        if (!pTarget || pTarget == pVictim || !pTarget->IsPlayer() ||
            !(bot->InBattleground() ? bot->CanSeeOrDetect(pTarget) : pTarget->CanSeeOrDetect(bot)) ||
            !bot->IsWithinDist(pTarget, VISIBILITY_DISTANCE_NORMAL))
            continue;

        if (CarriesOurFlag(bot, pTarget))  // #572: BG side, not race
            return pTarget;

        targets.push_back(pTarget);
    }

    if (!targets.empty())
    {
        std::sort(targets.begin(), targets.end(),
            [&](Unit const* pUnit1, Unit const* pUnit2)
            { return bot->GetDistance(pUnit1) < bot->GetDistance(pUnit2); });

        return *targets.begin();
    }

    // 2. Find enemy player in range.

    GuidVector players = AI_VALUE(GuidVector, "nearest enemy players");
    float const maxAggroDistance = GetMaxAttackDistance();
    for (const auto& gTarget : players)
    {
        Unit* pUnit = botAI->GetUnit(gTarget);
        if (!pUnit)
            continue;

        Player* pTarget = dynamic_cast<Player*>(pUnit);
        if (!pTarget)
            continue;

        if (pTarget == pVictim)
            continue;

        if (CarriesOurFlag(bot, pTarget))  // #572: BG side, not race (AC EnemyPlayerValue.cpp:98-107)
            return pTarget;

        // Aggro weak enemies from further away.
        // If controlling mobile vehicle only agro close enemies (otherwise will never reach objective)
        uint32 const aggroDistance = passenger ? 30.0f
            : controllingVehicle ? 5.0f
            : (controllingCannon || bot->GetHealth() > pTarget->GetHealth()) ? maxAggroDistance
            : 20.0f;
        if (!bot->IsWithinDist(pTarget, aggroDistance))
            continue;

        if (bot->IsWithinLOSInMap(pTarget) &&
            (controllingCannon || (fabs(bot->GetPositionZ() - pTarget->GetPositionZ()) < 30.0f)))
            return pTarget;
    }

    // 3. Check party attackers.

    if (Group* pGroup = bot->GetGroup())
    {
        for (GroupReference* itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            if (Unit* pMember = itr->GetSource())
            {
                if (pMember == bot)
                    continue;

                if (sServerFacade->GetDistance2d(bot, pMember) > 30.0f)
                    continue;

                if (Unit* pAttacker = pMember->getAttackerForHelper())
                    if (pAttacker->IsPlayer() && bot->IsWithinDist(pAttacker, maxAggroDistance * 2.0f) &&
                        bot->IsWithinLOSInMap(pAttacker) && pAttacker != pVictim &&
                        (bot->InBattleground() ? bot->CanSeeOrDetect(pAttacker) : pAttacker->CanSeeOrDetect(bot)))
                        return pAttacker;
            }
        }
    }

    return nullptr;
}

float EnemyPlayerValue::GetMaxAttackDistance()
{
    if (!bot->GetBattleground())
        return 60.0f;

    /*Battleground* bg = bot->GetBattleground();
    if (!bg)
        return 40.0f;

    BattlegroundTypeId bgType = bg->GetBgTypeID();
    if (bgType == BATTLEGROUND_RB)
        bgType = bg->GetBgTypeID(true);

    if (bgType == BATTLEGROUND_IC)
    {
        if (botAI->IsInVehicle(false, true))
            return 120.0f;
    }*/

    return 40.0f;
}
