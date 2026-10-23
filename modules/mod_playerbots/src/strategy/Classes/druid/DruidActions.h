/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DRUIDACTIONS_H
#define _PLAYERBOT_DRUIDACTIONS_H

#include "GenericSpellActions.h"
#include "SharedDefines.h"

class PlayerbotAI;
class Unit;

class CastFaerieFireAction : public CastDebuffSpellAction
{
public:
    CastFaerieFireAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "faerie fire") {}
};

// #384: 5.x has one Faerie Fire (770); "faerie fire (feral)" is NO-SUCH-NAME in the 5.4.8 DBC (cls.py), and
// SpellIdValue matches spells by exact name, so the old string could never resolve. The action keeps its
// registry/log name via getName().
class CastFaerieFireFeralAction : public CastSpellAction
{
public:
    CastFaerieFireFeralAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "faerie fire") {}
    std::string const getName() override { return "faerie fire (feral)"; }
};

class CastRejuvenationAction : public CastHealingSpellAction
{
public:
    CastRejuvenationAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "rejuvenation") {}
};

class CastRegrowthAction : public CastHealingSpellAction
{
public:
    CastRegrowthAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "regrowth") {}
};

class CastHealingTouchAction : public CastHealingSpellAction
{
public:
    CastHealingTouchAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "healing touch") {}
};

class CastRejuvenationOnPartyAction : public HealPartyMemberAction
{
public:
    CastRejuvenationOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "rejuvenation", 15.0f, HealingManaEfficiency::VERY_HIGH)
    {
    }
};

class CastRegrowthOnPartyAction : public HealPartyMemberAction
{
public:
    CastRegrowthOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "regrowth", 35.0f, HealingManaEfficiency::HIGH)
    {
    }
};

class CastHealingTouchOnPartyAction : public HealPartyMemberAction
{
public:
    CastHealingTouchOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "healing touch", 50.0f, HealingManaEfficiency::LOW)
    {
    }
};

class CastReviveAction : public ResurrectPartyMemberAction
{
public:
    CastReviveAction(PlayerbotAI* botAI) : ResurrectPartyMemberAction(botAI, "revive") {}

    NextAction** getPrerequisites() override;
};

class CastRebirthAction : public ResurrectPartyMemberAction
{
public:
    CastRebirthAction(PlayerbotAI* botAI) : ResurrectPartyMemberAction(botAI, "rebirth") {}

    NextAction** getPrerequisites() override;
    bool isUseful() override;
};

class CastMarkOfTheWildAction : public CastBuffSpellAction
{
public:
    CastMarkOfTheWildAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "mark of the wild") {}
};

class CastMarkOfTheWildOnPartyAction : public BuffOnPartyAction
{
public:
    CastMarkOfTheWildOnPartyAction(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, "mark of the wild") {}
};

class CastSurvivalInstinctsAction : public CastBuffSpellAction
{
public:
    CastSurvivalInstinctsAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "survival instincts") {}
};

class CastFrenziedRegenerationAction : public CastBuffSpellAction
{
public:
    CastFrenziedRegenerationAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "frenzied regeneration") {}
};

class CastThornsAction : public CastBuffSpellAction
{
public:
    CastThornsAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "thorns") {}
};

class CastThornsOnPartyAction : public BuffOnPartyAction
{
public:
    CastThornsOnPartyAction(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, "thorns") {}
};

class CastThornsOnMainTankAction : public BuffOnMainTankAction
{
public:
    CastThornsOnMainTankAction(PlayerbotAI* botAI) : BuffOnMainTankAction(botAI, "thorns", false) {}
};

class CastOmenOfClarityAction : public CastBuffSpellAction
{
public:
    CastOmenOfClarityAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "omen of clarity") {}
};

// #363 (AC parity, Ai/Class/Druid/Action/DruidActions.h:139-145): Lifebloom on the main tank. AC
// re-seeds when the bloom is missing, below 3 stacks, or about to expire. Body in the .cpp -
// Aura::GetStackAmount()/GetDuration() need SpellAuras.h, which the header TU does not pull in
// (hard rule 7/2).
class CastLifebloomOnMainTankAction : public BuffOnMainTankAction
{
public:
    CastLifebloomOnMainTankAction(PlayerbotAI* botAI) : BuffOnMainTankAction(botAI, "lifebloom", true) {}

    bool isUseful() override;
};

class CastWrathAction : public CastSpellAction
{
public:
    CastWrathAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "wrath") {}
};

class CastStarfallAction : public CastSpellAction
{
public:
    CastStarfallAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "starfall") {}
};

class CastHurricaneAction : public CastSpellAction
{
public:
    CastHurricaneAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "hurricane") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};

class CastMoonfireAction : public CastDebuffSpellAction
{
public:
    CastMoonfireAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "moonfire", true) {}
};

// #384: Insect Swarm (5570) is NOT-LEARNABLE in 5.4.8; Sunfire (93402, balance, level 18) replaces it (Wowhead
// mop-classic 93402, 2026-10-04). getName() keeps the AC registry/log name.
class CastInsectSwarmAction : public CastDebuffSpellAction
{
public:
    CastInsectSwarmAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "sunfire", true) {}
    std::string const getName() override { return "insect swarm"; }
};

class CastStarfireAction : public CastSpellAction
{
public:
    CastStarfireAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "starfire") {}
};

class CastEntanglingRootsAction : public CastSpellAction
{
public:
    CastEntanglingRootsAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "entangling roots") {}
};

class CastEntanglingRootsCcAction : public CastSpellAction
{
public:
    CastEntanglingRootsCcAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "entangling roots on cc") {}
    Value<Unit*>* GetTargetValue() override;
    bool Execute(Event event) override;
};

class CastHibernateAction : public CastSpellAction
{
public:
    CastHibernateAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "hibernate") {}
};

class CastHibernateCcAction : public CastSpellAction
{
public:
    CastHibernateCcAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "hibernate on cc") {}
    Value<Unit*>* GetTargetValue() override;
    bool Execute(Event event) override;
};

// #163 S8 (AC parity, Ai/Class/Druid/DruidActions.h:222): Cyclone, unchanged since introduction -
// 5.4.8 spell id 33786. Same pattern as CastHibernateAction/CastHibernateCcAction above.
class CastCycloneAction : public CastSpellAction
{
public:
    CastCycloneAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "cyclone") {}
};

class CastCycloneCcAction : public CastSpellAction
{
public:
    CastCycloneCcAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "cyclone on cc") {}
    Value<Unit*>* GetTargetValue() override;
    bool Execute(Event event) override;
};

class CastNaturesGraspAction : public CastBuffSpellAction
{
public:
    CastNaturesGraspAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "nature's grasp") {}
};

// #384: Cure Poison (cls.py: NOT-LEARNABLE, ids 26677) and Abolish Poison (14253) are gone in 5.4.8. Their 5.x
// successor is Remove Corruption (2782; spec102/103/104 level 22; removes Curse + Poison) for balance/feral/guardian
// and Nature's Cure (88423; spec105 level 22; Magic + Curse + Poison) for restoration - Wowhead mop-classic 2782 and
// 88423, 2026-10-04. DruidDispelSpell() picks whichever one this bot knows (defined in DruidActions.cpp). The
// actions keep their AC registry/log names via getName(), so every strategy edge and the party-dispel triggers are
// unchanged.
std::string const DruidDispelSpell(PlayerbotAI* botAI);

class CastCurePoisonAction : public CastCureSpellAction
{
public:
    CastCurePoisonAction(PlayerbotAI* botAI);
    std::string const getName() override { return "cure poison"; }
};

class CastCurePoisonOnPartyAction : public CurePartyMemberAction
{
public:
    CastCurePoisonOnPartyAction(PlayerbotAI* botAI);
    std::string const getName() override { return "cure poison on party"; }
};

class CastAbolishPoisonAction : public CastCureSpellAction
{
public:
    CastAbolishPoisonAction(PlayerbotAI* botAI);
    std::string const getName() override { return "abolish poison"; }
    NextAction** getAlternatives() override;
};

class CastAbolishPoisonOnPartyAction : public CurePartyMemberAction
{
public:
    CastAbolishPoisonOnPartyAction(PlayerbotAI* botAI);
    std::string const getName() override { return "abolish poison on party"; }

    NextAction** getAlternatives() override;
};

class CastBarkskinAction : public CastBuffSpellAction
{
public:
    CastBarkskinAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "barkskin") {}
};

class CastInnervateAction : public CastSpellAction
{
public:
    CastInnervateAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "innervate") {}

    std::string const GetTargetName() override { return "self target"; }
};

// #363 (AC parity, Ai/Class/Druid/Action/DruidActions.h:278-286): Innervate handed to the party
// healer who is low on mana, instead of self-buffing. The target value is the shared
// "healer low mana" (strategy/value/ValueContext.h:209, strategy/triggers/TriggerContext.h:52).
class CastInnervateOnHealerAction : public CastSpellAction
{
public:
    CastInnervateOnHealerAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "innervate") {}

    std::string const GetTargetName() override { return "healer low mana"; }
    bool isPossible() override;
    NextAction** getPrerequisites() override;
};

class CastTranquilityAction : public CastAoeHealSpellAction
{
public:
    CastTranquilityAction(PlayerbotAI* botAI)
        : CastAoeHealSpellAction(botAI, "tranquility", 15.0f, HealingManaEfficiency::MEDIUM)
    {
    }
};

class CastNaturesSwiftnessAction : public CastBuffSpellAction
{
public:
    CastNaturesSwiftnessAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "nature's swiftness") {}
};

class CastWildGrowthOnPartyAction : public HealPartyMemberAction
{
public:
    CastWildGrowthOnPartyAction(PlayerbotAI* ai)
        : HealPartyMemberAction(ai, "wild growth", 15.0f, HealingManaEfficiency::VERY_HIGH)
    {
    }
};

class CastPartySwiftmendAction : public HealPartyMemberAction
{
public:
    CastPartySwiftmendAction(PlayerbotAI* ai)
        : HealPartyMemberAction(ai, "swiftmend", 15.0f, HealingManaEfficiency::MEDIUM)
    {
    }
};

class CastPartyNourishAction : public HealPartyMemberAction
{
public:
    CastPartyNourishAction(PlayerbotAI* ai) : HealPartyMemberAction(ai, "nourish", 25.0f, HealingManaEfficiency::LOW) {}
};

// #384: "remove curse" was the 3.3.5 druid spell (it is a mage spell in 5.4.8); the druid's curse dispel is
// Remove Corruption / Nature's Cure (see DruidDispelSpell above).
class CastDruidRemoveCurseOnPartyAction : public CurePartyMemberAction
{
public:
    CastDruidRemoveCurseOnPartyAction(PlayerbotAI* ai);
    std::string const getName() override { return "remove curse on party"; }
};

// #384: Insect Swarm -> Sunfire (93402), see InsectSwarmTrigger in DruidTriggers.h. getName() keeps the AC name.
class CastInsectSwarmOnAttackerAction : public CastDebuffSpellOnAttackerAction
{
public:
    CastInsectSwarmOnAttackerAction(PlayerbotAI* ai) : CastDebuffSpellOnAttackerAction(ai, "sunfire") {}
    std::string const getName() override { return "insect swarm on attacker"; }
};

class CastMoonfireOnAttackerAction : public CastDebuffSpellOnAttackerAction
{
public:
    CastMoonfireOnAttackerAction(PlayerbotAI* ai) : CastDebuffSpellOnAttackerAction(ai, "moonfire") {}
};

class CastEnrageAction : public CastBuffSpellAction
{
public:
    CastEnrageAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "enrage") {}
};


class CastRejuvenationOnNotFullAction : public HealPartyMemberAction
{
public:
    CastRejuvenationOnNotFullAction(PlayerbotAI* ai)
        : HealPartyMemberAction(ai, "rejuvenation", 5.0f, HealingManaEfficiency::VERY_HIGH)
    {
    }
    bool isUseful() override;
    Unit* GetTarget() override;
};

class CastForceOfNatureAction : public CastSpellAction
{
public:
    CastForceOfNatureAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "force of nature") {}
};

// #363 (AC parity, Ai/Class/Druid/Action/DruidActions.h:169-175): Typhoon knocks up everything
// within 15 yards, so AC only fires it when facing and already in melee range.
class CastTyphoonAction : public CastSpellAction
{
public:
    CastTyphoonAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "typhoon") {}

    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
    bool isUseful() override;
};

// Base for blanket HoT actions (#363, AC parity, Ai/Class/Druid/Action/DruidActions.h:372-382).
// GetBlanketTarget() is a member so subclasses can use AI_VALUE and the standard context
// machinery, exactly as AC does.
class CastBlanketHotAction : public CastSpellAction
{
public:
    CastBlanketHotAction(PlayerbotAI* ai, std::string const& spell) : CastSpellAction(ai, spell)
    {
        range = botAI->GetRange("heal");
    }

protected:
    Unit* GetBlanketTarget(std::string const& auraName);
};

// #363 (AC parity, Ai/Class/Druid/Action/DruidActions.h:384-391). NOTE: the ACTION "rejuvenation
// blanket" shares its name with the unrelated TRIGGER of the same name (AC registers the trigger
// at DruidAiObjectContext.cpp:187 and the action at :288); getName() disambiguates the action.
class CastRejuvenationBlanketAction : public CastBlanketHotAction
{
public:
    CastRejuvenationBlanketAction(PlayerbotAI* ai) : CastBlanketHotAction(ai, "rejuvenation") {}

    bool isUseful() override;
    Unit* GetTarget() override;
    std::string const getName() override { return "rejuvenation blanket"; }
};

// #363 (AC parity, Ai/Class/Druid/Action/DruidActions.h:393-400). Same name collision as above
// with the "wild growth blanket" TRIGGER (AC :188 vs :289).
class CastWildGrowthBlanketAction : public CastBlanketHotAction
{
public:
    CastWildGrowthBlanketAction(PlayerbotAI* ai) : CastBlanketHotAction(ai, "wild growth") {}

    bool isUseful() override;
    Unit* GetTarget() override;
    std::string const getName() override { return "wild growth blanket"; }
};


// ---- Rotation lane 5 (#860, epic #854): MoP druid rotation ports. Every spell name resolves in the 5.4.8 Spell.dbc
// (spellname.py mop: 78674 Starsurge, 112071 Celestial Alignment, 124974 Nature's Vigil, 106922 Might of Ursoc, 62606
// Savage Defense, 102342 Ironbark, 106839 Skull Bash).
// MoP rotation row 10205 (spec 102, prio group 0/4): Starsurge.
class CastStarsurgeAction : public CastSpellAction
{
public:
    CastStarsurgeAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "starsurge") {}
};

// MoP rotation row 10213/10218 (spec 102, prio group 5/1): Celestial Alignment cooldown.
class CastCelestialAlignmentAction : public CastBuffSpellAction
{
public:
    CastCelestialAlignmentAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "celestial alignment") {}
};

// MoP rotation row 10214 (spec 102) / 10410 (spec 104), prio group 5/1: Nature's Vigil cooldown.
class CastNaturesVigilAction : public CastBuffSpellAction
{
public:
    CastNaturesVigilAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "nature's vigil") {}
};

// MoP rotation row 1100 (class row, all specs, prio group 5/0): Might of Ursoc at health <= 25%.
class CastMightOfUrsocAction : public CastBuffSpellAction
{
public:
    CastMightOfUrsocAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "might of ursoc") {}
};

// MoP rotation row 10417 (spec 104, prio group 5/1): Savage Defense at rage >= 80.
class CastSavageDefenseAction : public CastBuffSpellAction
{
public:
    CastSavageDefenseAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "savage defense") {}
};

// MoP rotation row 10505 (spec 105, prio group 1/13): Ironbark on the tank at health <= 40%.
class CastIronbarkOnMainTankAction : public BuffOnMainTankAction
{
public:
    CastIronbarkOnMainTankAction(PlayerbotAI* botAI) : BuffOnMainTankAction(botAI, "ironbark") {}
};

// MoP rotation row 26 (spec 103, prio group 10/2) + 10421 (spec 104): Skull Bash on a casting enemy. The
// shared "enemy caster target" value (lane 0, #855) scans every hostile in range, so the edge is limited to
// battlegrounds and arenas ("in pvp instance", rogue lane pattern); open-world cat/bear keep AC's current-target "bash".
class CastSkullBashOnEnemyCasterAction : public CastSpellOnEnemyCasterAction
{
public:
    CastSkullBashOnEnemyCasterAction(PlayerbotAI* botAI) : CastSpellOnEnemyCasterAction(botAI, "skull bash") {}

    bool isUseful() override;
};

#endif
