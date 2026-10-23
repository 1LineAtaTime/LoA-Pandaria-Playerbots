/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Deadmines (epic #373, issue #667). See CataDmShared.h for sources and ids.

#ifndef PLAYERBOTS_CATADMTRIGGERS_H
#define PLAYERBOTS_CATADMTRIGGERS_H

#include "Trigger.h"

class DmBlossomLandingTrigger : public Trigger
{
public:
    DmBlossomLandingTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dm blossom landing") {}
    bool IsActive() override;
};

class DmBlossomAddTrigger : public Trigger
{
public:
    DmBlossomAddTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dm blossom add") {}
    bool IsActive() override;
};

class DmStickyBombTrigger : public Trigger
{
public:
    DmStickyBombTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dm sticky bomb") {}
    bool IsActive() override;
};

class DmChestBombTrigger : public Trigger
{
public:
    DmChestBombTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dm chest bomb") {}
    bool IsActive() override;
};

class DmHelixFocusTrigger : public Trigger
{
public:
    DmHelixFocusTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dm helix focus") {}
    bool IsActive() override;
};

class DmHarvestTrigger : public Trigger
{
public:
    DmHarvestTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dm harvest") {}
    bool IsActive() override;
};

class DmOverdriveTrigger : public Trigger
{
public:
    DmOverdriveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dm overdrive") {}
    bool IsActive() override;
};

class DmVaporTrigger : public Trigger
{
public:
    DmVaporTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dm vapor") {}
    bool IsActive() override;
};

class DmVanessaAddTrigger : public Trigger
{
public:
    DmVanessaAddTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dm vanessa add") {}
    bool IsActive() override;
};



// ---- #681 / #826 (see CataDmShared.h) ----
class DmFireWallTrigger : public Trigger
{
public:
    DmFireWallTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dm fire wall") {}
    bool IsActive() override;
};

class DmMoltenSlagTrigger : public Trigger
{
public:
    DmMoltenSlagTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dm molten slag") {}
    bool IsActive() override;
};

class DmSlagAddTrigger : public Trigger
{
public:
    DmSlagAddTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dm slag add") {}
    bool IsActive() override;
};

class DmReaperFocusTrigger : public Trigger
{
public:
    DmReaperFocusTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dm reaper focus") {}
    bool IsActive() override;
};

class DmVanessaBlazeTrigger : public Trigger
{
public:
    DmVanessaBlazeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dm vanessa blaze") {}
    bool IsActive() override;
};

class DmPowderKegTrigger : public Trigger
{
public:
    DmPowderKegTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dm powder keg") {}
    bool IsActive() override;
};

class DmSteamValveTrigger : public Trigger
{
public:
    DmSteamValveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dm steam valve") {}
    bool IsActive() override;
};

class DmNightmareHazardTrigger : public Trigger
{
public:
    DmNightmareHazardTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dm nightmare hazard") {}
    bool IsActive() override;
};

class DmHarringtonTrigger : public Trigger
{
public:
    DmHarringtonTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dm harrington") {}
    bool IsActive() override;
};

#endif
