/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Actions/CheckMountStateAction) into the
 * Legends of Azeroth Pandaria fork. The upstream version follows a master's mount state, uses the
 * playerbots_preferred_mounts table, battleground flag rules and druid travel forms; this fork keeps the
 * self-contained part (mount for a real journey, dismount when riding no longer makes sense) which is what
 * random bots roaming the open world need (issue #62).
 */

#ifndef _PLAYERBOT_CHECKMOUNTSTATEACTION_H
#define _PLAYERBOT_CHECKMOUNTSTATEACTION_H

#include "UseItemAction.h"  // #530: AC CheckMountStateAction derives from UseItemAction (AC .h:11/:30)
#include "Unit.h"  // ShapeshiftForm / FORM_* for the captured shapeshift members

class PlayerbotAI;

class CheckMountStateAction : public UseItemAction
{
public:
    CheckMountStateAction(PlayerbotAI* botAI) : UseItemAction(botAI, "check mount state") {}

    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override { return true; }

    /// #871 (AC CheckMountStateAction.h:45 / .cpp:372-390): after a server-forced dismount (SMSG_DISMOUNT for
    /// this bot, e.g. a no-fly/no-mount area) land the bot: MoveFall + HandleFall from the ground Z, then clear
    /// the falling flags. Called from PlayerbotAI::HandleBotOutgoingPacket (AC Bot/PlayerbotAI.cpp:1549-1558).
    static void CompleteDismount(Player* bot);

private:
    /// Best mount spell in the bot's spellbook for the bot's current riding skill and location (0 if
    /// none). #179 (AC ShouldFollowMasterMountState/GetMountType/CalculateMasterMountSpeed): when the
    /// bot has a real master outside a battleground, `preferFlying` says whether the *master*'s mount
    /// is a flyer (MasterWantsFlying()) - still gated by the bot's own ability to fly here
    /// (BotCanUseFlyingMount) so a grounded bot never tries an impossible cast; with no real master,
    /// callers pass BotCanUseFlyingMount(bot) directly, unchanged from before #179. #394: preferFlying
    /// is false in a battleground (AC :648), so no caller can hand this function a flyer inside a BG.
    uint32 FindMountSpell(bool preferFlying) const;

    /// Upstream's thresholds (Ai/Base/Actions/CheckMountStateAction.cpp). Mounting is only worth it
    /// beyond ~21 yd: below that the mount cast time costs more than the speed it saves.
    float CalculateMountDistance() const;
    float CalculateDismountDistance() const;

    /// #179 (AC CheckMountStateAction.cpp, ported): match the master's mount state instead of the
    /// random-bot travel-distance heuristic, while a real master exists and the bot is not in a
    /// battleground (AC ShouldFollowMasterMountState/ShouldDismountForMaster/GetMountType - the
    /// battleground no-flyer clamp is no longer trimmed: #394 restored AC's `!bot->InBattleground()`
    /// (AC :648) on the no-real-master mount path, so a bot in a BG never selects a flying mount.
    /// Still left out: AC's WotLK Flight Form/Swift
    /// Flight Form shapeshift matching and its playerbots_preferred_mounts DB table - see the .cpp
    /// file comment. AC's own BG ground-speed cap is not portable and is not ported: it only ever
    /// clamps CalculateMasterMountSpeed()'s local maxGround, which is 59 or 99 and therefore never
    /// greater than the 99 it is compared against, and this fork has no master-speed value to clamp).
    bool ShouldFollowMasterMountState(Player* master, bool noAttackers, bool shouldMount) const;
    bool ShouldDismountForMaster(Player* master) const;
    /// Whether the master's current mount (if any) is a flying mount, inspected the same way
    /// FindMountSpell() ranks the bot's own mounts: a mount spell's single SPELL_AURA_MOUNTED effect
    /// only names a MountType; flight comes from the MountCapability the core resolves for the rider
    /// (here, the master) via its SpeedModSpell.
    bool MasterWantsFlying(Player* master) const;

    /// AC CheckMountStateAction::Mount() (AC :290-344), adapted: drop a non-travel shapeshift, try the
    /// druid forms (TryForms), then the mount spell (FindMountSpell). `preferFlying` / `travelDist` /
    /// `status` are what the two Execute() paths already computed. AC's preferred-mount table and mount
    /// ITEM fallback are not ported (see the .cpp comment in Mount()).
    bool Mount(bool preferFlying, float travelDist, std::string const& status);
    /// AC CheckMountStateAction::TryForms() (AC :395-443), travel form only (5.4.8 has one Travel Form).
    bool TryForms(float travelDist, std::string const& status);

    Player* master = nullptr;
    // AC CheckMountStateAction.h: shapeshift forms captured in isUseful() for Execute()/Mount().
    ShapeshiftForm botInShapeshiftForm = FORM_NONE;
    ShapeshiftForm masterInShapeshiftForm = FORM_NONE;
};

#endif
