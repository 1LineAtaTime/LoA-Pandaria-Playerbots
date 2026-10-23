#include "RacialsStrategy.h"

#include "Playerbots.h"

namespace
{
    // 5.4.8 ids, verified against the client Spell.dbc (build 18414). AC only lists the first three
    // (paladin/priest/mage/warlock mana, rogue energy, DK runic power); the rest are Cata/MoP
    // additions with no AC entry (research doc B.8).
    constexpr uint32 SPELL_ARCANE_TORRENT_MANA = 28730;
    constexpr uint32 SPELL_ARCANE_TORRENT_ENERGY = 25046;
    constexpr uint32 SPELL_ARCANE_TORRENT_RUNIC_POWER = 50613;
    constexpr uint32 SPELL_ARCANE_TORRENT_RAGE = 69179;
    constexpr uint32 SPELL_ARCANE_TORRENT_FOCUS = 80483;
    constexpr uint32 SPELL_ARCANE_TORRENT_CHI = 129597;
    constexpr uint32 SPELL_WAR_STOMP = 20549;
    constexpr uint32 SPELL_BERSERKING = 26297;
    constexpr uint32 SPELL_EVERY_MAN_FOR_HIMSELF = 59752;
    constexpr uint32 SPELL_WILL_OF_THE_FORSAKEN = 7744;
    constexpr uint32 SPELL_STONEFORM = 20594;
    constexpr uint32 SPELL_ESCAPE_ARTIST = 20589;
}

void RacialsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Player* bot = botAI->GetBot();
    AiObjectContext* context = botAI->GetAiObjectContext();

    if (bot->HasSpell(SPELL_ARCANE_TORRENT_MANA))
        triggers.push_back(
            new TriggerNode("low mana", NextAction::array(0, new NextAction("arcane torrent", ACTION_NORMAL + 5), nullptr)));

    if (bot->HasSpell(SPELL_ARCANE_TORRENT_ENERGY))
        triggers.push_back(
            new TriggerNode("low energy", NextAction::array(0, new NextAction("arcane torrent", ACTION_NORMAL + 5), nullptr)));

    if (bot->HasSpell(SPELL_ARCANE_TORRENT_RUNIC_POWER))
        triggers.push_back(new TriggerNode(
            "generic boost", NextAction::array(0, new NextAction("arcane torrent", ACTION_NORMAL + 5), nullptr)));

    // 5.4.8-only variants (no low-rage/low-focus/low-Chi trigger on this fork; "generic boost" is the
    // same fallback AC itself uses for the runic-power case).
    if (bot->HasSpell(SPELL_ARCANE_TORRENT_RAGE) || bot->HasSpell(SPELL_ARCANE_TORRENT_FOCUS) ||
        bot->HasSpell(SPELL_ARCANE_TORRENT_CHI))
        triggers.push_back(new TriggerNode(
            "generic boost", NextAction::array(0, new NextAction("arcane torrent", ACTION_NORMAL + 5), nullptr)));

    if (bot->HasSpell(SPELL_WAR_STOMP))
        triggers.push_back(
            new TriggerNode("medium aoe", NextAction::array(0, new NextAction("war stomp", ACTION_NORMAL + 5), nullptr)));

    if (bot->HasSpell(SPELL_BERSERKING))
        triggers.push_back(
            new TriggerNode("generic boost", NextAction::array(0, new NextAction("berserking", ACTION_NORMAL + 5), nullptr)));

    if (bot->HasSpell(SPELL_EVERY_MAN_FOR_HIMSELF))
        triggers.push_back(new TriggerNode(
            "loss of control", NextAction::array(0, new NextAction("every man for himself", ACTION_EMERGENCY + 1), nullptr)));

    if (bot->HasSpell(SPELL_WILL_OF_THE_FORSAKEN))
        triggers.push_back(new TriggerNode(
            "fear charm sleep", NextAction::array(0, new NextAction("will of the forsaken", ACTION_EMERGENCY + 1), nullptr)));

    if (bot->HasSpell(SPELL_STONEFORM))
        triggers.push_back(new TriggerNode(
            "poison disease bleed", NextAction::array(0, new NextAction("stoneform", ACTION_DISPEL), nullptr)));

    if (bot->HasSpell(SPELL_ESCAPE_ARTIST))
        triggers.push_back(new TriggerNode(
            "movement impaired", NextAction::array(0, new NextAction("escape artist", ACTION_EMERGENCY + 1), nullptr)));

    // These three are resolved by name, not a hardcoded id (AC does the same): only the race/class
    // that actually knows the spell resolves a nonzero id here, so this doubles as the HasSpell gate.
    // (PlayerbotAI has no string-keyed HasSpell(); "spell id" (ValueContext.h) is this fork's existing
    // by-name spell lookup, already used throughout the module.)
    if (AI_VALUE2(uint32, "spell id", "blood fury"))
        triggers.push_back(
            new TriggerNode("generic boost", NextAction::array(0, new NextAction("blood fury", ACTION_NORMAL + 5), nullptr)));

    if (AI_VALUE2(uint32, "spell id", "gift of the naaru"))
        triggers.push_back(new TriggerNode(
            "medium health", NextAction::array(0, new NextAction("gift of the naaru", ACTION_LIGHT_HEAL + 5), nullptr)));

    if (AI_VALUE2(uint32, "spell id", "lifeblood"))
        triggers.push_back(new TriggerNode(
            "medium health", NextAction::array(0, new NextAction("lifeblood", ACTION_LIGHT_HEAL + 5), nullptr)));

    triggers.push_back(
        new TriggerNode("generic boost", NextAction::array(0, new NextAction("use trinket", ACTION_NORMAL + 4), nullptr)));
}
