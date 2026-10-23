/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Shadowfang Keep (epic #373, issue #668). See CataSfkShared.h for sources and ids.

#include "CataSfkMultipliers.h"
#include "CataSfkActions.h"
#include "CataSfkShared.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Timer.h"

float SfkAvoidMultiplier::GetValue(Action* action)
{
    bool const isMove = dynamic_cast<MovementAction*>(action) != nullptr;
    CastSpellAction* const spellAction = isMove ? nullptr : dynamic_cast<CastSpellAction*>(action);
    if (!isMove && !spellAction)
        return 1.0f;

    // The ground-effect scan is a grid search: do it at most every 150 ms per bot, not once per candidate action
    uint32 const now = getMSTime();
    if (!_lastCheck || GetMSTimeDiffToNow(_lastCheck) >= 150)
    {
        _danger = CataSfk::InDanger(botAI, bot);
        _coagulant = CataSfk::CoagulantTicking(bot);
        _catalyst = CataSfk::CatalystUp(botAI, bot);
        _barrage = CataSfk::BarrageCasting(bot);
        _lastCheck = now;
    }

    if (spellAction)
    {
        // Godfrey's Pistol Barrage (93520) has no SpellCategories row (PreventionType 0), so a kick cannot stop it
        // (Spell::EffectInterruptCast needs SPELL_PREVENTION_TYPE_SILENCE, SpellEffects.cpp:3868), but the class interrupt
        // triggers fire on any cast (PlayerbotAI::IsInterruptableSpellCasting tests the bot's own spell, PlayerbotAI.cpp:
        // 3315-3341, as AC does) and spent kick / pummel / counterspell on it. Cursed Bullets (93629, PreventionType 3,
        // "can and should be interrupted", Wowpedia Lord Godfrey (tactics)) comes right after the barrage: keep them for it
        // (w65: 3 interrupts landed, 233 "sfk interrupt cursed bullets" FAILED; w67 the tank's pummel was on cooldown).
        if (_barrage && CataSfk::IsClassInterrupt(spellAction->getSpell()))
            return 0.0f;

        // Walden's Toxic Coagulant ticks on the bot: no cast that roots it (AC IngvarThePlundererMultiplier,
        // UKMultipliers.cpp:59-80 / fork UKMultipliers.cpp; channels counted as in AC NaxxMultipliers.cpp:83-89)
        if (!_coagulant)
            return 1.0f;

        uint32 const spellId = AI_VALUE2(uint32, "spell id", spellAction->getSpell());
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            return 1.0f;

        // 5.4.8: SpellInfo::CalcCastTime(uint8 level, Spell*) (SpellInfo.h:504), as in the fork's UKMultipliers.cpp
        return (spellInfo->CalcCastTime(bot->GetLevel()) != 0 || spellInfo->IsChanneled()) ? 0.0f : 1.0f;
    }

    // Walden's red Toxic Catalyst cauldron: stand still; only the Poisonous Mixture escape may move the bot (AC
    // IngvarThePlundererMultiplier "prevent movement actions", UKMultipliers.cpp:53-57)
    if (_catalyst)
        return (dynamic_cast<SfkLeavePoisonMixtureAction*>(action) || dynamic_cast<SfkCoagulantJumpAction*>(action))
                   ? 1.0f
                   : 0.0f;

    if (dynamic_cast<SfkAvoidAction*>(action))
        return 1.0f;

    return _danger ? 0.0f : 1.0f;
}
