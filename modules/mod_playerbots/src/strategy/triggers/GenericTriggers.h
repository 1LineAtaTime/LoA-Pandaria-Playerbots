#ifndef _PLAYERBOT_GENERICTRIGGERS_H
#define _PLAYERBOT_GENERICTRIGGERS_H

#include <utility>

#include "RangeTriggers.h"
#include "Trigger.h"

class PlayerbotAI;
class Unit;

class AlwaysTrigger : public Trigger
{
public:
    AlwaysTrigger(PlayerbotAI* ai) : Trigger(ai, "always trigger") {}
    bool IsActive() override { return true; }
};

class StatAvailable : public Trigger
{
public:
    StatAvailable(PlayerbotAI* botAI, int32 amount, std::string const name = "stat available")
        : Trigger(botAI, name), amount(amount)
    {
    }

protected:
    int32 amount;
};

class HighManaTrigger : public Trigger
{
public:
    HighManaTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high mana") {}

    bool IsActive() override;
};

class EnoughManaTrigger : public Trigger
{
public:
    EnoughManaTrigger(PlayerbotAI* botAI) : Trigger(botAI, "enough mana") {}

    bool IsActive() override;
};

class AlmostFullManaTrigger : public Trigger
{
public:
    AlmostFullManaTrigger(PlayerbotAI* botAI) : Trigger(botAI, "almost full mana") {}

    bool IsActive() override;
};

// Ported from AC mod-playerbots GenericTriggers.h:105-138 (#149). StatAvailable, the "combo" value and
// the "estimated group dps" value already existed on this fork.
class ComboPointsAvailableTrigger : public StatAvailable
{
public:
    ComboPointsAvailableTrigger(PlayerbotAI* botAI, int32 amount = 5)
        : StatAvailable(botAI, amount, "combo points available")
    {
    }

    bool IsActive() override;
};

class TargetWithComboPointsLowerHealTrigger : public ComboPointsAvailableTrigger
{
public:
    TargetWithComboPointsLowerHealTrigger(PlayerbotAI* botAI, int32 combo_point = 5, float lifeTime = 8.0f)
        : ComboPointsAvailableTrigger(botAI, combo_point), lifeTime(lifeTime)
    {
    }
    bool IsActive() override;

private:
    float lifeTime;
};

class ComboPointsNotFullTrigger : public StatAvailable
{
public:
    ComboPointsNotFullTrigger(PlayerbotAI* botAI, int32 amount = 5, std::string const name = "combo points not full")
        : StatAvailable(botAI, amount, name)
    {
    }

    bool IsActive() override;
};

class RageAvailable : public StatAvailable
{
public:
    RageAvailable(PlayerbotAI* botAI, int32 amount) : StatAvailable(botAI, amount, "rage available") {}

    bool IsActive() override;
};

class LightRageAvailableTrigger : public RageAvailable
{
public:
    LightRageAvailableTrigger(PlayerbotAI* botAI) : RageAvailable(botAI, 20) {}
};

class MediumRageAvailableTrigger : public RageAvailable
{
public:
    MediumRageAvailableTrigger(PlayerbotAI* botAI) : RageAvailable(botAI, 40) {}
};

class HighRageAvailableTrigger : public RageAvailable
{
public:
    HighRageAvailableTrigger(PlayerbotAI* botAI) : RageAvailable(botAI, 60) {}
};

class EnergyAvailable : public StatAvailable
{
public:
    EnergyAvailable(PlayerbotAI* botAI, int32 amount) : StatAvailable(botAI, amount, "energy available") {}

    bool IsActive() override;
};

class LightEnergyAvailableTrigger : public EnergyAvailable
{
public:
    LightEnergyAvailableTrigger(PlayerbotAI* botAI) : EnergyAvailable(botAI, 20) {}
};

class MediumEnergyAvailableTrigger : public EnergyAvailable
{
public:
    MediumEnergyAvailableTrigger(PlayerbotAI* botAI) : EnergyAvailable(botAI, 40) {}
};

class HighEnergyAvailableTrigger : public EnergyAvailable
{
public:
    HighEnergyAvailableTrigger(PlayerbotAI* botAI) : EnergyAvailable(botAI, 60) {}
};


class LoseAggroTrigger : public Trigger
{
public:
    LoseAggroTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lose aggro") {}

    bool IsActive() override;
};

class HasAggroTrigger : public Trigger
{
public:
    HasAggroTrigger(PlayerbotAI* botAI) : Trigger(botAI, "have aggro") {}

    bool IsActive() override;
};

class SpellTrigger : public Trigger
{
public:
    SpellTrigger(PlayerbotAI* botAI, std::string const spell, int32 checkInterval = 1)
        : Trigger(botAI, spell, checkInterval), spell(spell)
    {
    }

    std::string const GetTargetName() override { return "current target"; }
    std::string const getName() override { return spell; }
    bool IsActive() override;

protected:
    std::string spell;
};

class SpellCanBeCastTrigger : public SpellTrigger
{
public:
    SpellCanBeCastTrigger(PlayerbotAI* botAI, std::string const spell) : SpellTrigger(botAI, spell) {}

    bool IsActive() override;
};

class SpellNoCooldownTrigger : public SpellTrigger
{
public:
    SpellNoCooldownTrigger(PlayerbotAI* botAI, std::string const spell) : SpellTrigger(botAI, spell) {}

    bool IsActive() override;
};

class SpellCooldownTrigger : public SpellTrigger
{
public:
    SpellCooldownTrigger(PlayerbotAI* botAI, std::string const spell) : SpellTrigger(botAI, spell) {}

    std::string const GetTargetName() override { return "self target"; }
    bool IsActive() override;
};

// TODO: check other targets
class InterruptSpellTrigger : public SpellTrigger
{
public:
    InterruptSpellTrigger(PlayerbotAI* botAI, std::string const spell) : SpellTrigger(botAI, spell) {}

    bool IsActive() override;
};

class DeflectSpellTrigger : public SpellTrigger
{
public:
    DeflectSpellTrigger(PlayerbotAI* botAI, std::string const spell) : SpellTrigger(botAI, spell) {}

    bool IsActive() override;
};

class AttackerCountTrigger : public Trigger
{
public:
    AttackerCountTrigger(PlayerbotAI* botAI, int32 amount, float distance = sPlayerbotAIConfig->sightDistance)
        : Trigger(botAI), amount(amount), distance(distance)
    {
    }

    bool IsActive() override;
    std::string const getName() override { return "attacker count"; }

protected:
    int32 amount;
    float distance;
};

class HasAttackersTrigger : public AttackerCountTrigger
{
public:
    HasAttackersTrigger(PlayerbotAI* botAI) : AttackerCountTrigger(botAI, 1) {}
};

class MyAttackerCountTrigger : public AttackerCountTrigger
{
public:
    MyAttackerCountTrigger(PlayerbotAI* botAI, int32 amount) : AttackerCountTrigger(botAI, amount) {}

    bool IsActive() override;
    std::string const getName() override { return "my attacker count"; }
};

class MediumThreatTrigger : public MyAttackerCountTrigger
{
public:
    MediumThreatTrigger(PlayerbotAI* botAI) : MyAttackerCountTrigger(botAI, 2) {}
};

class LowTankThreatTrigger : public Trigger
{
public:
    LowTankThreatTrigger(PlayerbotAI* botAI) : Trigger(botAI, "low tank threat") {}
    bool IsActive() override;
};

class AoeTrigger : public AttackerCountTrigger
{
public:
    AoeTrigger(PlayerbotAI* botAI, int32 amount = 3, float range = 15.0f)
        : AttackerCountTrigger(botAI, amount), range(range)
    {
    }

    bool IsActive() override;
    std::string const getName() override { return "aoe"; }

private:
    float range;
};

class NoFoodTrigger : public Trigger
{
public:
    NoFoodTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no food trigger") {}

    bool IsActive() override;
};

class NoDrinkTrigger : public Trigger
{
public:
    NoDrinkTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no drink trigger") {}

    bool IsActive() override;
};

class LightAoeTrigger : public AoeTrigger
{
public:
    LightAoeTrigger(PlayerbotAI* botAI) : AoeTrigger(botAI, 2, 8.0f) {}
};

class MediumAoeTrigger : public AoeTrigger
{
public:
    MediumAoeTrigger(PlayerbotAI* botAI) : AoeTrigger(botAI, 3, 8.0f) {}
};

class HighAoeTrigger : public AoeTrigger
{
public:
    HighAoeTrigger(PlayerbotAI* botAI) : AoeTrigger(botAI, 4, 8.0f) {}
};

class BuffTrigger : public SpellTrigger
{
public:
    BuffTrigger(PlayerbotAI* botAI, std::string const spell, int32 checkInterval = 1, bool checkIsOwner = false, bool checkDuration = false, uint32 beforeDuration = 0)
        : SpellTrigger(botAI, spell, checkInterval)
    {
        this->checkIsOwner = checkIsOwner;
        this->checkDuration = checkDuration;
        this->beforeDuration = beforeDuration;
    }

public:
    std::string const GetTargetName() override { return "self target"; }
    bool IsBuffTrigger() override { return true; }  // AC GenericTriggers.h:324 (#628)
    bool IsActive() override;

protected:
    bool checkIsOwner;
    bool checkDuration;
    uint32 beforeDuration;
};

class BuffOnPartyTrigger : public BuffTrigger
{
public:
    BuffOnPartyTrigger(PlayerbotAI* botAI, std::string const spell, int32 checkInterval = 1)
        : BuffTrigger(botAI, spell, checkInterval)
    {
    }

    Value<Unit*>* GetTargetValue() override;
    std::string const getName() override { return spell + " on party"; }
};

class ProtectPartyMemberTrigger : public Trigger
{
public:
    ProtectPartyMemberTrigger(PlayerbotAI* botAI) : Trigger(botAI, "protect party member") {}

    std::string const GetTargetName() override { return "party member to protect"; }
    bool IsActive() override;
};

class NoAttackersTrigger : public Trigger
{
public:
    NoAttackersTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no attackers") {}

    bool IsActive() override;
};

// #868 (beyond AC): in a started duel the bot has no current target while its opponent is alive (it dropped the target
// after a flee/LOS 'invalid target', or never picked one). AC's "no attackers" -> "attack duel opponent" never fires in a
// duel because the opponent is always an attacker. w76/w77: hunter Merick idle 14 s, holy priest Chrisandy idle 83 s.
class DuelOpponentUntargetedTrigger : public Trigger
{
public:
    DuelOpponentUntargetedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "duel opponent untargeted") {}

    bool IsActive() override;
};

class NoTargetTrigger : public Trigger
{
public:
    NoTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no target") {}

    bool IsActive() override;
};

class InvalidTargetTrigger : public Trigger
{
public:
    InvalidTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "invalid target") {}

    bool IsActive() override;
};

class TargetInSightTrigger : public Trigger
{
public:
    TargetInSightTrigger(PlayerbotAI* botAI) : Trigger(botAI, "target in sight") {}

    bool IsActive() override;
};

class DebuffTrigger : public BuffTrigger
{
public:
    DebuffTrigger(PlayerbotAI* botAI, std::string const spell, int32 checkInterval = 1, bool checkIsOwner = false,
        float needLifeTime = 8.0f, uint32 beforeDuration = 0)
        : BuffTrigger(botAI, spell, checkInterval, checkIsOwner, false, beforeDuration), needLifeTime(needLifeTime)
    {
    }

    std::string const GetTargetName() override { return "current target"; }
    bool IsDebuffTrigger() override { return true; }  // AC GenericTriggers.h:393 (#628)
    bool IsActive() override;

protected:
    float needLifeTime;
};

class DebuffOnBossTrigger : public DebuffTrigger
{
public:
    DebuffOnBossTrigger(PlayerbotAI* botAI, std::string const spell, int32 checkInterval = 1, bool checkIsOwner = false)
        : DebuffTrigger(botAI, spell, checkInterval, checkIsOwner)
    {
    }
    bool IsActive() override;
};

class DebuffOnAttackerTrigger : public DebuffTrigger
{
public:
    DebuffOnAttackerTrigger(PlayerbotAI* botAI, std::string const spell, bool checkIsOwner = true,
        float needLifeTime = 8.0f)
        : DebuffTrigger(botAI, spell, 1, checkIsOwner, needLifeTime)
    {
    }

    Value<Unit*>* GetTargetValue() override;
    std::string const getName() override { return spell + " on attacker"; }
};

class DebuffOnMeleeAttackerTrigger : public DebuffTrigger
{
public:
    DebuffOnMeleeAttackerTrigger(PlayerbotAI* botAI, std::string const spell, bool checkIsOwner = true,
        float needLifeTime = 8.0f)
        : DebuffTrigger(botAI, spell, 1, checkIsOwner, needLifeTime)
    {
    }

    Value<Unit*>* GetTargetValue() override;
    std::string const getName() override { return spell + " on attacker"; }
};

class BoostTrigger : public BuffTrigger
{
public:
    BoostTrigger(PlayerbotAI* botAI, std::string const spell, float balance = 50.f)
        : BuffTrigger(botAI, spell, 1), balance(balance)
    {
    }

    bool IsActive() override;

protected:
    float balance;
};

class GenericBoostTrigger : public Trigger
{
public:
    GenericBoostTrigger(PlayerbotAI* botAI, float balance = 50.f)
        : Trigger(botAI, "generic boost", 1), balance(balance)
    {
    }

    bool IsActive() override;

protected:
    float balance;
};

class HealerShouldAttackTrigger : public Trigger
{
public:
    HealerShouldAttackTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "healer should attack", 1)
    {
    }

    bool IsActive() override;
};

class RandomTrigger : public Trigger
{
public:
    RandomTrigger(PlayerbotAI* botAI, std::string const name, int32 probability = 7);

    bool IsActive() override;

protected:
    int32 probability;
    uint32 lastCheck;
};

class AndTrigger : public Trigger
{
public:
    AndTrigger(PlayerbotAI* botAI, Trigger* ls, Trigger* rs) : Trigger(botAI), ls(ls), rs(rs) {}
    virtual ~AndTrigger()
    {
        delete ls;
        delete rs;
    }

    bool IsActive() override;
    std::string const getName() override;

protected:
    Trigger* ls;
    Trigger* rs;
};

class TwoTriggers : public Trigger
{
public:
    explicit TwoTriggers(PlayerbotAI* botAI, std::string name1 = "", std::string name2 = "") : Trigger(botAI)
    {
        this->name1 = std::move(name1);
        this->name2 = std::move(name2);
    }
    bool IsActive() override;
    std::string const getName() override;

protected:
    std::string name1;
    std::string name2;
};

class SnareTargetTrigger : public DebuffTrigger
{
public:
    SnareTargetTrigger(PlayerbotAI* botAI, std::string const spell) : DebuffTrigger(botAI, spell) {}

    Value<Unit*>* GetTargetValue() override;
    std::string const getName() override { return spell + " on snare target"; }
};

class LowManaTrigger : public Trigger
{
public:
    LowManaTrigger(PlayerbotAI* botAI) : Trigger(botAI, "low mana") {}

    bool IsActive() override;
};

class MediumManaTrigger : public Trigger
{
public:
    MediumManaTrigger(PlayerbotAI* botAI) : Trigger(botAI, "medium mana") {}

    bool IsActive() override;
};

BEGIN_TRIGGER(PanicTrigger, Trigger)
std::string const getName() override { return "panic"; }
END_TRIGGER()

BEGIN_TRIGGER(OutNumberedTrigger, Trigger)
std::string const getName() override { return "outnumbered"; }
END_TRIGGER()

class NoPetTrigger : public Trigger
{
public:
    NoPetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no pet", 5 * 1000) {}

    virtual bool IsActive() override;
};

class HasPetTrigger : public Trigger
{
public:
    HasPetTrigger(PlayerbotAI* ai) : Trigger(ai, "has pet", 5 * 1000) {}

    virtual bool IsActive() override;
};

class PetAttackTrigger : public Trigger
{
public:
    PetAttackTrigger(PlayerbotAI* ai) : Trigger(ai, "pet attack") {}

    virtual bool IsActive() override;
};

class ItemCountTrigger : public Trigger
{
public:
    ItemCountTrigger(PlayerbotAI* botAI, std::string const item, int32 count, int32 interval = 30 * 1000)
        : Trigger(botAI, item, interval), item(item), count(count)
    {
    }

    bool IsActive() override;
    std::string const getName() override { return "item count"; }

protected:
    std::string const item;
    int32 count;
};

class AmmoCountTrigger : public ItemCountTrigger
{
public:
    AmmoCountTrigger(PlayerbotAI* botAI, std::string const item, uint32 count = 1, int32 interval = 30 * 1000)
        : ItemCountTrigger(botAI, item, count, interval)
    {
    }
};

class HasAuraTrigger : public Trigger
{
public:
    HasAuraTrigger(PlayerbotAI* botAI, std::string const spell, int32 checkInterval = 1)
        : Trigger(botAI, spell, checkInterval)
    {
    }

    std::string const GetTargetName() override { return "self target"; }
    bool IsActive() override;
};

class HasAuraStackTrigger : public Trigger
{
public:
    HasAuraStackTrigger(PlayerbotAI* ai, std::string spell, int stack, int checkInterval = 1)
        : Trigger(ai, spell, checkInterval), stack(stack)
    {
    }

    std::string const GetTargetName() override { return "self target"; }
    bool IsActive() override;

private:
    int stack;
};

class HasNoAuraTrigger : public Trigger
{
public:
    HasNoAuraTrigger(PlayerbotAI* botAI, std::string const spell) : Trigger(botAI, spell) {}

    std::string const GetTargetName() override { return "self target"; }
    bool IsActive() override;
};

class TimerTrigger : public Trigger
{
public:
    TimerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "timer"), lastCheck(0) {}

    bool IsActive() override;

private:
    time_t lastCheck;
};

class TankAssistTrigger : public NoAttackersTrigger
{
public:
    TankAssistTrigger(PlayerbotAI* botAI) : NoAttackersTrigger(botAI) {}

    bool IsActive() override;
};

class IsBehindTargetTrigger : public Trigger
{
public:
    IsBehindTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "behind target") {}

    bool IsActive() override;
};

class IsNotBehindTargetTrigger : public Trigger
{
public:
    IsNotBehindTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "is not behind target") {}

    bool IsActive() override;
};

class IsNotFacingTargetTrigger : public Trigger
{
public:
    IsNotFacingTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "not facing target") {}

    bool IsActive() override;
};

class NoMovementTrigger : public Trigger
{
public:
    NoMovementTrigger(PlayerbotAI* botAI, std::string const name) : Trigger(botAI, name) {}

    bool IsActive() override;
};

class NoPossibleTargetsTrigger : public Trigger
{
public:
    NoPossibleTargetsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no possible targets") {}

    bool IsActive() override;
};

class NotDpsTargetActiveTrigger : public Trigger
{
public:
    NotDpsTargetActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "not dps target active") {}

    bool IsActive() override;
};

class NotDpsAoeTargetActiveTrigger : public Trigger
{
public:
    NotDpsAoeTargetActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "not dps aoe target active") {}

    bool IsActive() override;
};

class PossibleAddsTrigger : public Trigger
{
public:
    PossibleAddsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "possible adds") {}

    bool IsActive() override;
};

class IsSwimmingTrigger : public Trigger
{
public:
    IsSwimmingTrigger(PlayerbotAI* botAI) : Trigger(botAI, "swimming") {}

    bool IsActive() override;
};

// Ported from AC mod-playerbots StuckTriggers.h/.cpp (#149). Only "combat stuck" is referenced by any
// MoP strategy; "move stuck"/"move long stuck"/"combat long stuck" are not ported since nothing uses
// them. Uses the "current position" and "combat" values, both already present on this fork.
class CombatStuckTrigger : public Trigger
{
public:
    CombatStuckTrigger(PlayerbotAI* botAI) : Trigger(botAI, "combat stuck", 5) {}

    bool IsActive() override;
};

class HasNearestAddsTrigger : public Trigger
{
public:
    HasNearestAddsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "has nearest adds") {}

    bool IsActive() override;
};

class HasItemForSpellTrigger : public Trigger
{
public:
    HasItemForSpellTrigger(PlayerbotAI* botAI, std::string const spell) : Trigger(botAI, spell) {}

    bool IsActive() override;
};

class TargetChangedTrigger : public Trigger
{
public:
    TargetChangedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "target changed") {}

    bool IsActive() override;
};

class InterruptEnemyHealerTrigger : public SpellTrigger
{
public:
    InterruptEnemyHealerTrigger(PlayerbotAI* botAI, std::string const spell) : SpellTrigger(botAI, spell) {}

    Value<Unit*>* GetTargetValue() override;
    std::string const getName() override { return spell + " on enemy healer"; }
};

// #855 (epic #854, rotation lane 0): shared rotation triggers.
// "enemy caster in range": the "enemy caster target" value found a hostile casting an interruptible spell within 30 yd
// (MoP priority group 10 interrupt rows, condition 17 "casting any spell (interruptible)", target type 2).
class EnemyCasterInRangeTrigger : public Trigger
{
public:
    EnemyCasterInRangeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "enemy caster in range") {}

    std::string const GetTargetName() override { return "enemy caster target"; }
    bool IsActive() override;
};

// "bot is cc": the bot is stunned, feared or confused (unit state without the
// possessed / charging bits) - escapes such as Berserker Rage / Every Man for Himself.
class BotIsCcTrigger : public Trigger
{
public:
    BotIsCcTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bot is cc") {}

    bool IsActive() override;
};

// "enemy player in melee range": an attackable enemy player within 5 yd of the bot while it is in combat (MoP Blink /
// Frost Nova / Disengage / Psychic Scream rows: condition 3 EnemiesInRange with a TypeID player sub-filter, 3-5 yd).
class EnemyPlayerInMeleeRangeTrigger : public Trigger
{
public:
    EnemyPlayerInMeleeRangeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "enemy player in melee range") {}

    bool IsActive() override;
};

class RandomBotUpdateTrigger : public RandomTrigger
{
public:
    RandomBotUpdateTrigger(PlayerbotAI* botAI) : RandomTrigger(botAI, "random bot update", 30 * 1000) {}

    bool IsActive() override;
};

class NewPlayerNearbyTrigger : public Trigger
{
public:
    NewPlayerNearbyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "new player nearby", 10 * 1000) {}

    bool IsActive() override;
};

class CollisionTrigger : public Trigger
{
public:
    CollisionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "collision", 5 * 1000) {}

    bool IsActive() override;
};

class StayTimeTrigger : public Trigger
{
public:
    StayTimeTrigger(PlayerbotAI* botAI, uint32 delay, std::string const name)
        : Trigger(botAI, name, 5 * 1000), delay(delay)
    {
    }

    bool IsActive() override;

private:
    uint32 delay;
};

class SitTrigger : public StayTimeTrigger
{
public:
    SitTrigger(PlayerbotAI* botAI) : StayTimeTrigger(botAI, sPlayerbotAIConfig->sitDelay, "sit") {}
};

// Ported from AC mod-playerbots Ai/Base/Trigger/GenericTriggers.h (#177). StayStrategy schedules
// this to snap the bot back to its "stay" position once it has been pushed too far from it.
class ReturnToStayPositionTrigger : public Trigger
{
public:
    ReturnToStayPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "return to stay position", 2) {}

    bool IsActive() override;
};

class ReturnTrigger : public StayTimeTrigger
{
public:
    ReturnTrigger(PlayerbotAI* botAI) : StayTimeTrigger(botAI, sPlayerbotAIConfig->returnDelay, "return") {}
};

class GiveItemTrigger : public Trigger
{
public:
    GiveItemTrigger(PlayerbotAI* botAI, std::string const name, std::string const item)
        : Trigger(botAI, name, 2 * 1000), item(item)
    {
    }

    bool IsActive() override;

protected:
    std::string const item;
};

class GiveFoodTrigger : public GiveItemTrigger
{
public:
    GiveFoodTrigger(PlayerbotAI* botAI) : GiveItemTrigger(botAI, "give food", "conjured food") {}

    bool IsActive() override;
};

class GiveWaterTrigger : public GiveItemTrigger
{
public:
    GiveWaterTrigger(PlayerbotAI* botAI) : GiveItemTrigger(botAI, "give water", "conjured water") {}

    bool IsActive() override;
};

class IsMountedTrigger : public Trigger
{
public:
    IsMountedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mounted", 1) {}

    bool IsActive() override;
};

class CorpseNearTrigger : public Trigger
{
public:
    CorpseNearTrigger(PlayerbotAI* botAI) : Trigger(botAI, "corpse near", 1 * 1000) {}

    bool IsActive() override;
};

// #149: AC's LootAvailableTrigger (Ai/Base/Trigger/LootTriggers.h) drives this off a LootObjectStack
// plus "has available loot"/"loot target" AI values this fork does not have (LootAction.h - this fork
// takes the simple solo-corpse case, issue #62). Reuses LootAction's own lootable-corpse scan instead
// so the trigger and the action it gates never disagree. Registered generically (TriggerContext.h),
// same as AC, so every class - not just DK's corpse-explosion use (UnholyDKStrategy.cpp) - can see it.
class LootAvailableTrigger : public Trigger
{
public:
    LootAvailableTrigger(PlayerbotAI* botAI) : Trigger(botAI, "loot available") {}

    bool IsActive() override;
};

class IsFallingTrigger : public Trigger
{
public:
    IsFallingTrigger(PlayerbotAI* botAI) : Trigger(botAI, "falling", 10 * 1000) {}

    bool IsActive() override;
};

class IsFallingFarTrigger : public Trigger
{
public:
    IsFallingFarTrigger(PlayerbotAI* botAI) : Trigger(botAI, "falling far", 10 * 1000) {}

    bool IsActive() override;
};

class HasAreaDebuffTrigger : public Trigger
{
public:
    HasAreaDebuffTrigger(PlayerbotAI* botAI) : Trigger(botAI, "have area debuff") {}

    bool IsActive() override;
};

class BuffOnMainTankTrigger : public BuffTrigger
{
public:
    BuffOnMainTankTrigger(PlayerbotAI* botAI, std::string spell, bool checkIsOwner = false, int checkInterval = 1)
        : BuffTrigger(botAI, spell, checkInterval, checkIsOwner)
    {
    }

public:
    virtual Value<Unit*>* GetTargetValue();
};

// #163 S8 gap fix (b) (AC parity, Ai/Base/Trigger/GenericTriggers.h:685-694 + RtiTriggers.h): is the
// CC target for this ability (named by getName(), e.g. "cyclone"/"hibernate") free to CC - in a
// non-raid dungeon it must match the RTI-assigned CC icon target; elsewhere any found "cc target"
// works - and not already crowd-controlled ("current cc target").
class HasCcTargetTrigger : public Trigger
{
public:
    HasCcTargetTrigger(PlayerbotAI* botAI, std::string const name) : Trigger(botAI, name) {}

    bool IsActive() override;

protected:
    bool IsCcTargetFree(Unit* ccTarget, Unit* rtiCcTarget);
};

// #163 S8 gap fix (b) (AC parity, RtiTriggers.h): same as above, but also fires when nothing has
// been assigned to "cc target" yet and the ability can still be cast on the RTI-marked target - this
// is the trigger AC's Cyclone/Hibernate/Entangling Roots use.
class RtiCcTrigger : public HasCcTargetTrigger
{
public:
    RtiCcTrigger(PlayerbotAI* botAI, std::string const name) : HasCcTargetTrigger(botAI, name) {}

    bool IsActive() override;
};

// #163 S8 (AC parity, Ai/Base/Trigger/RtiTriggers.h:15-20): no raid-target icon assigned while the
// bot is in combat (icon assignment while merely grouped, not fighting, is a chat command instead).
class NoRtiTrigger : public Trigger
{
public:
    NoRtiTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no rti target") {}

    bool IsActive() override;
};

// #163 S8 (AC parity, Ai/Base/Trigger/GenericTriggers.h): fires once, the tick a new pet/guardian
// appears (summon or resummon), so class strategies can schedule "set pet stance".
class NewPetTrigger : public Trigger
{
public:
    NewPetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "new pet"), lastPetGuid(ObjectGuid::Empty), triggered(false) {}

    bool IsActive() override;

private:
    ObjectGuid lastPetGuid;
    bool triggered;
};

// #163 S8 (AC parity, Ai/Base/Trigger/GenericTriggers.h): the six generic triggers RacialsStrategy
// needs, none of which existed on this fork yet.
class LowEnergyTrigger : public Trigger
{
public:
    LowEnergyTrigger(PlayerbotAI* botAI, uint8 threshold = 30) : Trigger(botAI, "low energy"), threshold(threshold) {}

    bool IsActive() override;

private:
    uint8 threshold;
};

class LossOfControlTrigger : public Trigger
{
public:
    LossOfControlTrigger(PlayerbotAI* botAI) : Trigger(botAI, "loss of control", 1) {}

    bool IsActive() override;
};

class FearCharmSleepTrigger : public Trigger
{
public:
    FearCharmSleepTrigger(PlayerbotAI* botAI) : Trigger(botAI, "fear charm sleep", 1) {}

    bool IsActive() override;
};

class PoisonDiseaseBleedTrigger : public Trigger
{
public:
    PoisonDiseaseBleedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "poison disease bleed", 1) {}

    bool IsActive() override;
};

// 5.4.8: PlayerbotAI::IsMovementImpaired() doesn't exist on this fork (see the IsSnared() helpers in
// PaladinActions.cpp/PartyMemberSnaredTargetValue.cpp for the equivalent check); reimplemented here
// rather than adding to PlayerbotAI.h.
class MovementImpairedTrigger : public Trigger
{
public:
    MovementImpairedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "movement impaired", 1) {}

    bool IsActive() override;
};

// #628 (AC GenericTriggers.h:979-985): fires while a force-rebuff pass is pending (ForceRebuffStrategy).
class ForceRebuffPendingTrigger : public Trigger
{
public:
    ForceRebuffPendingTrigger(PlayerbotAI* botAI) : Trigger(botAI, "force rebuff pending") {}

    bool IsActive() override;
};
#endif
