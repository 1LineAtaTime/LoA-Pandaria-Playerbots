#include "WarriorTriggers.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"

bool ShieldBarrierTrigger::IsActive()
{
    return BuffTrigger::IsActive() && AI_VALUE2(uint8, "rage", "self target") >= 40;
}

// #196 (AC WarriorTriggers.cpp:26-48): fire unless a group member already carries the bot's own Vigilance.
// The action (CastVigilanceAction::GetTarget) picks the highest-gear-score DPS; the old body here duplicated
// the tank-first pick with the same bot->GetHealth() bug.
bool VigilanceTrigger::IsActive()
{
    if (!bot->HasSpell(114030))
    {
        return false;
    }

    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        // The action casts Vigilance on dps, but the trigger fails if any party member has
        // Vigilance from the bot (so the player can have a bot cast Vigilance on any group member).
        if (botAI->HasAura("vigilance", member, false, true))
            return false;
    }

    return true;
}

// ---------------------------------------------------------------------------
// Issue #357 slice 10 - bodies for the trigger classes ported from AC
// Ai/Class/Warrior/WarriorTriggers.cpp:20-24 and :50-73. They must live here:
// AI_VALUE2 expands to `context->GetValue<...>()` (an AiObject member) and
// PlayerbotAI::HasAura()/HasSpellCooldown() need a complete Player type.
// ---------------------------------------------------------------------------
bool BloodrageBuffTrigger::IsActive()
{
    // AC WarriorTriggers.cpp:20-24. Bloodrage itself was removed in Cataclysm,
    // so this stays inert on 5.4.8 (no strategy references it); the class is
    // registered only for AC name parity.
    return AI_VALUE2(uint8, "health", "self target") >= sPlayerbotAIConfig->mediumHealth &&
           AI_VALUE2(uint8, "rage", "self target") < 20;
}

namespace
{
// AC WarriorTriggers.cpp:13-16 - unchanged on 5.4.8.
constexpr uint32 SPELL_SHATTERING_THROW = 64382;
constexpr uint32 SPELL_DIVINE_SHIELD = 642;
constexpr uint32 SPELL_ICE_BLOCK = 45438;
constexpr uint32 SPELL_BLESSING_OF_PROTECTION = 41450;
}  // namespace

bool ShatteringThrowTrigger::IsActive()
{
    if (!bot->HasSpell(SPELL_SHATTERING_THROW) || bot->HasSpellCooldown(SPELL_SHATTERING_THROW))
        return false;

    GuidVector enemies = AI_VALUE(GuidVector, "possible targets");

    for (ObjectGuid const& guid : enemies)
    {
        Unit* enemy = botAI->GetUnit(guid);
        if (!enemy || !enemy->IsAlive() || enemy->IsFriendlyTo(bot))
            continue;

        if (bot->IsWithinDistInMap(enemy, 25.0f) &&
            (enemy->HasAura(SPELL_DIVINE_SHIELD) ||
             enemy->HasAura(SPELL_ICE_BLOCK) ||
             enemy->HasAura(SPELL_BLESSING_OF_PROTECTION)))
        {
            return true;
        }
    }

    return false;
}

// #522 (AC Ai/Base/Trigger/GenericTriggers.cpp:483-488): AC's base "fear sleep sap" trigger is not registered in
// this fork's shared TriggerContext, so the warrior context registers its own copy (Warrior folder only).
bool WarriorFearSleepSapTrigger::IsActive()
{
    return bot->HasAuraType(SPELL_AURA_MOD_FEAR) ||
           bot->HasAuraWithMechanic(1 << MECHANIC_SLEEP) ||
           bot->HasAuraWithMechanic(1 << MECHANIC_SAPPED);
}

// Lane 11 (#866): see ThunderClapTrigger in WarriorTriggers.h.
bool ThunderClapTrigger::IsActive()
{
    constexpr uint32 SPELL_THUNDER_CLAP = 6343;
    constexpr uint32 SPELL_WEAKENED_BLOWS = 115798;
    if (!bot->HasSpell(SPELL_THUNDER_CLAP) || bot->HasSpellCooldown(SPELL_THUNDER_CLAP))
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive() || target->IsFriendlyTo(bot) || !bot->IsWithinDistInMap(target, 7.0f))
        return false;

    return !target->HasAura(SPELL_WEAKENED_BLOWS, bot->GetGUID());
}

// Lane 11 (#866): see WarriorSelfHealthTrigger in WarriorTriggers.h.
bool WarriorSelfHealthTrigger::IsActive()
{
    return bot->IsAlive() && bot->GetHealthPct() <= float(pct);
}
