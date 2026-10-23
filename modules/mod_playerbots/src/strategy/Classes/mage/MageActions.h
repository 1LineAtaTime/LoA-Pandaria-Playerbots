/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MAGEACTIONS_H
#define _PLAYERBOT_MAGEACTIONS_H

#include "GenericSpellActions.h"
#include "SharedDefines.h"
#include "UseItemAction.h"

class PlayerbotAI;

BUFF_ACTION(CastFireWardAction, "fire ward");
BUFF_ACTION(CastFrostWardAction, "frost ward");

class CastFireballAction : public CastSpellAction
{
public:
    CastFireballAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "fireball") {}
};

class CastScorchAction : public CastSpellAction
{
public:
    CastScorchAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "scorch") {}
};

class CastFireBlastAction : public CastSpellAction
{
public:
    CastFireBlastAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "fire blast") {}
};

class CastArcaneBlastAction : public CastBuffSpellAction
{
public:
    CastArcaneBlastAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "arcane blast") {}
    std::string const GetTargetName() override { return "current target"; }
};

class CastSlowAction : public CastSpellAction
{
public:
    CastSlowAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "slow") {}
    bool isUseful() override;
};

class CastFrostjawAction : public CastSpellAction
{
public:
    CastFrostjawAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "frostjaw") {}
};

class CastArcaneBarrageAction : public CastSpellAction
{
public:
    CastArcaneBarrageAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "arcane barrage") {}
};

class CastArcaneMissilesAction : public CastSpellAction
{
public:
    CastArcaneMissilesAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "arcane missiles") {}
};

class CastArcaneExplosionAction : public CastSpellAction
{
public:
    CastArcaneExplosionAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "arcane explosion") {}
};

class CastPyroblastAction : public CastSpellAction
{
public:
    CastPyroblastAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "pyroblast") {}
};

class CastFlamestrikeAction : public CastDebuffSpellAction
{
public:
    CastFlamestrikeAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "flamestrike", true, 0.0f) {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};

class CastFrostNovaAction : public CastSpellAction
{
public:
    CastFrostNovaAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "frost nova") {}
    bool isUseful() override;
};

// Rotation lane 10 (#865): MoP talents with no AC counterpart, Temporal Shield (self, in combat) and
// Blazing Speed (self, in combat). Spell ids 115610 / 108843 resolve on 5.4.8 (Spell.dbc).
class CastTemporalShieldAction : public CastBuffSpellAction
{
public:
    CastTemporalShieldAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "temporal shield") {}
};

class CastBlazingSpeedAction : public CastBuffSpellAction
{
public:
    CastBlazingSpeedAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "blazing speed") {}
};

// Lane 0 contract base CastSpellOnEnemyCasterAction (GenericSpellActions.h): Counterspell on any interruptible enemy
// caster.
class CastCounterspellOnEnemyCasterAction : public CastSpellOnEnemyCasterAction
{
public:
    CastCounterspellOnEnemyCasterAction(PlayerbotAI* botAI) : CastSpellOnEnemyCasterAction(botAI, "counterspell") {}
};

class CastBlinkAction : public CastSpellAction
{
public:
    CastBlinkAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "blink") {}
    bool isUseful() override;
};

// #408 (AC parity, MageActions.h:139-142). AC's "blink back": Blink fired AWAY from the current
// target instead of MoP's CastBlinkAction, which blinks toward it. Triggered by AC's
// "enemy too close for spell and no firestarter strategy" node. MoP's CastBlinkAction is kept.
class CastBlinkBackAction : public CastSpellAction
{
public:
    CastBlinkBackAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "blink") {}
    bool Execute(Event event) override;
};

// #385 (AC MageActions.h:146-149 CastManaShieldAction). 5.4.8 DIVERGENCE: Mana Shield is not learnable by a
// 5.4.8 mage (cls.py mage -> NOT-LEARNABLE). This replaces the dead "mana shield" action with the 5.x
// Incanter's Ward talent (1463) for manual/strategy use only; no strategy edge uses it, because no source
// documents Incanter's Ward as Mana Shield's successor (Wowpedia patch notes, read 2026-10-04).
class CastIncantersWardAction : public CastBuffSpellAction
{
public:
    CastIncantersWardAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "incanter's ward") {}
};

// #385 (AC MageActions.h:161-164). Conjure Mana Gem, 759 (learnable L47 per cls.py).
class CastConjureManaGemAction : public CastBuffSpellAction
{
public:
    CastConjureManaGemAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "conjure mana gem") {}
};

// #385 (AC MageActions.h:180-218 UseMana{Sapphire,...}Action + MageActions.cpp:32-70 isUseful).
// AC's six WotLK gem actions collapse to one: 5.4.8 Conjure Mana Gem (759) creates only Mana Gem,
// item 36799 (Wowhead mop-classic tooltip item 36799, 2026-10-04). Action name = item name, the same
// way AC passes "mana sapphire", so UseItemAction resolves it through "inventory items".
class UseManaGemAction : public UseItemAction
{
public:
    UseManaGemAction(PlayerbotAI* botAI) : UseItemAction(botAI, "mana gem") {}
    bool isUseful() override;
};

class CastFrostboltAction : public CastSpellAction
{
public:
    CastFrostboltAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "frostbolt") {}
};

class CastFrostfireBoltAction : public CastSpellAction
{
public:
    CastFrostfireBoltAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "frostfire bolt") {}
};

class CastIceLanceAction : public CastSpellAction
{
public:
    CastIceLanceAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "ice lance") {}
};

class CastDeepFreezeAction : public CastSpellAction
{
public:
    CastDeepFreezeAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "deep freeze") {}
    bool isUseful() override;
};

class CastBlizzardAction : public CastSpellAction
{
public:
    CastBlizzardAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "blizzard") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
    // "NOT map type is arena".
    bool isUseful() override;
};

class CastConeOfColdAction : public CastSpellAction
{
public:
    CastConeOfColdAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "cone of cold") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
    bool isUseful() override;
};

class CastFrozenOrbAction : public CastCureSpellAction
{
public:
    CastFrozenOrbAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "frozen orb") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
    bool isUseful() override;
};

class CastArcaneBrillanceAction : public CastBuffSpellAction
{
public:
    CastArcaneBrillanceAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "arcane brilliance") {}
};

// 5.4.8: AC's "arcane intellect on party" ported onto MoP's "arcane brilliance" (see MageTriggers.h).
BUFF_PARTY_ACTION(CastArcaneBrillianceOnPartyAction, "arcane brilliance");

class CastRemoveCurseAction : public CastCureSpellAction
{
public:
    CastRemoveCurseAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "remove curse") {}
};

// 5.4.8: "Remove Lesser Curse" does not exist; curses are removed by the single "Remove Curse" spell on
// this core (MageAiObjectContext.cpp aliases the "remove lesser curse" action names to
// CastRemoveCurseAction/CastRemoveCurseOnPartyAction instead of these classes). Kept unregistered for
// reference in case AC's dual-curse-spell split needs revisiting.
class CastRemoveLesserCurseAction : public CastCureSpellAction
{
public:
    CastRemoveLesserCurseAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "remove lesser curse") {}
};

class CastIcyVeinsAction : public CastBuffSpellAction
{
public:
    CastIcyVeinsAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "icy veins") {}
};

class CastFingersOfFrostAction : public CastBuffSpellAction
{
public:
    CastFingersOfFrostAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "fingers of frost") {}
};

class CastColdSnapAction : public CastBuffSpellAction
{
public:
    CastColdSnapAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "cold snap") {}
};

class CastIceBarrierAction : public CastBuffSpellAction
{
public:
    CastIceBarrierAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "ice barrier") {}
};

class CastSummonWaterElementalAction : public CastBuffSpellAction
{
public:
    CastSummonWaterElementalAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "summon water elemental") {}
};

// 5.4.8: Combustion 11129 is an enemy-targeted nuke+stun ("Instantly deals $s2 Fire damage and stuns the target",
// Spell.dbc; core spell_mage.cpp:450-477 OnHit on the hit unit). AC's WotLK Combustion is a self buff, hence
// CastBuffSpellAction there (AC MageActions.h:72-76); self-targeted here it fails CheckCast result=13 BAD_TARGETS
// (w19 Castorin x11). Same shape as the other single-target mage spells.
class CastCombustionAction : public CastSpellAction
{
public:
    CastCombustionAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "combustion") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Single; }
};

BEGIN_SPELL_ACTION(CastCounterspellAction, "counterspell")
END_SPELL_ACTION()

class CastRemoveCurseOnPartyAction : public CurePartyMemberAction
{
public:
    CastRemoveCurseOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "remove curse", DISPEL_CURSE) {}
};

class CastRemoveLesserCurseOnPartyAction : public CurePartyMemberAction
{
public:
    CastRemoveLesserCurseOnPartyAction(PlayerbotAI* botAI)
        : CurePartyMemberAction(botAI, "remove lesser curse", DISPEL_CURSE)
    {
    }
};

// 5.4.8: AC's separate "conjure food"/"conjure water" (Cast{Food,Water}Action) merged into a single
// "Conjure Refreshment" in Cataclysm (one item makes food and drink together). Verified against
// Spell.dbc: "Conjure Food" (8736)/"Conjure Water" (29975/36879) are still present as rows but
// "Conjure Refreshment" is the one with a full level-bracket rank ladder (42955/42956/43988/74625/
// 92727/92739/92799/92802/92805/92822/92826, mirroring "Create Healthstone"'s 4-rank shape) - the
// old spells are leftover DBC data, not what a leveling mage is taught.
class CastConjureRefreshmentAction : public CastBuffSpellAction
{
public:
    CastConjureRefreshmentAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "conjure refreshment") {}
};

class CastIceBlockAction : public CastBuffSpellAction
{
public:
    CastIceBlockAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "ice block") {}
};

class CastMoltenArmorAction : public CastBuffSpellAction
{
public:
    CastMoltenArmorAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "molten armor") {}
};

class CastMageArmorAction : public CastBuffSpellAction
{
public:
    CastMageArmorAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "mage armor") {}
};

class CastIceArmorAction : public CastBuffSpellAction
{
public:
    CastIceArmorAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "ice armor") {}
};

class CastFrostArmorAction : public CastBuffSpellAction
{
public:
    CastFrostArmorAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "frost armor") {}
};

class CastPolymorphAction : public CastBuffSpellAction
{
public:
    CastPolymorphAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "polymorph") {}
    Value<Unit*>* GetTargetValue() override;
};

class CastSpellstealAction : public CastSpellAction
{
public:
    CastSpellstealAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "spellsteal") {}
};

class CastLivingBombAction : public CastDebuffSpellAction
{
public:
    CastLivingBombAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "living bomb", true) {}
};

class CastFrostBombAction : public CastDebuffSpellAction
{
public:
    CastFrostBombAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "frost bomb", true) {}
};

class CastNetherTempestAction : public CastDebuffSpellAction
{
public:
    CastNetherTempestAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "nether tempest", true) {}
};

class CastLivingBombOnAttackersAction : public CastDebuffSpellOnAttackerAction
{
public:
    CastLivingBombOnAttackersAction(PlayerbotAI* botAI) : CastDebuffSpellOnAttackerAction(botAI, "living bomb", true) {}
};

// #521: same shape as CastLivingBombOnAttackersAction for the other two tier-75 talents.
class CastFrostBombOnAttackersAction : public CastDebuffSpellOnAttackerAction
{
public:
    CastFrostBombOnAttackersAction(PlayerbotAI* botAI) : CastDebuffSpellOnAttackerAction(botAI, "frost bomb", true) {}
};

class CastNetherTempestOnAttackersAction : public CastDebuffSpellOnAttackerAction
{
public:
    CastNetherTempestOnAttackersAction(PlayerbotAI* botAI) : CastDebuffSpellOnAttackerAction(botAI, "nether tempest", true) {}
};

class CastDragonsBreathAction : public CastSpellAction
{
public:
    CastDragonsBreathAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "dragon's breath") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
    bool isUseful() override;
};

class CastInfernoBlastAction : public CastSpellAction
{
public:
    CastInfernoBlastAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "inferno blast") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Single; }
};

class CastBlastWaveAction : public CastSpellAction
{
public:
    CastBlastWaveAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "blast wave") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
    bool isUseful() override;
};

class CastInvisibilityAction : public CastBuffSpellAction
{
public:
    CastInvisibilityAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "invisibility") {}
};

class CastRuneOfPowerAction : public CastSpellAction
{
public:
    CastRuneOfPowerAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "rune of power") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
    bool Execute(Event event) override;
};

class CastEvocationAction : public CastSpellAction
{
public:
    CastEvocationAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "evocation") {}
    std::string const GetTargetName() override { return "self target"; }
};

class CastCounterspellOnEnemyHealerAction : public CastSpellOnEnemyHealerAction
{
public:
    CastCounterspellOnEnemyHealerAction(PlayerbotAI* botAI) : CastSpellOnEnemyHealerAction(botAI, "counterspell") {}
};

class CastArcanePowerAction : public CastBuffSpellAction
{
public:
    CastArcanePowerAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "arcane power") {}
};

class CastPresenceOfMindAction : public CastBuffSpellAction
{
public:
    CastPresenceOfMindAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "presence of mind") {}
};

class CastMirrorImageAction : public CastBuffSpellAction
{
public:
    CastMirrorImageAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "mirror image") {}
};

class CastFocusMagicOnPartyAction : public CastSpellAction
{
public:
    CastFocusMagicOnPartyAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "focus magic") {}
    Unit* GetTarget() override;
};

#endif
