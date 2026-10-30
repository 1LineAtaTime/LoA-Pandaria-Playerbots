/*
* This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "GenericSpellActions.h"

#include "BgTacticsIC.h"  // #675: IocBeyondAc::RamTarget
#include "BgTacticsSA.h"  // #765: SotaBeyondAc::RamTarget

#include "Event.h"
//#include "ItemTemplate.h"
#include "ObjectDefines.h"
#include "Opcodes.h"
#include "Player.h"
#include "Playerbots.h"
#include "ScriptMgr.h"
#include "SpellDefines.h"
#include "SpellHistory.h"  // #604: SpellHistory::IsReady in UseTrinketAction::UseTrinket
#include "ServerFacade.h"
#include "WorldPacket.h"

CastSpellAction::CastSpellAction(PlayerbotAI* botAI, std::string const spell)
    : Action(botAI, spell), range(botAI->GetRange("spell")), spell(spell)
{
}

bool CastSpellAction::Execute(Event event)
{
    if (spell == "conjure food" || spell == "conjure water")
    {
        // uint32 id = AI_VALUE2(uint32, "spell id", spell);
        // if (!id)
        //     return false;

        uint32 castId = 0;

        for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
        {
            uint32 spellId = itr->first;

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!spellInfo)
                continue;

            std::string const namepart = spellInfo->SpellName;
            std::wstring wnamepart;
            if (!Utf8toWStr(namepart, wnamepart))
                return false;

            wstrToLower(wnamepart);

            if (!Utf8FitTo(spell, wnamepart))
                continue;

            if (spellInfo->Effects[0].Effect != SPELL_EFFECT_CREATE_ITEM)
                continue;

            uint32 itemId = spellInfo->Effects[0].ItemType;
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
            if (!proto)
                continue;

            if (bot->CanUseItem(proto) != EQUIP_ERR_OK)
                continue;

            if (spellInfo->Id > castId)
                castId = spellInfo->Id;
        }

        return botAI->CastSpell(castId, bot);
    }

    if (!botAI->CastSpell(spell, GetTarget()))
    {
        // issue #98: a failed cast was retried on the very next tick, so an ability that cannot land
        // (target moved out of range, line of sight, already charging) was re-picked endlessly - one
        // warrior logged 1715 charge attempts while the rest of its rotation never ran. Put the spell
        // on a short cooldown on failure so the engine falls through to something else, the way a
        // player who cannot spam a failed ability behaves.
        uint32 const failedSpellId = AI_VALUE2(uint32, "spell id", spell);
        if (failedSpellId)
            bot->AddSpellCooldown(failedSpellId, 0, 750);

        return false;
    }

    return true;
}

bool CastSpellAction::isPossible()
{
    if (botAI->IsInVehicle() && !botAI->IsInVehicle(false, false, true))
    {
        // AC GenericSpellActions.cpp:213 (#530 item 6)
        if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && botAI->HasRealPlayerMaster()))
        {
            TC_LOG_DEBUG("playerbots", "Can cast spell failed. Vehicle. - bot name: %s", bot->GetName().c_str());
        }
        return false;
    }

    if (spell == "mount" && !bot->IsMounted() && !bot->IsInCombat())
        return true;

    if (spell == "mount" && bot->IsInCombat())
    {
        // AC GenericSpellActions.cpp:224 (#530 item 6)
        if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && botAI->HasRealPlayerMaster()))
        {
            TC_LOG_DEBUG("playerbots", "Can cast spell failed. Mount. - bot name: %s", bot->GetName().c_str());
        }
        bot->Dismount();
        return false;
    }

    Spell* currentSpell = bot->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    return botAI->CanCastSpell(spell, GetTarget());
}

bool CastSpellAction::isUseful()
{
    if (botAI->IsInVehicle() && !botAI->IsInVehicle(false, false, true))
        return false;

    if (spell == "mount" && !bot->IsMounted() && !bot->IsInCombat())
        return true;

    if (spell == "mount" && bot->IsInCombat())
    {
        bot->Dismount();
        return false;
    }

    Unit* spellTarget = GetTarget();
    if (!spellTarget)
        return false;

    if (!spellTarget->IsInWorld() || spellTarget->GetMapId() != bot->GetMapId())
        return false;

    // float combatReach = bot->GetCombatReach() + spellTarget->GetCombatReach();
    // if (!botAI->IsRanged(bot))
    //     combatReach += 4.0f / 3.0f;

    return spellTarget &&
        AI_VALUE2(bool, "spell cast useful",
            spell);  // && sServerFacade->GetDistance2d(bot, spellTarget) <= (range + combatReach);
}

CastMeleeSpellAction::CastMeleeSpellAction(PlayerbotAI* botAI, std::string const spell) : CastSpellAction(botAI, spell)
{
    range = ATTACK_DISTANCE;
}

bool CastMeleeSpellAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    if (!bot->IsWithinMeleeRange(target))
        return false;

    return CastSpellAction::isUseful();
}

CastMeleeDebuffSpellAction::CastMeleeDebuffSpellAction(PlayerbotAI* botAI, std::string const spell, bool isOwner, float needLifeTime) : CastDebuffSpellAction(botAI, spell, isOwner, needLifeTime)
{
    range = ATTACK_DISTANCE;
}

bool CastMeleeDebuffSpellAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    if (!bot->IsWithinMeleeRange(target))
        return false;

    return CastDebuffSpellAction::isUseful();
}

bool CastAuraSpellAction::isUseful()
{
    if (!GetTarget() || !CastSpellAction::isUseful())
        return false;
    Aura* aura = botAI->GetAura(spell, GetTarget(), isOwner, checkDuration);
    // #628 (AC GenericSpellActions.cpp:265-272, BuffBelowRefreshTarget): same as the old rule when no pass pends.
    if (!aura)
        return true;
    return botAI->forceRebuff.BuffBelowRefreshTarget(aura->GetDuration(), aura->GetMaxDuration(), beforeDuration);
}

CastEnchantItemAction::CastEnchantItemAction(PlayerbotAI* botAI, std::string const spell)
    : CastSpellAction(botAI, spell)
{
    range = botAI->GetRange("spell");
}

bool CastEnchantItemAction::isPossible()
{
    // if (!CastSpellAction::isPossible()) {

    //     botAI->TellMasterNoFacing("Impossible: " + spell);
    //     return false;
    // }

    uint32 spellId = AI_VALUE2(uint32, "spell id", spell);

    // bool ok = AI_VALUE2(Item*, "item for spell", spellId);
    // Item* item = AI_VALUE2(Item*, "item for spell", spellId);
    // botAI->TellMasterNoFacing("spell: " + spell + ", spell id: " + std::to_string(spellId) + " item for spell: " +
    // std::to_string(ok));
    return spellId && AI_VALUE2(Item*, "item for spell", spellId);
}

CastHealingSpellAction::CastHealingSpellAction(PlayerbotAI* botAI, std::string const spell, uint8 estAmount,
    HealingManaEfficiency manaEfficiency, bool isOwner)
    : CastAuraSpellAction(botAI, spell, isOwner), estAmount(estAmount), manaEfficiency(manaEfficiency)
{
    range = botAI->GetRange("heal");
}

bool CastHealingSpellAction::isUseful() { return CastAuraSpellAction::isUseful(); }

bool CastAoeHealSpellAction::isUseful() { return CastSpellAction::isUseful(); }

CastCureSpellAction::CastCureSpellAction(PlayerbotAI* botAI, std::string const spell) : CastSpellAction(botAI, spell)
{
    range = botAI->GetRange("heal");
}

Value<Unit*>* CurePartyMemberAction::GetTargetValue()
{
    return context->GetValue<Unit*>("party member to dispel", dispelType);
}

Value<Unit*>* BuffOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>("party member without aura", spell);
}

CastShootAction::CastShootAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "shoot")
{
    if (Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
    {
        spell = "shoot";

        switch (pItem->GetTemplate()->SubClass)
        {
        case ITEM_SUBCLASS_WEAPON_GUN:
            spell += " gun";
            break;
        case ITEM_SUBCLASS_WEAPON_BOW:
            spell += " bow";
            break;
        case ITEM_SUBCLASS_WEAPON_CROSSBOW:
            spell += " crossbow";
            break;
        }
    }
}

// #930 (MoP-only; AC GenericSpellActions.cpp:431-437 decides the same through CheckCast): patch 5.0.4 removed the ranged
// slot, so wands sit in the main hand (Player::FindEquipSlot INVTYPE_RANGEDRIGHT) and the ranged-slot lookup above never
// finds one. Shoot (5019) needs a wand there; without one the action is impossible up front. Priests, mages and warlocks
// without a wand otherwise ran a full CheckCast for it (EQUIPPED_ITEM_CLASS) on every tick nothing else could run.
bool CastShootAction::isPossible()
{
    if (spell == "shoot")
    {
        Item* const weapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        ItemTemplate const* proto = weapon ? weapon->GetTemplate() : nullptr;
        if (!proto || proto->Class != ITEM_CLASS_WEAPON || proto->SubClass != ITEM_SUBCLASS_WEAPON_WAND)
            return false;
    }

    return CastSpellAction::isPossible();
}

NextAction** CastSpellAction::getPrerequisites()
{
    return nullptr;
}

Value<Unit*>* CastDebuffSpellOnAttackerAction::GetTargetValue()
{
    return context->GetValue<Unit*>("attacker without aura", spell);
}

Value<Unit*>* CastDebuffSpellOnMeleeAttackerAction::GetTargetValue()
{
    return context->GetValue<Unit*>("melee attacker without aura", spell);
}

CastBuffSpellAction::CastBuffSpellAction(PlayerbotAI* botAI, std::string const spell, bool checkIsOwner, uint32 beforeDuration)
    : CastAuraSpellAction(botAI, spell, checkIsOwner, false, beforeDuration)
{
    range = botAI->GetRange("spell");
}

// #628 (AC GenericSpellActions.cpp:285-290). AC calls botAI->CastSpell directly; here the base
// CastSpellAction::Execute is used so this fork's failed-cast cooldown (#98) is kept for buff spells.
bool CastBuffSpellAction::Execute(Event event)
{
    botAI->forceRebuff.NoteBuffWork();

    return CastAuraSpellAction::Execute(event);
}

Value<Unit*>* CastSpellOnEnemyHealerAction::GetTargetValue()
{
    return context->GetValue<Unit*>("enemy healer target", spell);
}

// #855 (epic #854): see GenericSpellActions.h.
Value<Unit*>* CastSpellOnEnemyCasterAction::GetTargetValue() { return context->GetValue<Unit*>("enemy caster target"); }

bool CastSpellOnEnemyCasterAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target || !CastSpellAction::isUseful())
        return false;

    uint32 const spellId = AI_VALUE2(uint32, "spell id", spell);
    SpellInfo const* spellInfo = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
    if (!spellInfo)
        return false;

    float const maxRange = spellInfo->GetMaxRange(false, bot);
    if (maxRange <= NOMINAL_MELEE_RANGE)
        return bot->IsWithinMeleeRange(target);

    return bot->IsWithinDistInMap(target, maxRange);
}

Value<Unit*>* CastSnareSpellAction::GetTargetValue() { return context->GetValue<Unit*>("snare target", spell); }

Value<Unit*>* CastCrowdControlSpellAction::GetTargetValue() { return context->GetValue<Unit*>("cc target", getName()); }

bool CastCrowdControlSpellAction::Execute(Event event) { return botAI->CastSpell(getName(), GetTarget()); }

bool CastCrowdControlSpellAction::isPossible() { return botAI->CanCastSpell(getName(), GetTarget()); }

bool CastCrowdControlSpellAction::isUseful() { return true; }

std::string const CastProtectSpellAction::GetTargetName() { return "party member to protect"; }

bool CastProtectSpellAction::isUseful() { return GetTarget() && !botAI->HasAura(spell, GetTarget()); }

// #765 (beyond AC, SotA only): a driven Battleground Demolisher rams the enemy in its Ram cone (SotaBeyondAc::RamTarget,
// BgTacticsSA.cpp); every other vehicle spell, and Ram anywhere else, keeps AC's "current target".
static Unit* VehicleSpellTarget(Player* bot, AiObjectContext* context, std::string const& spell, Unit* target)
{
    if (spell == "ram")
    {
        if (Unit* ram = SotaBeyondAc::RamTarget(bot, context))
            return ram;

        // #675: the IoC Demolisher's Ram (67441) the same way (map 628 only, BgTacticsIC.cpp)
        if (Unit* ram = IocBeyondAc::RamTarget(bot, context))
            return ram;
    }

    return target;
}

bool CastVehicleSpellAction::isPossible()
{
    // #581, AC GenericSpellActions.cpp:483-487.
    uint32 spellId = AI_VALUE2(uint32, "vehicle spell id", spell);
    return botAI->CanCastVehicleSpell(spellId, VehicleSpellTarget(bot, context, spell, GetTarget()));
}

bool CastVehicleSpellAction::isUseful() { return botAI->IsInVehicle(false, true); }

bool CastVehicleSpellAction::Execute(Event event)
{
    // #581, AC GenericSpellActions.cpp:491-495.
    uint32 spellId = AI_VALUE2(uint32, "vehicle spell id", spell);
    return botAI->CastVehicleSpell(spellId, VehicleSpellTarget(bot, context, spell, GetTarget()));
}

bool UseTrinketAction::Execute(Event event)
{
    Item* trinket1 = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_TRINKET1);

    if (trinket1 && UseTrinket(trinket1))
        return true;

    Item* trinket2 = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_TRINKET2);

    if (trinket2 && UseTrinket(trinket2))
        return true;

    return false;
}

bool UseTrinketAction::UseTrinket(Item* item)
{
    if (bot->CanUseItem(item) != EQUIP_ERR_OK)
        return false;

    if (bot->IsNonMeleeSpellCasted(true))
        return false;

    uint8 bagIndex = item->GetBagSlot();
    uint8 slot = item->GetSlot();
    uint8 spell_index = 0;
    uint8 cast_count = 1;
    ObjectGuid item_guid = item->GetGUID();
    uint32 glyphIndex = 0;
    uint8 castFlags = 0;
    uint32 targetFlag = TARGET_FLAG_NONE;
    uint32 spellId = 0;
    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (item->GetTemplate()->Spells[i].SpellId > 0 && item->GetTemplate()->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_USE)
        {
            spellId = item->GetTemplate()->Spells[i].SpellId;

            // #604/#606 (AC Ai/Base/Actions/GenericSpellActions.cpp:552-616): AC only uses a trinket whose on-use spell
            // is positive and applies an aura or restores mana, and skips it while the item or its category is on
            // cooldown. Without these gates a summon / non-aura on-use (e.g. 68270/68271 Brewmaiden) or an item on
            // category cooldown "succeeded" every tick: CastItemUseSpell failed silently, the action still returned
            // true and ended the tick, so wave 10's assassination rogue never reached Mutilate (42x "use trinket - OK").
            // The 5.4.8 core tracks item and category cooldowns in SpellHistory, so IsReady() replaces AC's local
            // expiry caches. AC's tank-defensive / mixed ON_EQUIP+ON_USE exclusions are not ported (combat-rating
            // masks differ in 5.x); the aura/mana gate already filters those procs here.
            SpellInfo const* useSpellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!useSpellInfo || !useSpellInfo->IsPositive())
                return false;

            if (!bot->GetSpellHistory()->IsReady(useSpellInfo, item->GetEntry(), false))
                return false;

            bool applyAura = false;
            bool restoresMana = false;
            for (uint8 e = 0; e < MAX_SPELL_EFFECTS; ++e)
            {
                SpellEffectInfo const& effectInfo = useSpellInfo->Effects[e];
                if (effectInfo.Effect == SPELL_EFFECT_APPLY_AURA)
                    applyAura = true;
                if ((effectInfo.Effect == SPELL_EFFECT_ENERGIZE && effectInfo.MiscValue == POWER_MANA) ||
                    (effectInfo.Effect == SPELL_EFFECT_APPLY_AURA &&
                     effectInfo.ApplyAuraName == SPELL_AURA_PERIODIC_ENERGIZE && effectInfo.MiscValue == POWER_MANA))
                    restoresMana = true;
            }

            if (!applyAura && !restoresMana)
                return false;

            // AC :599-609: a mana-restore trinket waits for medium mana.
            if (restoresMana && (!AI_VALUE2(bool, "has mana", "self target") ||
                                 AI_VALUE2(uint8, "mana", "self target") >= sPlayerbotAIConfig->mediumMana))
                return false;

            if (!botAI->CanCastSpell(spellId, bot, false))
            {
                return false;
            }
            break;
        }
    }
    if (!spellId)
        return false;
    Unit* target = AI_VALUE(Unit*, "current target");
    // 5.4.8: CMSG_USE_ITEM is bit-packed on this core. The WotLK-shaped packet AC builds here made
    // HandleUseItemOpcode read past the end and abort the process (#194 S8, once "use trinket" and
    // "potions" started firing). Do what the handler does after parsing the packet instead.
    if (item->GetTemplate()->Bonding == BIND_ON_USE && !item->IsSoulBound())
    {
        item->SetState(ITEM_CHANGED, bot);
        item->SetBinding(true);
    }
    SpellCastTargets targets;
    targets.SetUnitTarget(target ? target : bot);
    if (!sScriptMgr->OnItemUse(bot, item, targets))
        bot->CastItemUseSpell(item, targets, cast_count, glyphIndex);
    return true;
}

Value<Unit*>* BuffOnMainTankAction::GetTargetValue() { return context->GetValue<Unit*>("main tank", spell); }

bool CastDebuffSpellAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
    {
        return false;
    }
    return CastAuraSpellAction::isUseful() &&
        (target->GetHealth() / AI_VALUE(float, "estimated group dps")) >= needLifeTime;
}