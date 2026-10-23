/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GENERICWARLOCKNONCOMBATSTRATEGY_H
#define _PLAYERBOT_GENERICWARLOCKNONCOMBATSTRATEGY_H

#include "NonCombatStrategy.h"

class PlayerbotAI;

class GenericWarlockNonCombatStrategy : public NonCombatStrategy
{
public:
    GenericWarlockNonCombatStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "nc"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class WarlockPetStrategy : public Strategy
{
public:
    WarlockPetStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "pet"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class SummonImpStrategy : public NonCombatStrategy
{
public:
    SummonImpStrategy(PlayerbotAI* ai);
    virtual std::string const getName() override { return "bhealth"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class SummonFelguardStrategy : public NonCombatStrategy
{
public:
    SummonFelguardStrategy(PlayerbotAI* ai);
    virtual std::string const getName() override { return "bdps"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class SummonFelhunterStrategy : public NonCombatStrategy
{
public:
    SummonFelhunterStrategy(PlayerbotAI* ai);
    virtual std::string const getName() override { return "bmana"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

// #521 (AC parity, AC WarlockAiObjectContext.cpp:69-73): AC's pet-strategy keys "imp"/"felhunter"/
// "felguard". Same behaviour as bhealth/bmana/bdps above (both key sets stay registered); Engine::addStrategy
// needs getName() to equal the creator key, so each alias is its own class.
class SummonImpAcKeyStrategy : public SummonImpStrategy
{
public:
    SummonImpAcKeyStrategy(PlayerbotAI* ai) : SummonImpStrategy(ai) {}
    virtual std::string const getName() override { return "imp"; }
};

class SummonFelhunterAcKeyStrategy : public SummonFelhunterStrategy
{
public:
    SummonFelhunterAcKeyStrategy(PlayerbotAI* ai) : SummonFelhunterStrategy(ai) {}
    virtual std::string const getName() override { return "felhunter"; }
};

class SummonFelguardAcKeyStrategy : public SummonFelguardStrategy
{
public:
    SummonFelguardAcKeyStrategy(PlayerbotAI* ai) : SummonFelguardStrategy(ai) {}
    virtual std::string const getName() override { return "felguard"; }
};

// #163 S8 (AC parity, GenericWarlockNonCombatStrategy.h:73-111): opt-in strategies picking who a
// created soulstone goes to, mirroring the "boost"/"save mana" opt-in pattern used elsewhere.
class SoulstoneSelfStrategy : public NonCombatStrategy
{
public:
    SoulstoneSelfStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}
    std::string const getName() override { return "ss self"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class SoulstoneMasterStrategy : public NonCombatStrategy
{
public:
    SoulstoneMasterStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}
    std::string const getName() override { return "ss master"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class SoulstoneTankStrategy : public NonCombatStrategy
{
public:
    SoulstoneTankStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}
    std::string const getName() override { return "ss tank"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class SoulstoneHealerStrategy : public NonCombatStrategy
{
public:
    SoulstoneHealerStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}
    std::string const getName() override { return "ss healer"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

// -- #408 (AC parity, AC Strategy/GenericWarlockNonCombatStrategy.h:33-51). imp/felhunter/felguard
// already exist above under this module's spec-slot names ("bhealth"/"bmana"/"bdps"), so only the two
// strategies this module is missing are added here; their creator keys match their getName() exactly.
class SummonVoidwalkerStrategy : public NonCombatStrategy
{
public:
    SummonVoidwalkerStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}
    std::string const getName() override { return "voidwalker"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class SummonSuccubusStrategy : public NonCombatStrategy
{
public:
    SummonSuccubusStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}
    std::string const getName() override { return "succubus"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

// #408 (AC parity, AC Strategy/GenericWarlockNonCombatStrategy.h:113-131). Unlike AC these only
// dispatch the "use the stone" action - AC also chains "no firestone"/"no spellstone" -> "create
// firestone"/"create spellstone", and this module has no CastCreateFirestoneAction /
// CastCreateSpellstoneAction (see the create_soulstone note in WarlockAiObjectContext.cpp).
class UseSpellstoneStrategy : public NonCombatStrategy
{
public:
    UseSpellstoneStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}
    std::string const getName() override { return "spellstone"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class UseFirestoneStrategy : public NonCombatStrategy
{
public:
    UseFirestoneStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}
    std::string const getName() override { return "firestone"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
