/*
* This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "GenericTriggers.h"

#include <string>

#include "DatabaseEnvFwd.h"
#include "BattlegroundWS.h"
#include "CreatureAI.h"
#include "GameTime.h"
#include "LastSpellCastValue.h"
#include "LootObjectStack.h"
#include "ObjectGuid.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "ServerFacade.h"
#include "SharedDefines.h"
#include "TemporarySummon.h"
#include "ThreatManager.h"
#include "Timer.h"

bool LowManaTrigger::IsActive()
{
    return AI_VALUE2(bool, "has mana", "self target") &&
        AI_VALUE2(uint8, "mana", "self target") < sPlayerbotAIConfig->lowMana;
}

bool MediumManaTrigger::IsActive()
{
    return AI_VALUE2(bool, "has mana", "self target") &&
        AI_VALUE2(uint8, "mana", "self target") < sPlayerbotAIConfig->mediumMana;
}

bool NoPetTrigger::IsActive()
{
    return (bot->GetMinionGUID().IsEmpty()) && (!AI_VALUE(Unit*, "pet target")) && (!bot->GetGuardianPet()) &&
        (!bot->GetFirstControlled()) && (!AI_VALUE2(bool, "mounted", "self target"));
}

bool HasPetTrigger::IsActive()
{
    return (AI_VALUE(Unit*, "pet target")) && !AI_VALUE2(bool, "mounted", "self target");
}

bool PetAttackTrigger::IsActive()
{
    Guardian* pet = bot->GetGuardianPet();
    if (!pet)
    {
        return false;
    }
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
    {
        return false;
    }
    if (pet->GetVictim() == target && pet->GetCharmInfo()->IsCommandAttack())
    {
        return false;
    }
    if (bot->GetMap()->IsDungeon() && bot->GetGroup() && !target->IsInCombat())
    {
        return false;
    }
    return true;
}

bool HighManaTrigger::IsActive()
{
    return AI_VALUE2(bool, "has mana", "self target") && AI_VALUE2(uint8, "mana", "self target") < sPlayerbotAIConfig->highMana;
}

bool AlmostFullManaTrigger::IsActive()
{
    return AI_VALUE2(bool, "has mana", "self target") && AI_VALUE2(uint8, "mana", "self target") > 85;
}

bool EnoughManaTrigger::IsActive()
{
    return AI_VALUE2(bool, "has mana", "self target") && AI_VALUE2(uint8, "mana", "self target") > sPlayerbotAIConfig->highMana;
}

// Ported from AC mod-playerbots GenericTriggers.cpp:94-106 (#149).
bool ComboPointsAvailableTrigger::IsActive() { return AI_VALUE2(uint8, "combo", "current target") >= amount; }

bool ComboPointsNotFullTrigger::IsActive() { return AI_VALUE2(uint8, "combo", "current target") < amount; }

bool TargetWithComboPointsLowerHealTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive() || !target->IsInWorld())
        return false;

    return ComboPointsAvailableTrigger::IsActive() &&
           (target->GetHealth() / AI_VALUE(float, "estimated group dps")) <= lifeTime;
}

bool RageAvailable::IsActive() { return AI_VALUE2(uint8, "rage", "self target") >= amount; }

bool EnergyAvailable::IsActive() { return AI_VALUE2(uint8, "energy", "self target") >= amount; }

bool LoseAggroTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && target->IsPlayer())
        return false;  // players have no threat: taunts only waste the cooldown
    return !AI_VALUE2(bool, "has aggro", "current target");
}

bool HasAggroTrigger::IsActive() { return AI_VALUE2(bool, "has aggro", "current target"); }

bool PanicTrigger::IsActive()
{
    return AI_VALUE2(uint8, "health", "self target") < sPlayerbotAIConfig->criticalHealth &&
        (!AI_VALUE2(bool, "has mana", "self target") ||
            AI_VALUE2(uint8, "mana", "self target") < sPlayerbotAIConfig->lowMana);
}

bool OutNumberedTrigger::IsActive()
{
    if (bot->GetMap() && (bot->GetMap()->IsDungeon() || bot->GetMap()->IsRaid()))
        return false;

    if (bot->GetGroup() && bot->GetGroup()->isRaidGroup())
        return false;

    int32 botLevel = bot->GetLevel();
    uint32 friendPower = 200;
    uint32 foePower = 0;
    for (auto& attacker : botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get())
    {
        Creature* creature = botAI->GetCreature(attacker);
        if (!creature)
            continue;

        int32 dLevel = creature->GetLevel() - botLevel;
        if (dLevel > -10)
            foePower = std::max(100 + 10 * dLevel, dLevel * 200);
    }

    if (!foePower)
        return false;

    for (auto& helper : botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest friendly players")->Get())
    {
        Unit* player = botAI->GetUnit(helper);
        if (!player || player == bot)
            continue;

        int32 dLevel = player->GetLevel() - botLevel;

        if (dLevel > -10 && bot->GetDistance(player) < 10.0f)
            friendPower += std::max(200 + 20 * dLevel, dLevel * 200);
    }

    return friendPower < foePower;
}

bool BuffTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    if (!SpellTrigger::IsActive())
        return false;
    Aura* aura = botAI->GetAura(spell, target, checkIsOwner, checkDuration);
    // #628 (AC GenericTriggers.cpp:165-166 + GenericBuffUtils.cpp:36-43 BuffBelowRefreshTarget): with no
    // force-rebuff pass pending this is exactly the old rule (missing, or below beforeDuration).
    if (!aura)
        return true;
    return botAI->forceRebuff.BuffBelowRefreshTarget(aura->GetDuration(), aura->GetMaxDuration(), beforeDuration);
}

Value<Unit*>* BuffOnPartyTrigger::GetTargetValue()
{
    return context->GetValue<Unit*>("party member without aura", spell);
}

bool ProtectPartyMemberTrigger::IsActive() { return AI_VALUE(Unit*, "party member to protect"); }

Value<Unit*>* DebuffOnAttackerTrigger::GetTargetValue()
{
    return context->GetValue<Unit*>("attacker without aura", spell);
}

Value<Unit*>* DebuffOnMeleeAttackerTrigger::GetTargetValue()
{
    return context->GetValue<Unit*>("melee attacker without aura", spell);
}

bool DuelOpponentUntargetedTrigger::IsActive()
{
    if (!bot->duel || !bot->duel->opponent || bot->duel->startTime == 0)
        return false;

    return bot->duel->opponent->IsAlive() && !AI_VALUE(Unit*, "current target");
}

bool NoAttackersTrigger::IsActive()
{
    return !AI_VALUE(Unit*, "current target") && AI_VALUE(uint8, "my attacker count") > 0;
}

bool InvalidTargetTrigger::IsActive() { return AI_VALUE2(bool, "invalid target", "current target"); }

bool NoTargetTrigger::IsActive() { return !AI_VALUE(Unit*, "current target"); }

bool MyAttackerCountTrigger::IsActive()
{
    return AI_VALUE2(bool, "combat", "self target") && AI_VALUE(uint8, "my attacker count") >= amount;
}

bool LowTankThreatTrigger::IsActive()
{
    Unit* current_target = AI_VALUE(Unit*, "current target");
    if (!current_target || current_target->IsPlayer())
        return false;  // players have no threat: Tricks of the Trade / Misdirection would be wasted

    Unit* mt = AI_VALUE(Unit*, "main tank");
    if (!mt)
        return false;

    ThreatManager& mgr = current_target->GetThreatManager();
    float threat = mgr.GetThreat(bot);
    float tankThreat = mgr.GetThreat(mt);
    return tankThreat == 0.0f || threat > tankThreat * 0.5f;
}

bool AoeTrigger::IsActive()
{
    Unit* current_target = AI_VALUE(Unit*, "current target");
    if (!current_target)
    {
        return false;
    }
    GuidVector attackers = context->GetValue<GuidVector>("attackers")->Get();
    int attackers_count = 0;
    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetDistance(current_target->GetPosition()) <= range)
        {
            attackers_count++;
        }
    }
    return attackers_count >= amount;
}

// #163 S8 gap fix (a): these were stubbed to `return false` pending "inventory items"/"conjured
// food"|"conjured water" qualifier support. That support already exists (InventoryAction.cpp's
// Find(), qualifiers "conjured food"/"conjured water") - only these two triggers hadn't been wired
// back up to it. AC's `sPlayerbotAIConfig.freeFood` random-bot gate doesn't exist in either this
// fork's or AC's current PlayerbotAIConfig, so it is dropped rather than invented.
bool NoFoodTrigger::IsActive()
{
    return AI_VALUE2(std::vector<Item*>, "inventory items", "conjured food").empty();
}

bool NoDrinkTrigger::IsActive()
{
    return AI_VALUE2(std::vector<Item*>, "inventory items", "conjured water").empty();
}

bool TargetInSightTrigger::IsActive() { return AI_VALUE(Unit*, "grind target"); }

bool DebuffTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
    {
        return false;
    }
    return BuffTrigger::IsActive() && (target->GetHealth() / AI_VALUE(float, "estimated group dps")) >= needLifeTime;
}

bool DebuffOnBossTrigger::IsActive()
{
    if (!DebuffTrigger::IsActive())
    {
        return false;
    }
    Creature* c = GetTarget()->ToCreature();
    return c && ((c->IsDungeonBoss()) || (c->isWorldBoss()));
}

bool SpellTrigger::IsActive() { return GetTarget(); }

bool SpellCanBeCastTrigger::IsActive()
{
    Unit* target = GetTarget();
    return target && botAI->CanCastSpell(spell, target);
}

bool SpellNoCooldownTrigger::IsActive()
{
    uint32 spellId = AI_VALUE2(uint32, "spell id", name);
    if (!spellId)
        return false;

    return !bot->HasSpellCooldown(spellId);
}

bool SpellCooldownTrigger::IsActive()
{
    uint32 spellId = AI_VALUE2(uint32, "spell id", name);
    if (!spellId)
        return false;

    return bot->HasSpellCooldown(spellId);
}

RandomTrigger::RandomTrigger(PlayerbotAI* botAI, std::string const name, int32 probability)
    : Trigger(botAI, name), probability(probability), lastCheck(getMSTime())
{
}

bool RandomTrigger::IsActive()
{
    if (getMSTime() - lastCheck < sPlayerbotAIConfig->repeatDelay)
        return false;

    lastCheck = getMSTime();
    int32 k = (int32)(probability / sPlayerbotAIConfig->randomChangeMultiplier);
    if (k < 1)
        k = 1;
    return (rand() % k) == 0;
}

bool AndTrigger::IsActive() { return ls && rs && ls->IsActive() && rs->IsActive(); }

std::string const AndTrigger::getName()
{
    std::string name(ls->getName());
    name = name + " and ";
    name = name + rs->getName();
    return name;
}

bool TwoTriggers::IsActive()
{
    if (name1.empty() || name2.empty())
        return false;

    Trigger* trigger1 = botAI->GetAiObjectContext()->GetTrigger(name1);
    Trigger* trigger2 = botAI->GetAiObjectContext()->GetTrigger(name2);

    if (!trigger1 || !trigger2)
        return false;

    return trigger1->IsActive() && trigger2->IsActive();
}

std::string const TwoTriggers::getName()
{
    std::string name;
    name = name1 + " and " + name2;
    return name;
}

bool BoostTrigger::IsActive()
{
    if (!BuffTrigger::IsActive())
        return false;
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && target->ToPlayer())
        return true;
    return AI_VALUE(uint8, "balance") <= balance;
}

bool GenericBoostTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && target->ToPlayer())
        return true;
    return AI_VALUE(uint8, "balance") <= balance;
}

bool HealerShouldAttackTrigger::IsActive()
{
    // nobody can help me
    if (botAI->GetNearGroupMemberCount(sPlayerbotAIConfig->sightDistance) <= 1)
        return true;

    if (AI_VALUE2(uint8, "health", "party member to heal") < sPlayerbotAIConfig->almostFullHealth)
        return false;

    // special check for resto druid (dont remove tree of life frequently)
    if (bot->GetAura(33891))
    {
        LastSpellCast& lastSpell = botAI->GetAiObjectContext()->GetValue<LastSpellCast&>("last spell cast")->Get();
        if (lastSpell.timer + 5 > time(nullptr))
            return false;
    }

    int manaThreshold;
    int balance = AI_VALUE(uint8, "balance");
    // higher threshold in higher pressure
    if (balance <= 50)
        manaThreshold = 85;
    else if (balance <= 100)
        manaThreshold = sPlayerbotAIConfig->highMana;
    else
        manaThreshold = sPlayerbotAIConfig->mediumMana;

    if (AI_VALUE2(bool, "has mana", "self target") && AI_VALUE2(uint8, "mana", "self target") < manaThreshold)
        return false;

    return true;
}

bool ItemCountTrigger::IsActive() { return AI_VALUE2(uint32, "item count", item) < count; }

bool InterruptSpellTrigger::IsActive()
{
    return SpellTrigger::IsActive() && botAI->IsInterruptableSpellCasting(GetTarget(), getName());
}

bool DeflectSpellTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    if (!target->IsNonMeleeSpellCasted(true))
        return false;

    if (target->GetTarget() != bot->GetGUID())
        return false;

    uint32 spellid = context->GetValue<uint32>("spell id", spell)->Get();
    if (!spellid)
        return false;

    SpellInfo const* deflectSpell = sSpellMgr->GetSpellInfo(spellid);
    if (!deflectSpell)
        return false;

    // warrior deflects all
    if (spell == "spell reflection")
        return true;

    // human priest feedback
    if (spell == "feedback")
        return true;

    SpellSchoolMask deflectSchool = SpellSchoolMask(deflectSpell->Effects[EFFECT_0].MiscValue);
    SpellSchoolMask attackSchool = SPELL_SCHOOL_MASK_NONE;

    if (Spell* spell = target->GetCurrentSpell(CURRENT_GENERIC_SPELL))
    {
        if (SpellInfo const* tarSpellInfo = spell->GetSpellInfo())
        {
            attackSchool = tarSpellInfo->GetSchoolMask();
            if (deflectSchool == attackSchool)
                return true;
        }
    }
    return false;
}

bool AttackerCountTrigger::IsActive() { return AI_VALUE(uint8, "attacker count") >= amount; }

bool HasAuraTrigger::IsActive() { return botAI->HasAura(getName(), GetTarget(), false, false, -1, true); }

bool HasAuraStackTrigger::IsActive()
{
    Aura* aura = botAI->GetAura(getName(), GetTarget(), false, true, stack);
    // sLog->outMessage("playerbot", LOG_LEVEL_DEBUG, "HasAuraStackTrigger::IsActive %s %d", getName(), aura ?
    // aura->GetStackAmount() : -1);
    return aura;
}

bool TimerTrigger::IsActive()
{
    if (time(nullptr) != lastCheck)
    {
        lastCheck = time(nullptr);
        return true;
    }

    return false;
}

bool HasNoAuraTrigger::IsActive() { return !botAI->HasAura(getName(), GetTarget()); }

bool TankAssistTrigger::IsActive()
{
    if (!AI_VALUE(uint8, "attacker count"))
        return false;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (!currentTarget)
        return true;

    Unit* tankTarget = AI_VALUE(Unit*, "tank target");
    if (!tankTarget || currentTarget == tankTarget)
        return false;

    return AI_VALUE2(bool, "has aggro", "current target");
}

bool IsBehindTargetTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return target && AI_VALUE2(bool, "behind", "current target");
}

bool IsNotBehindTargetTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return target && !AI_VALUE2(bool, "behind", "current target");
}

bool IsNotFacingTargetTrigger::IsActive() { return !AI_VALUE2(bool, "facing", "current target"); }

bool NoMovementTrigger::IsActive() { return !AI_VALUE2(bool, "moving", "self target"); }

bool NoPossibleTargetsTrigger::IsActive()
{
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    return !targets.size();
}

bool PossibleAddsTrigger::IsActive() { return AI_VALUE(bool, "possible adds") && !AI_VALUE(ObjectGuid, "pull target"); }

bool NotDpsTargetActiveTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    // do not switch if enemy target
    if (target && target->IsAlive())
    {
        Unit* enemy = AI_VALUE(Unit*, "enemy player target");
        if (target == enemy)
            return false;
    }

    Unit* dps = AI_VALUE(Unit*, "dps target");
    return dps && target != dps;
}

bool NotDpsAoeTargetActiveTrigger::IsActive()
{
    Unit* dps = AI_VALUE(Unit*, "dps aoe target");
    Unit* target = AI_VALUE(Unit*, "current target");
    Unit* enemy = AI_VALUE(Unit*, "enemy player target");

    // do not switch if enemy target
    if (target && target == enemy && target->IsAlive())
        return false;

    return dps && target != dps;
}

bool IsSwimmingTrigger::IsActive() { return AI_VALUE2(bool, "swimming", "self target"); }

// Ported from AC mod-playerbots StuckTriggers.cpp:144-170 (#149). Uses the "current position" and
// "combat" values, both already present on this fork (see ReleaseSpiritAction.cpp's
// RESET_AI_VALUE(bool, "combat::self target") / RESET_AI_VALUE(WorldPosition, "current position")).
bool CombatStuckTrigger::IsActive()
{
    if (!bot->IsInCombat())
        return false;

    // AC IsRealPlayer(Player*): a master with no PlayerbotAI attached is a person at a client.
    if (Player* master = botAI->GetMaster())
        if (!GET_PLAYERBOT_AI(master))
            return false;

    if (!botAI->AllowActivity(ALL_ACTIVITY))
        return false;

    MemoryCalculatedValue<bool>* combatVal =
        dynamic_cast<MemoryCalculatedValue<bool>*>(context->GetUntypedValue("combat::self target"));

    if (combatVal && combatVal->LastChangeDelay() > 5 * MINUTE)
        return true;

    return false;
}

bool HasNearestAddsTrigger::IsActive()
{
    GuidVector targets = AI_VALUE(GuidVector, "nearest adds");
    return targets.size();
}

bool HasItemForSpellTrigger::IsActive()
{
    std::string const spell = getName();
    uint32 spellId = AI_VALUE2(uint32, "spell id", spell);
    return spellId && AI_VALUE2(Item*, "item for spell", spellId);
}

bool TargetChangedTrigger::IsActive()
{
    Unit* oldTarget = context->GetValue<Unit*>("old target")->Get();
    Unit* target = context->GetValue<Unit*>("current target")->Get();
    return target && oldTarget != target;
}

Value<Unit*>* InterruptEnemyHealerTrigger::GetTargetValue()
{
    return context->GetValue<Unit*>("enemy healer target", spell);
}

bool RandomBotUpdateTrigger::IsActive() { return RandomTrigger::IsActive() && AI_VALUE(bool, "random bot update"); }

// #855 (epic #854): shared rotation triggers, see GenericTriggers.h.
bool EnemyCasterInRangeTrigger::IsActive() { return AI_VALUE(Unit*, "enemy caster target") != nullptr; }

bool BotIsCcTrigger::IsActive()
{
    return bot->IsAlive() && bot->HasUnitState(UNIT_STATE_STUNNED | UNIT_STATE_FLEEING | UNIT_STATE_CONFUSED);
}

bool EnemyPlayerInMeleeRangeTrigger::IsActive()
{
    if (!bot->IsInWorld() || !bot->IsAlive() || !bot->IsInCombat())
        return false;

    std::list<Player*> players;
    bot->GetPlayerListInGrid(players, NOMINAL_MELEE_RANGE);  // alive players, surface distance <= 5 yd
    for (Player* player : players)
    {
        if (player == bot || !bot->IsValidAttackTarget(player) || !bot->CanSeeOrDetect(player))
            continue;

        return true;
    }

    return false;
}


bool NewPlayerNearbyTrigger::IsActive() { return AI_VALUE(ObjectGuid, "new player nearby"); }

bool CollisionTrigger::IsActive() { return AI_VALUE2(bool, "collision", "self target"); }

bool GiveItemTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "party member without item", item) && AI_VALUE2(uint32, "item count", item);
}

bool GiveFoodTrigger::IsActive()
{
    return AI_VALUE(Unit*, "party member without food") && AI_VALUE2(uint32, "item count", item);
}

bool GiveWaterTrigger::IsActive()
{
    return AI_VALUE(Unit*, "party member without water") && AI_VALUE2(uint32, "item count", item);
}

Value<Unit*>* SnareTargetTrigger::GetTargetValue() { return context->GetValue<Unit*>("snare target", spell); }

bool StayTimeTrigger::IsActive()
{
    time_t stayTime = AI_VALUE(time_t, "stay time");
    time_t now = time(nullptr);
    return delay && stayTime && now > stayTime + 2 * delay / 1000;
}

// Ported from AC mod-playerbots Ai/Base/Trigger/GenericTriggers.cpp:681-691 (#177).
bool ReturnToStayPositionTrigger::IsActive()
{
    PositionInfo stayPosition = AI_VALUE(PositionMap&, "position")["stay"];
    if (stayPosition.isSet())
    {
        float const distance = bot->GetDistance(stayPosition.x, stayPosition.y, stayPosition.z);
        return distance > sPlayerbotAIConfig->followDistance;
    }

    return false;
}

bool IsMountedTrigger::IsActive() { return AI_VALUE2(bool, "mounted", "self target"); }

bool CorpseNearTrigger::IsActive()
{
    return bot->GetCorpse() && bot->GetCorpse()->IsWithinDistInMap(bot, CORPSE_RECLAIM_RADIUS, true);
}

// Ported from AC mod-playerbots Ai/Base/Trigger/LootTriggers.cpp:12-39 (issue #151), replacing the
// #149 5-yd corpse-scan diverged body. FarFromCurrentLootTrigger/CanLootTrigger (AC :41-50) are the
// new strategy/triggers/LootTriggers.h/.cpp; only this trigger stays declared here (see the class
// comment above) to avoid the ~21-TU GenericTriggers.h rebuild a move would cost.
bool LootAvailableTrigger::IsActive()
{
    if (!AI_VALUE(bool, "has available loot"))
        return false;

    bool distanceCheck = false;
    if (botAI->HasStrategy("stay", BOT_STATE_NON_COMBAT))
    {
        distanceCheck =
            sServerFacade->IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", "loot target"), CONTACT_DISTANCE);
    }
    else
    {
        distanceCheck = sServerFacade->IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", "loot target"),
                                                                  INTERACTION_DISTANCE - 2.0f);
    }

    // Loot target in range, or no hostile targets to deal with first.
    if (distanceCheck || AI_VALUE(GuidVector, "all targets").empty())
        return true;

    // A target that became not loot-possible after being selected - the bot was pulled into combat
    // while approaching it, or it despawned - never returns to range, so without this it stays
    // selected forever and the loot action never gets to pick another. Report active so it can.
    LootObject lootTarget = AI_VALUE(LootObject, "loot target");
    return !lootTarget.IsEmpty() && !lootTarget.IsLootPossible(bot);
}

bool IsFallingTrigger::IsActive() { return bot->HasUnitMovementFlag(MOVEMENTFLAG_FALLING); }

bool IsFallingFarTrigger::IsActive() { return bot->HasUnitMovementFlag(MOVEMENTFLAG_FALLING_FAR); }

bool HasAreaDebuffTrigger::IsActive() { return AI_VALUE2(bool, "has area debuff", "self target"); }

Value<Unit*>* BuffOnMainTankTrigger::GetTargetValue() { return context->GetValue<Unit*>("main tank", spell); }

bool HasCcTargetTrigger::IsCcTargetFree(Unit* ccTarget, Unit* rtiCcTarget)
{
    if (!ccTarget || (rtiCcTarget && ccTarget != rtiCcTarget))
        return false;

    return !AI_VALUE2(Unit*, "current cc target", getName());
}

bool HasCcTargetTrigger::IsActive()
{
    Unit* rtiCcTarget = nullptr;
    // 5.4.8: PlayerbotAI::IsInNonRaidDungeon() doesn't exist on this fork; Map::IsNonRaidDungeon()
    // (Map.h) is the direct core equivalent AC's helper itself calls into.
    if (bot->GetMap()->IsNonRaidDungeon())
    {
        rtiCcTarget = AI_VALUE(Unit*, "rti cc target");
        if (!rtiCcTarget)
            return false;
    }

    return IsCcTargetFree(AI_VALUE2(Unit*, "cc target", getName()), rtiCcTarget);
}

bool RtiCcTrigger::IsActive()
{
    Unit* rtiCcTarget = AI_VALUE(Unit*, "rti cc target");
    if (!rtiCcTarget)
        return false;

    Unit* ccTarget = AI_VALUE2(Unit*, "cc target", getName());
    if (ccTarget && ccTarget == rtiCcTarget)
        return IsCcTargetFree(ccTarget, rtiCcTarget);

    return botAI->CanCastSpell(getName(), rtiCcTarget);
}

bool NoRtiTrigger::IsActive()
{
    if (!bot->IsInCombat())
        return false;

    return AI_VALUE(Unit*, "rti target") == nullptr;
}

bool NewPetTrigger::IsActive()
{
    ObjectGuid currentPetGuid = ObjectGuid::Empty;

    if (Pet* pet = bot->GetPet())
        currentPetGuid = pet->GetGUID();
    else if (Guardian* guardian = bot->GetGuardianPet())
        currentPetGuid = guardian->GetGUID();

    if (currentPetGuid != lastPetGuid)
    {
        triggered = false;
        lastPetGuid = currentPetGuid;
    }

    if (currentPetGuid != ObjectGuid::Empty && !triggered)
    {
        triggered = true;
        return true;
    }

    return false;
}

bool LowEnergyTrigger::IsActive() { return AI_VALUE2(uint8, "energy", "self target") < threshold; }

bool LossOfControlTrigger::IsActive()
{
    return bot->HasAuraType(SPELL_AURA_MOD_STUN) || bot->HasAuraType(SPELL_AURA_MOD_FEAR) ||
           bot->HasAuraType(SPELL_AURA_MOD_ROOT) || bot->HasAuraType(SPELL_AURA_MOD_CONFUSE) ||
           bot->HasAuraType(SPELL_AURA_MOD_CHARM);
}

bool FearCharmSleepTrigger::IsActive()
{
    return bot->HasAuraType(SPELL_AURA_MOD_FEAR) || bot->HasAuraType(SPELL_AURA_MOD_CHARM) ||
           bot->HasAuraType(SPELL_AURA_AOE_CHARM) || bot->HasAuraWithMechanic(1 << MECHANIC_SLEEP);
}

bool PoisonDiseaseBleedTrigger::IsActive()
{
    return botAI->HasAuraToDispel(bot, DISPEL_POISON) || botAI->HasAuraToDispel(bot, DISPEL_DISEASE) ||
           bot->HasAuraWithMechanic(1 << MECHANIC_BLEED);
}

bool MovementImpairedTrigger::IsActive()
{
    // 5.4.8: PlayerbotAI::IsMovementImpaired() equivalent, inlined (see the class comment).
    bool const impaired = bot->HasAuraType(SPELL_AURA_MOD_ROOT) || bot->HasUnitState(UNIT_STATE_ROOT) ||
        bot->GetSpeedRate(MOVE_RUN) < 1.0f;
    return impaired && !botAI->HasAnyAuraOf(bot, "stealth", "prowl", nullptr);
}

// #628 (AC GenericTriggers.cpp:767-770)
bool ForceRebuffPendingTrigger::IsActive() { return botAI->forceRebuff.IsPending(); }
