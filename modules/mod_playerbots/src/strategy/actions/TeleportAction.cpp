/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/TeleportAction.cpp, same order as AC (spell-caster/goober game
 * objects that teleport, then the last area trigger). The commented-out portal-entry list of AC is dropped.
 * 5.4.8 adaptations: CMSG_AREATRIGGER carries the two extra bits (fromClient, entered), as in AreaTriggerAction.cpp;
 * the spell is cast by prepare() alone, which already casts a spell without a cast time on this core (the extra
 * cast(true) after prepare in AC is dropped).
 */

#include "TeleportAction.h"

#include "AiObjectContext.h"
#include "Event.h"
#include "GameObject.h"
#include "LastMovementValue.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "WorldSession.h"

bool TeleportAction::Execute(Event /*event*/)
{
    // Spellcaster-type game objects that teleport the user.
    GuidVector gos = *context->GetValue<GuidVector>("nearest game objects");
    for (ObjectGuid const guid : gos)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go)
            continue;

        GameObjectTemplate const* goInfo = go->GetGOInfo();
        if (goInfo->type != GAMEOBJECT_TYPE_SPELLCASTER && goInfo->type != GAMEOBJECT_TYPE_GOOBER)
            continue;

        uint32 spellId = goInfo->spellcaster.spellId;
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo || !spellInfo->HasEffect(SPELL_EFFECT_TELEPORT_UNITS))
            continue;

        std::ostringstream out;
        out << "Teleporting using " << goInfo->name;
        botAI->TellMasterNoFacing(out.str());

        botAI->ChangeStrategy("-follow,+stay", BOT_STATE_NON_COMBAT);

        Spell* spell = new Spell(bot, spellInfo, TRIGGERED_NONE);
        SpellCastTargets targets;
        targets.SetUnitTarget(bot);
        spell->prepare(&targets);
        return true;
    }

    // If no game objects were found, try using the last area trigger.
    LastMovement& movement = context->GetValue<LastMovement&>("last area trigger")->Get();
    if (movement.lastAreaTrigger)
    {
        WorldPacket p(CMSG_AREATRIGGER);
        p << uint32(movement.lastAreaTrigger);
        p.WriteBit(0);  // fromClient
        p.WriteBit(1);  // entered
        p.FlushBits();
        p.rpos(0);

        bot->GetSession()->HandleAreaTriggerOpcode(p);
        movement.lastAreaTrigger = 0;
        return true;
    }

    // If no teleport option is found.
    botAI->TellError("Cannot find any portal to teleport");
    return false;
}
