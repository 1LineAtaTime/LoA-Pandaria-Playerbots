/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sh" (Scarlet Halls, epic #373, issue #657). See MopShShared.h for sources.

#ifndef PLAYERBOTS_MOPSHTRIGGERCONTEXT_H
#define PLAYERBOTS_MOPSHTRIGGERCONTEXT_H

#include "MopDungeonShared.h"
#include "MopShShared.h"
#include "MopShTriggers.h"
#include "NamedObjectContext.h"
#include "TriggerContext.h"

class MopDungeonScarletHallsTriggerContext : public NamedObjectContext<Trigger>
{
public:
    MopDungeonScarletHallsTriggerContext()
    {
        creators["sh knife line"] = &MopDungeonScarletHallsTriggerContext::sh_knife_line;
        creators["sh blades of light"] = &MopDungeonScarletHallsTriggerContext::sh_blades_of_light;
        creators["sh koegler interrupt"] = &MopDungeonScarletHallsTriggerContext::sh_koegler_interrupt;
        creators["sh burning books"] = &MopDungeonScarletHallsTriggerContext::sh_burning_books;
        creators["sh book burner intercept"] = &MopDungeonScarletHallsTriggerContext::sh_book_burner_intercept;
        creators["sh dragon breath"] = &MopDungeonScarletHallsTriggerContext::sh_dragon_breath;
        creators["sh hounds"] = &MopDungeonScarletHallsTriggerContext::sh_hounds;
        creators["sh tank hounds"] = &MopDungeonScarletHallsTriggerContext::sh_tank_hounds;
        creators["sh defenders"] = &MopDungeonScarletHallsTriggerContext::sh_defenders;
        creators["sh tank defenders"] = &MopDungeonScarletHallsTriggerContext::sh_tank_defenders;
    }

private:
    static Trigger* sh_knife_line(PlayerbotAI* botAI) { return new MopShKnifeLineTrigger(botAI); }
    static Trigger* sh_blades_of_light(PlayerbotAI* botAI) { return new MopShBladesOfLightTrigger(botAI); }
    static Trigger* sh_koegler_interrupt(PlayerbotAI* botAI) { return new MopShKoeglerInterruptTrigger(botAI); }
    static Trigger* sh_burning_books(PlayerbotAI* botAI) { return new MopShBurningBooksTrigger(botAI); }
    static Trigger* sh_book_burner_intercept(PlayerbotAI* botAI) { return new MopShBookBurnerInterceptTrigger(botAI); }
    static Trigger* sh_dragon_breath(PlayerbotAI* botAI) { return new MopShDragonBreathTrigger(botAI); }

    static Trigger* sh_hounds(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveTrigger(botAI, "sh hounds", MopScarletHalls::HoundWave(MopDungeon::WaveRole::Dps));
    }

    static Trigger* sh_tank_hounds(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveTrigger(botAI, "sh tank hounds",
                                              MopScarletHalls::HoundWave(MopDungeon::WaveRole::Tank));
    }

    static Trigger* sh_defenders(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveTrigger(botAI, "sh defenders",
                                              MopScarletHalls::DefenderWave(MopDungeon::WaveRole::Dps));
    }

    static Trigger* sh_tank_defenders(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveTrigger(botAI, "sh tank defenders",
                                              MopScarletHalls::DefenderWave(MopDungeon::WaveRole::Tank));
    }
};

#endif
