/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/LootAction.cpp (issue #151). 5.4.8 adaptations are
 * called out at each site below. Config keys AiPlayerbot.FreeMethodLoot and
 * AiPlayerbot.DisallowedGameObjects are read locally (not added to PlayerbotAIConfig.h) to avoid the
 * full-module rebuild that header change would force - see loot-stack-port.md section 2 step 0.
 */

#include "LootAction.h"

#include "BroadcastHelper.h"
#include "Config.h"
#include "Event.h"
#include "GameObject.h"
#include "LootMgr.h"
#include "LootStrategyValue.h"
#include "LootObjectStack.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "WorldSession.h"

#include <sstream>

namespace
{
    // AiPlayerbot.FreeMethodLoot / AiPlayerbot.DisallowedGameObjects (AC PlayerbotAIConfig.h/.cpp)
    // are not present on this fork's PlayerbotAIConfig (loot-stack-port.md section 3). Read once,
    // locally, with the identical AC keys and defaults.
    // TODO move to PlayerbotAIConfig with the next header build.
    bool FreeMethodLoot()
    {
        static bool const v = sConfigMgr->GetBoolDefault("AiPlayerbot.FreeMethodLoot", false);
        return v;
    }

    std::set<uint32> const& DisallowedGameObjects()
    {
        static std::set<uint32> const v = []
        {
            std::set<uint32> ids;
            std::string const raw = sConfigMgr->GetStringDefault("AiPlayerbot.DisallowedGameObjects",
                "176213,17155,2656,74448,19020,3719,3658,3705,3706,105579,75293,2857,179490,141596,"
                "160836,160845,179516,176224,181085,176112,128308,128403,165739,165738,175245,175970,"
                "176325,176327,123329,2560");
            std::stringstream ss(raw);
            std::string token;
            while (std::getline(ss, token, ','))
                if (!token.empty())
                    ids.insert(uint32(std::stoul(token)));
            return ids;
        }();
        return v;
    }

    // #958: the bot's stand-in for AC's queued CMSG_LOOT_RELEASE. AC's HandleLootReleaseOpcode
    // (AC Handlers/LootHandler.cpp:322-324) only releases a guid that is still the open loot, so a
    // container item the core already released and destroyed (autostore of its last item or its
    // money: core Handlers/LootHandler.cpp:147-148, :305-312) is a no-op there. This core's
    // DoLootRelease does not re-check, and a second call logs "RemoveLootedObject ... not found in
    // view"; release only a source that is still one of the bot's open loot views.
    void ReleaseIfStillOpen(Player* bot, ObjectGuid srcGuid)
    {
        for (auto const& view : bot->GetLootView())
        {
            if (view.second == srcGuid)
            {
                bot->GetSession()->DoLootRelease(srcGuid);
                return;
            }
        }
    }

    // AC PlayerbotAI::HasSkill(SkillType) (Bot/PlayerbotAI.cpp:5427) has no equivalent on this
    // fork's PlayerbotAI - see LootObjectStack.cpp for the same local substitute.
    bool BotHasSkill(Player* bot, uint32 skillId)
    {
        return skillId != SKILL_NONE && bot->HasSkill(skillId) && bot->GetSkillValue(skillId) > 0;
    }

    // AC's raw creature-gathering/skinning/opening spell ids (LootAction.cpp's ProfessionSpells
    // enum). Verify each exists in the 5.4.8 spell store before relying on this branch
    // (loot-stack-port.md section 5 risk 4) - Mining (2575) and Herb Gathering (2366) are confirmed
    // to exist in both cores; Skinning (8613) and Engineering (49383) are AC's ids kept as-is; 32605
    // and 32606 are AC's raw ids for the (rare) case where a creature corpse itself is the herb/ore
    // node.
    enum LootProfessionSpells : uint32
    {
        LOOT_SPELL_SKINNING = 8613,
        LOOT_SPELL_CREATURE_HERB_GATHERING = 32605,
        LOOT_SPELL_CREATURE_MINING = 32606,
        LOOT_SPELL_ENGINEERING = 49383,
        LOOT_SPELL_MINING = 2575,
        LOOT_SPELL_HERB_GATHERING = 2366,
    };

    // #843: AC LootAction.cpp:359-380 (IsGatheringNode), 1:1. Diagnostic-only classification for the
    // [GATHER] log line in StoreLootAction; does not affect loot behaviour.
    bool IsGatheringNode(GameObject* go)
    {
        if (!go)
            return false;

        if (go->GetGoType() == GAMEOBJECT_TYPE_FISHINGNODE)
            return true;

        LockEntry const* lockInfo = sLockStore.LookupEntry(go->GetGOInfo()->GetLockId());
        if (!lockInfo)
            return false;

        for (uint8 i = 0; i < MAX_LOCK_CASE; ++i)
        {
            if (lockInfo->Type[i] != LOCK_KEY_SKILL)
                continue;

            uint32 skillId = SkillByLockType(LockType(lockInfo->Index[i]));
            if (skillId == SKILL_HERBALISM || skillId == SKILL_MINING)
                return true;
        }

        return false;
    }
}

bool LootAction::Execute(Event /*event*/)
{
    if (!AI_VALUE(bool, "has available loot"))
        return false;

    LootObject prevLoot = AI_VALUE(LootObject, "loot target");
    LootObject const& lootObject = AI_VALUE(LootObjectStack*, "available loot")->GetLoot(sPlayerbotAIConfig->lootDistance);

    if (!prevLoot.IsEmpty() && prevLoot.guid != lootObject.guid)
    {
        // AC queues a raw CMSG_LOOT_RELEASE packet for the previous target. This fork's bot session
        // has no packet queue to drain outside PlayerbotAI's own pump, so call the session handler
        // that packet would trigger directly - the same convention as HandleUseItemOpcode
        // (UseItemAction.cpp) and HandleRepopRequestOpcode (ReleaseSpiritAction.cpp). Only release if
        // the guid is actually one of the bot's open loot views (mirrors HandleLootReleaseOpcode's
        // own guard, core:Handlers/LootHandler.cpp:382-416), since DoLootRelease itself does not
        // re-check that.
        for (auto const& view : bot->GetLootView())
        {
            if (view.second == prevLoot.guid)
            {
                bot->GetSession()->DoLootRelease(prevLoot.guid);
                break;
            }
        }
    }

    if (lootObject.guid.IsGameObject() && DisallowedGameObjects().count(lootObject.guid.GetEntry()))
    {
        return false;  // Game object ID is disallowed, so do not proceed
    }
    else
    {
        context->GetValue<LootObject>("loot target")->Set(lootObject);
        return true;
    }
}

bool LootAction::isUseful()
{
    // AC's IsSelfBot(bot) maps to this fork's PlayerbotAI::IsRealPlayer() (AI/PlayerbotAI.h:162) -
    // loot-stack-port.md section 2 step 4.2.
    return FreeMethodLoot() || !bot->GetGroup() || bot->GetGroup()->GetLootMethod() != FREE_FOR_ALL ||
           botAI->IsRealPlayer();
}

bool OpenLootAction::Execute(Event /*event*/)
{
    LootObject lootObject = AI_VALUE(LootObject, "loot target");
    bool result = DoLoot(lootObject);
    if (result)
    {
        AI_VALUE(LootObjectStack*, "available loot")->Remove(lootObject.guid);
        context->GetValue<LootObject>("loot target")->Set(LootObject());
    }
    return result;
}

bool OpenLootAction::DoLoot(LootObject& lootObject)
{
    if (lootObject.IsEmpty())
        return false;

    Creature* creature = botAI->GetCreature(lootObject.guid);
    if (creature && bot->GetDistance(creature) > INTERACTION_DISTANCE - 2.0f)
        return false;

    if (bot->IsMounted())
    {
        // #843: AC calls bot->Dismount(), which drops the mount aura on AC. On this core Unit::Dismount() leaves
        // SPELL_AURA_MOUNTED (#134), so dismount the way CheckMountStateAction does (CMSG_CANCEL_MOUNT_AURA).
        WorldPacket emptyPacket;
        bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
        botAI->SetNextCheckDelay(sPlayerbotAIConfig->lootDelay);
    }

    // AC: HasFlag(UNIT_DYNAMIC_FLAGS, ...) -> this core's update field is OBJECT_FIELD_DYNAMIC_FLAGS.
    if (creature && creature->HasFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE))
    {
        // Bit-packed CMSG_LOOT (core:Handlers/LootHandler.cpp:335-357): guid mask 4,5,2,7,0,1,3,6,
        // then bytes 3,5,0,6,4,1,7,2. HandleLootOpcode also AoE-loots every other allowed corpse
        // within 30 yd (LootHandler.cpp:309-376) - that is real-client, MoP-only-content behaviour
        // and correct to keep (loot-stack-port.md section 5 risk 1).
        WorldPacket p(CMSG_LOOT, 8);
        ObjectGuid const guid = lootObject.guid;
        p.WriteBit(guid[4]);
        p.WriteBit(guid[5]);
        p.WriteBit(guid[2]);
        p.WriteBit(guid[7]);
        p.WriteBit(guid[0]);
        p.WriteBit(guid[1]);
        p.WriteBit(guid[3]);
        p.WriteBit(guid[6]);
        p.FlushBits();
        p.WriteByteSeq(guid[3]);
        p.WriteByteSeq(guid[5]);
        p.WriteByteSeq(guid[0]);
        p.WriteByteSeq(guid[6]);
        p.WriteByteSeq(guid[4]);
        p.WriteByteSeq(guid[1]);
        p.WriteByteSeq(guid[7]);
        p.WriteByteSeq(guid[2]);
        bot->GetSession()->HandleLootOpcode(p);
        botAI->SetNextCheckDelay(sPlayerbotAIConfig->lootDelay);
        return true;
    }

    // Everything below this point casts, and every opening or gathering spell has a cast time -
    // PlayerbotAI::CastSpell refuses outright while isMoving(). Stop, then retry on a later tick.
    if (bot->isMoving())
    {
        bot->StopMoving();
        botAI->SetNextCheckDelay(sPlayerbotAIConfig->lootDelay);
        return false;
    }

    if (creature)
    {
        SkillType skill = creature->GetCreatureTemplate()->GetRequiredLootSkill();
        if (!CanOpenLock(skill, lootObject.reqSkillValue))
            return false;

        switch (skill)
        {
            case SKILL_ENGINEERING:
                return BotHasSkill(bot, SKILL_ENGINEERING) ? botAI->CastSpell(LOOT_SPELL_ENGINEERING, creature) : false;
            case SKILL_HERBALISM:
                return BotHasSkill(bot, SKILL_HERBALISM) ? botAI->CastSpell(LOOT_SPELL_CREATURE_HERB_GATHERING, creature) : false;
            case SKILL_MINING:
                return BotHasSkill(bot, SKILL_MINING) ? botAI->CastSpell(LOOT_SPELL_CREATURE_MINING, creature) : false;
            default:
                return BotHasSkill(bot, SKILL_SKINNING) ? botAI->CastSpell(LOOT_SPELL_SKINNING, creature) : false;
        }
    }

    GameObject* go = botAI->GetGameObject(lootObject.guid);
    if (go && bot->GetDistance(go) > INTERACTION_DISTANCE - 2.0f)
        return false;

    if (go && (go->GetGoState() != GO_STATE_READY))
        return false;

    // This prevents dungeon chests like Tribunal Chest (Halls of Stone) from being ninja'd by the
    // bots. Quest objects carry the same flag but are gated on quest state, which ActivateToQuest
    // answers. AC: GAMEOBJECT_FLAGS -> this core's update field is GAMEOBJECT_FIELD_FLAGS.
    if (go && go->HasFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_INTERACT_COND) && !go->ActivateToQuest(bot))
        return false;

    // This prevents raid chests like Gunship Armory (ICC) from being ninja'd by the bots
    if (go && go->HasFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE))
        return false;

    if (lootObject.skillId == SKILL_MINING)
        return BotHasSkill(bot, SKILL_MINING) ? botAI->CastSpell(LOOT_SPELL_MINING, bot) : false;

    if (lootObject.skillId == SKILL_HERBALISM)
        return BotHasSkill(bot, SKILL_HERBALISM) ? botAI->CastSpell(LOOT_SPELL_HERB_GATHERING, bot) : false;

    uint32 spellId = GetOpeningSpell(lootObject);
    if (!spellId)
        return false;

    // core:Spells/SpellEffects.cpp:2244-2334 (Spell::EffectOpenLock) is what actually opens the
    // chest via Player::SendLoot once the spell targets the GO - see the CastSpell OPEN_LOCK/
    // SKINNING hookup re-enabled in AI/PlayerbotAI.cpp. GameObject::Use has no CHEST case on this
    // core, so this spell path is required; a direct SendLoot would not be the AC way
    // (loot-stack-port.md section 2 step 4.2).
    return botAI->CastSpell(spellId, bot);
}

uint32 OpenLootAction::GetOpeningSpell(LootObject& lootObject)
{
    if (GameObject* go = botAI->GetGameObject(lootObject.guid))
        if (go->isSpawned())
            return GetOpeningSpell(lootObject, go);

    return 0;
}

uint32 OpenLootAction::GetOpeningSpell(LootObject& lootObject, GameObject* go)
{
    // AC: PlayerSpell::State/Active -> this core spells the same fields lower-case, state/active
    // (core:Entities/Player/Player.h:140-146).
    for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
    {
        uint32 spellId = itr->first;

        if (itr->second->state == PLAYERSPELL_REMOVED || !itr->second->active)
            continue;

        if (spellId == LOOT_SPELL_MINING || spellId == LOOT_SPELL_HERB_GATHERING)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            continue;

        if (CanOpenLock(lootObject, spellInfo, go))
            return spellId;
    }

    for (uint32 spellId = 0; spellId < sSpellMgr->GetSpellInfoStoreSize(); spellId++)
    {
        if (spellId == LOOT_SPELL_MINING || spellId == LOOT_SPELL_HERB_GATHERING)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            continue;

        if (CanOpenLock(lootObject, spellInfo, go))
            return spellId;
    }

    return sPlayerbotAIConfig->openGoSpell;
}

bool OpenLootAction::CanOpenLock(LootObject& /*lootObject*/, SpellInfo const* spellInfo, GameObject* go)
{
    for (uint8 effIndex = 0; effIndex <= EFFECT_2; effIndex++)
    {
        if (spellInfo->Effects[effIndex].Effect != SPELL_EFFECT_OPEN_LOCK &&
            spellInfo->Effects[effIndex].Effect != SPELL_EFFECT_SKINNING)
            return false;

        uint32 lockId = go->GetGOInfo()->GetLockId();
        if (!lockId)
            return false;

        LockEntry const* lockInfo = sLockStore.LookupEntry(lockId);
        if (!lockInfo)
            return false;

        for (uint8 j = 0; j < 8; ++j)
        {
            switch (lockInfo->Type[j])
            {
                case LOCK_KEY_SKILL:
                {
                    if (uint32(spellInfo->Effects[effIndex].MiscValue) != lockInfo->Index[j])
                        continue;

                    uint32 skillId = SkillByLockType(LockType(lockInfo->Index[j]));
                    if (skillId == SKILL_NONE)
                        return true;

                    if (CanOpenLock(skillId, lockInfo->Skill[j]))
                        return true;
                }
                break;
                default:
                    break;
            }
        }
    }

    return false;
}

bool OpenLootAction::CanOpenLock(uint32 skillId, uint32 reqSkillValue)
{
    uint32 skillValue = bot->GetSkillValue(skillId);
    return skillValue >= reqSkillValue || !reqSkillValue;
}

bool StoreLootAction::Execute(Event /*event*/)
{
    bool anyLooted = false;

    // AC decodes SMSG_LOOT_RESPONSE's bit-packed item/currency buffers directly (LootAction.cpp:
    // 386-434). On 5.4.8 SMSG_LOOT_RESPONSE is bit-packed differently and with a separate currency
    // section (core:Loot/LootMgr.cpp:1468ff) that this fork's bot session never decodes. The "loot
    // response" event only signals "a loot window opened"; walk every (viewGuid, sourceGuid) pair
    // the server-side GetLootView() already has open instead. This also naturally covers the AoE
    // loot HandleLootOpcode produces - one server-side Loot per corpse, one CMSG_AUTOSTORE_LOOT_ITEM
    // per corpse below (loot-stack-port.md section 2 step 4.2 point 3).
    auto const lootView = bot->GetLootView();  // copy: DoLootRelease below mutates the live map
    for (auto const& view : lootView)
    {
        ObjectGuid const viewGuid = view.first;
        ObjectGuid const srcGuid = view.second;

        Loot* loot = nullptr;
        Object* src = nullptr;
        Creature* creature = nullptr;

        if (srcGuid.IsGameObject())
        {
            GameObject* go = botAI->GetGameObject(srcGuid);
            if (!go)
                continue;
            loot = &go->loot;
            src = go;
        }
        else if (srcGuid.IsItem())
        {
            Item* item = bot->GetItemByGuid(srcGuid);
            if (!item)
                continue;
            loot = &item->loot;
            src = item;
        }
        else
        {
            creature = botAI->GetCreature(srcGuid);
            if (!creature)
                continue;
            loot = &creature->loot;
            src = creature;
        }

        if (!loot)
            continue;

        uint32 sourceGold = 0;
        if (loot->gold > 0)
        {
            sourceGold = loot->gold;
            // AC: HandleLootMoneyOpcode(empty packet). It handles all open views and the group
            // split itself (core:Handlers/LootHandler.cpp:161-307).
            WorldPacket p(CMSG_LOOT_MONEY, 0);
            bot->GetSession()->HandleLootMoneyOpcode(p);
        }

        std::vector<uint8> allowedSlots;
        std::vector<ItemTemplate const*> lootedProtos;  // for BroadcastLootingItem (AC LootAction.cpp:488)
        uint32 const maxSlot = loot->GetMaxSlotInLootFor(bot);
        for (uint32 slot = 0; slot < maxSlot; ++slot)
        {
            QuestItem* qitem = nullptr;
            QuestItem* ffaitem = nullptr;
            QuestItem* conditem = nullptr;
            QuestItem* currency = nullptr;
            QuestItem* personal = nullptr;
            LootItem* item = loot->LootItemInSlot(slot, bot, &qitem, &ffaitem, &conditem, &currency, &personal);
            if (!item)
                continue;  // already looted (core:Loot/LootMgr.cpp:1317-1320)

            if (!qitem && item->is_blocked)
                continue;  // roll ongoing

            if (!item->AllowedForPlayer(bot, src))
                continue;

            // MoP-only content: currency slots have no ItemTemplate, so AC's IsLootAllowed (which
            // looks one up) would reject them outright - always take them
            // (loot-stack-port.md section 2 step 4.2 point 3, section 5 risk 2).
            if (!item->currency && !StoreLootAction::IsLootAllowed(item->itemid, botAI))
                continue;

            // AC bag-space rule (LootAction.cpp:439-460): a bot with no live human master and a
            // near-full bag skips further stacks of an item it cannot stack onto.
            if (!botAI->HasRealPlayerMaster() && AI_VALUE(uint8, "bag space") > 80)
            {
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(item->itemid);
                uint32 const maxStack = proto ? proto->GetMaxStackSize() : 1;
                if (maxStack == 1)
                    continue;
            }

            allowedSlots.push_back(uint8(slot));
            if (!item->currency)
                if (ItemTemplate const* lootedProto = sObjectMgr->GetItemTemplate(item->itemid))
                    lootedProtos.push_back(lootedProto);
        }

        if (!allowedSlots.empty())
        {
            // Bit-packed CMSG_AUTOSTORE_LOOT_ITEM (core:Handlers/LootHandler.cpp:40-78):
            // WriteBits(n, 23), then per entry guid mask 2,7,0,6,5,3,1,4, then per entry bytes
            // 0,4,1,7,6,5,3,2 + uint8 slot.
            WorldPacket p(CMSG_AUTOSTORE_LOOT_ITEM);
            p.WriteBits(allowedSlots.size(), 23);
            for (size_t i = 0; i < allowedSlots.size(); ++i)
            {
                p.WriteBit(viewGuid[2]);
                p.WriteBit(viewGuid[7]);
                p.WriteBit(viewGuid[0]);
                p.WriteBit(viewGuid[6]);
                p.WriteBit(viewGuid[5]);
                p.WriteBit(viewGuid[3]);
                p.WriteBit(viewGuid[1]);
                p.WriteBit(viewGuid[4]);
            }
            p.FlushBits();
            for (uint8 slot : allowedSlots)
            {
                p.WriteByteSeq(viewGuid[0]);
                p.WriteByteSeq(viewGuid[4]);
                p.WriteByteSeq(viewGuid[1]);
                p.WriteByteSeq(viewGuid[7]);
                p.WriteByteSeq(viewGuid[6]);
                p.WriteByteSeq(viewGuid[5]);
                p.WriteByteSeq(viewGuid[3]);
                p.WriteByteSeq(viewGuid[2]);
                p << slot;
            }
            bot->GetSession()->HandleAutostoreLootItemOpcode(p);

            // #322: AC LootAction.cpp:488 broadcasts once per looted item, right after the autostore.
            for (ItemTemplate const* lootedProto : lootedProtos)
                BroadcastHelper::BroadcastLootingItem(botAI, bot, lootedProto);
        }

        if (!allowedSlots.empty() || sourceGold)
        {
            anyLooted = true;

            // AC local diff LootAction.cpp:491-500. Only creature kills are logged here - the
            // run-scenario 1:1 parity check (loot-stack-port.md section 4) counts these lines, one
            // per corpse actually looted (including multiple corpses opened together by a single
            // AoE CMSG_LOOT).
            // AC LootAction.cpp:491 (#530 item 6): the whole [LOOT]/[GATHER] block is gated on logInGroupOnly.
            bool const logLoot = !sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && botAI->HasRealPlayerMaster());
            if (logLoot && creature)
                TC_LOG_DEBUG("playerbots", "[LOOT] %s looted %s (%u items, %u copper)", bot->GetName().c_str(),
                             creature->GetName().c_str(), uint32(allowedSlots.size()), sourceGold);
            // #843: AC LootAction.cpp:496-498 - herb/ore/fishing node loot gets its own [GATHER] line (the
            // t843 gather/h row counts it; MoP logged no gathering at all before this).
            else if (logLoot && srcGuid.IsGameObject())
                if (GameObject* lootGo = botAI->GetGameObject(srcGuid))
                    if (IsGatheringNode(lootGo))
                        TC_LOG_DEBUG("playerbots", "[GATHER] %s gathered %s (%u items)", bot->GetName().c_str(),
                                     lootGo->GetGOInfo()->name.c_str(), uint32(allowedSlots.size()));
        }
    }

    // #958: autostore/money above already released (and destroyed) a container item they emptied.
    for (auto const& view : lootView)
        ReleaseIfStillOpen(bot, view.second);

    LootObject target = context->GetValue<LootObject>("loot target")->Get();
    if (!target.IsEmpty())
        AI_VALUE(LootObjectStack*, "available loot")->Remove(target.guid);

    return anyLooted;
}

bool StoreLootAction::IsLootAllowed(uint32 itemid, PlayerbotAI* botAI)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    LootStrategy* lootStrategy = context->GetValue<LootStrategy*>("loot strategy")->Get();

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemid);
    if (!proto)
        return false;

    std::set<uint32>& lootItems = context->GetValue<std::set<uint32>&>("always loot list")->Get();
    if (lootItems.find(itemid) != lootItems.end())
        return true;

    int32 const max = proto->MaxCount;
    if (max > 0 && botAI->GetBot()->HasItemCount(itemid, uint32(max), true))
        return false;

    if (proto->StartQuest)
        return true;

    // AC walks RequiredItemId/RequiredItemCount over the quest log directly; this core's objective
    // model equivalent is Player::HasQuestForItem (same substitution as LootObjectStack::Refresh -
    // loot-stack-port.md section 2 step 1).
    if (botAI->GetBot()->HasQuestForItem(itemid))
        return true;

    return lootStrategy->CanLoot(proto, context);
}

bool ReleaseLootAction::Execute(Event /*event*/)
{
    // AC blindly releases every nearby GO/corpse guid, whether or not it is an open loot view; this
    // core's DoLootRelease does not re-check membership the way HandleLootReleaseOpcode does, so
    // only release the bot's actually-open views (loot-stack-port.md section 2 step 4.2).
    // #958: DoLootRelease erases from the live view map, so walk a copy (iterating the live map
    // erased the element under the range-for iterator).
    auto const lootView = bot->GetLootView();
    for (auto const& view : lootView)
        ReleaseIfStillOpen(bot, view.second);

    return true;
}

bool MoveToLootAction::Execute(Event /*event*/)
{
    LootObject loot = AI_VALUE(LootObject, "loot target");
    WorldObject* wo = loot.GetWorldObject(bot);
    if (!wo)
        return false;

    return MoveNear(wo, sPlayerbotAIConfig->contactDistance);
}
