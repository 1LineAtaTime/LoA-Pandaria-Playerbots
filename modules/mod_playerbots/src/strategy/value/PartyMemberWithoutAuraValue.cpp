#include "PartyMemberWithoutAuraValue.h"

#include "Playerbots.h"
#include "Helper.h"
#include "SpellMgr.h"

class PlayerWithoutAuraPredicate : public FindPlayerPredicate, public PlayerbotAIAware
{
public:
    PlayerWithoutAuraPredicate(PlayerbotAI* botAI, std::string const aura, bool skipPets = false)
        : PlayerbotAIAware(botAI), FindPlayerPredicate(), auras(split(aura, ',')), skipPets(skipPets)
    {
    }

public:
    bool Check(Unit* unit) override
    {
        if (!unit->IsAlive())
            return false;

        // #704 (MoP-only, extends AC Ai/Base/Value/PartyMemberValue.cpp:22-24, which also offers each member's pet):
        // a 5.4.8 consolidated raid buff never lands on a pet - the core's raid/party target check drops
        // IsPetGuardianStuff() targets of SPELL_ATTR7_CONSOLIDATED_RAID_BUFF spells (Spell.cpp:9351, 9372). The
        // pet therefore never gains the aura and the "on party" trigger recast the buff on it forever (owner
        // session 2026-10-06: 889 Legacy of the Emperor casts, 383 Arcane Brilliance). In WotLK these buffs do land
        // on pets, so AC needs no such rule.
        if (skipPets && unit->IsPetGuardianStuff())
            return false;

        for (std::vector<std::string>::iterator i = auras.begin(); i != auras.end(); ++i)
        {
            if (botAI->HasAura(*i, unit))
                return false;
        }

        return true;
    }

private:
    std::vector<std::string> auras;
    bool skipPets;
};

Unit* PartyMemberWithoutAuraValue::Calculate()
{
    // #704: the first qualifier name is the buff the trigger casts (BuffOnPartyTrigger passes its spell name).
    bool skipPets = false;
    std::vector<std::string> const names = split(qualifier, ',');
    if (!names.empty())
        if (uint32 const spellId = AI_VALUE2(uint32, "spell id", names.front()))
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId))
                skipPets = (spellInfo->AttributesEx7 & SPELL_ATTR7_CONSOLIDATED_RAID_BUFF) != 0;

    PlayerWithoutAuraPredicate predicate(botAI, qualifier, skipPets);
    return FindPartyMember(predicate);
}
