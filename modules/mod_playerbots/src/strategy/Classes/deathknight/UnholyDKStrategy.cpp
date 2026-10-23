#/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "UnholyDKStrategy.h"

#include "Playerbots.h"

class UnholyDKStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    UnholyDKStrategyActionNodeFactory()
    {
        // Unholy
        // creators["bone shield"] = &bone_shield;
        // creators["plague strike"] = &plague_strike;
        // creators["death grip"] = &death_grip;
        // creators["death coil"] = &death_coil;
        creators["death strike"] = &death_strike;
        // creators["unholy blight"] = &unholy_blight;
        creators["scourge strike"] = &scourge_strike;
        // creators["death and decay"] = &death_and_decay;
        // creators["unholy pressence"] = &unholy_pressence;
        // creators["raise dead"] = &raise_dead;
        // creators["army of the dead"] = &army of the dead;
        // creators["summon gargoyle"] = &army of the dead;
        // creators["anti magic shell"] = &anti_magic_shell;
        // creators["anti magic zone"] = &anti_magic_zone;
        creators["ghoul frenzy"] = &ghoul_frenzy;
        creators["corpse explosion"] = &corpse_explosion;
        creators["icy touch"] = &icy_touch;
    }

private:
    // #601 MoP-only divergence (AC UnholyDKStrategy.cpp:27,36,45,55,64 use "blood presence"): in 5.4.8 Unholy dps
    // uses Unholy Presence 48265 (+10% haste, +15% movement speed; Wowhead mop-classic tooltips 48263/48266/48265
    // and Wowpedia "Presence", read 2026-10-05), so every Unholy prerequisite below is "unholy presence".
    static ActionNode* death_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("death strike",
                              /*P*/ NextAction::array(0, new NextAction("unholy presence"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
    static ActionNode* ghoul_frenzy([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("ghoul frenzy",
                              /*P*/ NextAction::array(0, new NextAction("unholy presence"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
    static ActionNode* corpse_explosion([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("corpse explosion",
                              /*P*/ NextAction::array(0, new NextAction("unholy presence"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* scourge_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("scourge strike",
                              /*P*/ NextAction::array(0, new NextAction("unholy presence"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
    static ActionNode* icy_touch([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("icy touch",
                              /*P*/ NextAction::array(0, new NextAction("unholy presence"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
};

UnholyDKStrategy::UnholyDKStrategy(PlayerbotAI* botAI) : GenericDKStrategy(botAI)
{
    actionNodeFactories.Add(new UnholyDKStrategyActionNodeFactory());
}

NextAction** UnholyDKStrategy::getDefaultActions()
{
    // #601 (AC UnholyDKStrategy.cpp:76-85): identical to AC, including "death and decay" at ACTION_HIGH + 5 (AC
    // has it as a default too, plus the "medium aoe" edge in UnholyDKAoeStrategy). It falls through on cooldown
    // (why=cooldown in the wave-10 trail), so it does not starve the edges below it.
    return NextAction::array(
        0, new NextAction("death and decay", ACTION_HIGH + 5),
        // rot-lane-2 (#857): MoP rotation row 25201 puts Summon Gargoyle at prio 19 (right below Death Grip), i.e. cast on
        // cooldown, ahead of the whole disease / strike rotation; AC has it at DEFAULT+0.4.
        new NextAction("summon gargoyle", ACTION_DEFAULT + 1.0f),
        // new NextAction("empower rune weapon", ACTION_DEFAULT + 0.3f),
        new NextAction("horn of winter", ACTION_DEFAULT + 0.2f),
        new NextAction("death coil", ACTION_DEFAULT + 0.1f),
        new NextAction("melee", ACTION_DEFAULT), nullptr);
}

void UnholyDKStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDKStrategy::InitTriggers(triggers);

    // #128 (AC UnholyDKStrategy.cpp:92,94): only Unholy keeps a permanent ghoul in 5.4.8 (Master of Ghouls
    // 52143, Wowhead mop-classic tooltip, read 2026-10-04), so only Unholy raises on "no pet".
    triggers.push_back(
        new TriggerNode("no pet", NextAction::array(0, new NextAction("raise dead", ACTION_NORMAL + 5), nullptr)));
    triggers.push_back(
        new TriggerNode("has pet", NextAction::array(0, new NextAction("toggle pet spell", 60.0f), nullptr)));

    // #163 S8 / #147 (AC parity, UnholyDKStrategy.cpp:96): apply the configured stance to a freshly
    // raised ghoul.
    triggers.push_back(
        new TriggerNode("new pet", NextAction::array(0, new NextAction("set pet stance", 60.0f), nullptr)));

    triggers.push_back(new TriggerNode(
        "death and decay cooldown", NextAction::array(0,
            new NextAction("ghoul frenzy", ACTION_DEFAULT + 0.9f),
            new NextAction("scourge strike", ACTION_DEFAULT + 0.8f),
            new NextAction("icy touch", ACTION_DEFAULT + 0.7f),
            // rot-lane-2 (#857): MoP rotation row 25212 Festering Strike (prio 7, "always") and 25213 Soul Reaper (prio 7);
            // both sit below Scourge Strike (25211, prio 8) in MoP 5.4.8, AC's icy touch / blood strike / plague
            // strike fillers are kept around them.
            // w75: they were BELOW AC's unconditional Icy Touch (.7), which took the Frost rune Festering Strike (Blood +
            // Frost) needs: 6 festering / 6 soul reaper attempts all ended SPELL_FAILED_NO_POWER (88), 0 casts. MoP 5.4.8 only
            // casts Icy Touch with Frost Fever missing (25205, covered by Outbreak + "dd cd and icy touch 3s"), so both now
            // sit between Scourge Strike (.8) and Icy Touch (.7), Festering first.
            new NextAction("festering strike", ACTION_DEFAULT + 0.75f),
            new NextAction("soul reaper", ACTION_DEFAULT + 0.74f),
            new NextAction("blood strike", ACTION_DEFAULT + 0.6f),
            new NextAction("plague strike", ACTION_DEFAULT + 0.5f),
            nullptr)));

    // rot-fix-F (wave 74): was DEFAULT+0.75, above Festering Strike (.65) / Soul Reaper (.64): Blood Strike took the Blood
    // rune both of them need, so neither was ever cast (Festering 85948 / Soul Reaper 130736: 17 rejects, 0 casts). MoP
    // (deathknight-unholy.md) has no Blood Strike at all, so it only fills what the 5.x spenders leave.
    triggers.push_back(new TriggerNode("dd cd and no desolation",
                                       NextAction::array(0, new NextAction("blood strike", ACTION_DEFAULT + 0.35f), nullptr)));

    // triggers.push_back(
    //     new TriggerNode("icy touch", NextAction::array(0, new NextAction("icy touch", ACTION_HIGH + 2), nullptr)));
    // triggers.push_back(new TriggerNode(
    //     "plague strike", NextAction::array(0, new NextAction("plague strike", ACTION_HIGH + 1), nullptr)));

    triggers.push_back(new TriggerNode(
        // rot-fix-F: the three rune-dump edges below were ACTION_NORMAL+3/+2/+1 (23/22/21), above every DEFAULT-band
        // 5.x spender, so Icy Touch / Blood Strike / Plague Strike ate the Frost / Blood / Unholy runes before Scourge Strike
        // (25211), Festering Strike (25212) and Soul Reaper (25213) could use them. MoP 5.4.8 only casts Icy Touch / Plague
        // Strike when the disease is missing (25205/25206, covered by Outbreak + the disease triggers), so the dumps drop to
        // DEFAULT+0.37/0.36/0.355 (below Soul Reaper .64, above Death Coil / Horn): they still dump surplus runes.
        "high frost rune", NextAction::array(0, 
        new NextAction("icy touch", ACTION_DEFAULT + 0.37f), nullptr)));

    triggers.push_back(new TriggerNode(
        "high blood rune", NextAction::array(0, new NextAction("blood strike", ACTION_DEFAULT + 0.36f), nullptr)));

    triggers.push_back(new TriggerNode(
        "high unholy rune", NextAction::array(0,
            new NextAction("plague strike", ACTION_DEFAULT + 0.355f), nullptr)));

    triggers.push_back(
        new TriggerNode("dd cd and plague strike 3s", NextAction::array(0, new NextAction("plague strike", ACTION_HIGH + 1), nullptr)));

    triggers.push_back(
        new TriggerNode("dd cd and icy touch 3s", NextAction::array(0, new NextAction("icy touch", ACTION_HIGH + 2), nullptr)));

    // "no rune -> empower rune weapon" now comes from GenericDKStrategy (#523, AC GenericDKStrategy.cpp:77).

    // rot-lane-2 (#857): MoP Unholy priority list, numbers keep the order
    // Outbreak(12) > Icy Touch / Plague Strike(11) > Festering Strike on <= 2 s disease(10) > Death Coil at
    // >= 90 RP or Sudden Doom(9) > Scourge Strike(8) > Festering / Soul Reaper(7), with the AC edges above/below.
    // 25203/25204 Outbreak when Blood Plague or Frost Fever is missing (HIGH+3 > icy touch HIGH+2 > plague HIGH+1).
    triggers.push_back(
        new TriggerNode("dk outbreak", NextAction::array(0, new NextAction("outbreak", ACTION_HIGH + 3), nullptr)));
    // 25207/25208 Festering Strike when a disease has <= 2 s left.
    triggers.push_back(new TriggerNode(
        "dk festering", NextAction::array(0, new NextAction("festering strike", ACTION_HIGH + 0.5f), nullptr)));
    // 25209/25210 Death Coil at runic power >= 90 or with Sudden Doom (above Scourge Strike at DEFAULT+0.8).
    triggers.push_back(new TriggerNode(
        "dk death coil proc", NextAction::array(0, new NextAction("death coil", ACTION_DEFAULT + 0.85f), nullptr)));
    // 25219 Unholy Frenzy (self, group 5 cooldown).
    triggers.push_back(new TriggerNode(
        "unholy frenzy", NextAction::array(0, new NextAction("unholy frenzy", ACTION_HIGH + 4), nullptr)));

    // triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction(, ACTION_NORMAL + 2), nullptr)));
    triggers.push_back(new TriggerNode(
        "army of the dead", NextAction::array(0, new NextAction("army of the dead", ACTION_HIGH + 6), nullptr)));
    // #601 (AC UnholyDKStrategy.cpp:165-171 "bone shield" -> HIGH+3): not ported here - Bone Shield 49222 is
    // Blood-spec only in 5.4.8 (cls.py dk: spec250 L78), so Unholy could never cast it (IMPOSSIBLE x60 in wave
    // 10). The edge now lives in BloodDKStrategy at the same number.
}

void UnholyDKAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // #523 (AC UnholyDKStrategy.cpp:174-193): AC's "loot available -> corpse explosion" and the corpse
    // explosion half of "medium aoe" are not ported - Corpse Explosion is not learnable in 5.4.8 (Spell.dbc
    // only, no DK skill-line / talent / spec row; cls.py dk), so the edge could never cast.
    triggers.push_back(new TriggerNode(
        "medium aoe", NextAction::array(0, new NextAction("death and decay", ACTION_NORMAL + 3), nullptr)));
}
