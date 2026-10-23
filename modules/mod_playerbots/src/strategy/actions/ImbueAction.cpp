/*
 * Ported from AC mod-playerbots Ai/Base/Actions/ImbueAction.cpp:104-186 (ImbueWithStoneAction,
 * ImbueWithOilAction) plus AC Bot/PlayerbotAI.cpp:5612-5636 (FindItemInInventory), :5685
 * (FindConsumable), :5751 (FindStoneFor), :5800 (FindOilFor) and the item-id enums at AC
 * Bot/PlayerbotAI.h:189-230 (issue #434).
 *
 * Deviations from AC, all intentional:
 *  - The Find* helpers and the item-id enums are file-local here instead of PlayerbotAI members, so
 *    that AI/PlayerbotAI.h (included by ~49 module files) is not touched. Same approach as
 *    PlayerbotPetFollow (PetsAction.h). Behaviour is identical.
 *  - PlayerbotAI::ImbueItem(item, slot) is not a member either. AC builds a WotLK-shaped
 *    CMSG_USE_ITEM packet and queues it; on this core CMSG_USE_ITEM is bit-packed and the
 *    WotLK-shaped packet aborts the process (see UseTrinketAction::UseTrinket,
 *    GenericSpellActions.cpp:376). We do what the opcode handler does after parsing: set an item
 *    target and call Player::CastItemUseSpell.
 *  - "apply poison" is not ported (rogue poisons are self-cast spells in 5.4.8; see ImbueAction.h).
 *  - 5.4.8 spec adaptation in FindOilFor: AC's talent tab numbers become Specializations ids
 *    (Priest tab 2 = Shadow, Druid 0/1/2 = Balance/Feral/Restoration, Paladin 0/1/2 =
 *    Holy/Protection/Retribution). Druid Guardian is treated like Feral (AC's Feral tab covered
 *    bear and cat): no oil.
 *  - AC's weapon-subclass table uses ITEM_SUBCLASS_WEAPON_FIST; this core names it
 *    ITEM_SUBCLASS_WEAPON_FIST_WEAPON (ItemPrototype.h:390).
 *  - AC's bot->HasAura(SPELL_AURA_MOD_STEALTH) (an aura TYPE passed as a spell id, never true) is
 *    kept literally to stay 1:1; it is a no-op in AC and here.
 */

#include "ImbueAction.h"

#include <vector>

#include "AiFactory.h"
#include "Event.h"
#include "Playerbots.h"
#include "ScriptMgr.h"
#include "SpellDefines.h"

namespace
{
// AC Bot/PlayerbotAI.h:189-230. Item ids exist unchanged in the 5.4.8 item_template (verified
// against mop_world: class 0, subclass 8, spelltrigger ON_USE, spell = temporary weapon enchant).
enum SharpeningStoneId
{
    ROUGH_SHARPENING_STONE = 2862,
    COARSE_SHARPENING_STONE = 2863,
    HEAVY_SHARPENING_STONE = 2871,
    SOLID_SHARPENING_STONE = 7964,
    DENSE_SHARPENING_STONE = 12404,
    ELEMENTAL_SHARPENING_STONE = 18262,
    FEL_SHARPENING_STONE = 23528,
    ADAMANTITE_SHARPENING_STONE = 23529
};

enum WeightstoneId
{
    ROUGH_WEIGHTSTONE = 3239,
    COARSE_WEIGHTSTONE = 3240,
    HEAVY_WEIGHTSTONE = 3241,
    SOLID_WEIGHTSTONE = 7965,
    DENSE_WEIGHTSTONE = 12643,
    FEL_WEIGHTSTONE = 28420,
    ADAMANTITE_WEIGHTSTONE = 28421
};

enum WizardOilId
{
    MINOR_WIZARD_OIL = 20744,
    LESSER_WIZARD_OIL = 20746,
    WIZARD_OIL = 20750,
    BRILLIANT_WIZARD_OIL = 20749,
    SUPERIOR_WIZARD_OIL = 22522
};

enum ManaOilId
{
    MINOR_MANA_OIL = 20745,
    LESSER_MANA_OIL = 20747,
    BRILLIANT_MANA_OIL = 20748,
    SUPERIOR_MANA_OIL = 22521
};

// AC PlayerbotAI::FindItemInInventory (Bot/PlayerbotAI.cpp:5612).
template <typename Pred>
Item* FindItemInInventory(Player* bot, Pred checkItem)
{
    // main backpack
    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
    {
        if (Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            ItemTemplate const* pItemProto = pItem->GetTemplate();
            if (pItemProto && bot->CanUseItem(pItemProto) == EQUIP_ERR_OK && checkItem(pItemProto))
                return pItem;
        }
    }

    // other removable bags
    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        if (Bag const* const pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag))
        {
            for (uint8 slot = 0; slot < pBag->GetBagSize(); ++slot)
            {
                if (Item* const pItem = bot->GetItemByPos(bag, slot))
                {
                    ItemTemplate const* pItemProto = pItem->GetTemplate();
                    if (pItemProto && bot->CanUseItem(pItemProto) == EQUIP_ERR_OK && checkItem(pItemProto))
                        return pItem;
                }
            }
        }
    }

    return nullptr;
}

// AC PlayerbotAI::FindConsumable (Bot/PlayerbotAI.cpp:5685).
Item* FindConsumable(Player* bot, uint32 itemId)
{
    return FindItemInInventory(bot,
        [itemId](ItemTemplate const* pItemProto) -> bool
        {
            return (pItemProto->Class == ITEM_CLASS_CONSUMABLE || pItemProto->Class == ITEM_CLASS_TRADE_GOODS) &&
                   pItemProto->ItemId == itemId;
        });
}

// AC PlayerbotAI::FindStoneFor (Bot/PlayerbotAI.cpp:5751).
Item* FindStoneFor(Player* bot, Item* weapon)
{
    if (!weapon)
        return nullptr;

    ItemTemplate const* pProto = weapon->GetTemplate();
    if (!pProto)
        return nullptr;

    static const std::vector<uint32> uPrioritizedSharpStoneIds = {
        ADAMANTITE_SHARPENING_STONE, FEL_SHARPENING_STONE,   ELEMENTAL_SHARPENING_STONE, DENSE_SHARPENING_STONE,
        SOLID_SHARPENING_STONE,      HEAVY_SHARPENING_STONE, COARSE_SHARPENING_STONE,    ROUGH_SHARPENING_STONE};

    static const std::vector<uint32> uPrioritizedWeightStoneIds = {
        ADAMANTITE_WEIGHTSTONE, FEL_WEIGHTSTONE,    ELEMENTAL_SHARPENING_STONE, DENSE_WEIGHTSTONE, SOLID_WEIGHTSTONE,
        HEAVY_WEIGHTSTONE,      COARSE_WEIGHTSTONE, ROUGH_WEIGHTSTONE};

    Item* stone = nullptr;
    if (pProto->SubClass == ITEM_SUBCLASS_WEAPON_SWORD || pProto->SubClass == ITEM_SUBCLASS_WEAPON_SWORD2 ||
        pProto->SubClass == ITEM_SUBCLASS_WEAPON_AXE || pProto->SubClass == ITEM_SUBCLASS_WEAPON_AXE2 ||
        pProto->SubClass == ITEM_SUBCLASS_WEAPON_DAGGER || pProto->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM)
    {
        for (uint32 id : uPrioritizedSharpStoneIds)
        {
            stone = FindConsumable(bot, id);
            if (stone)
                return stone;
        }
    }
    else if (pProto->SubClass == ITEM_SUBCLASS_WEAPON_MACE || pProto->SubClass == ITEM_SUBCLASS_WEAPON_MACE2 ||
             pProto->SubClass == ITEM_SUBCLASS_WEAPON_STAFF || pProto->SubClass == ITEM_SUBCLASS_WEAPON_FIST_WEAPON)
    {
        for (uint32 id : uPrioritizedWeightStoneIds)
        {
            stone = FindConsumable(bot, id);
            if (stone)
                return stone;
        }
    }

    return stone;
}

// AC PlayerbotAI::FindOilFor (Bot/PlayerbotAI.cpp:5800).
Item* FindOilFor(Player* bot, Item* weapon)
{
    if (!weapon)
        return nullptr;

    if (!weapon->GetTemplate())
        return nullptr;

    static const std::vector<uint32> uPrioritizedWizardOilIds = {
        BRILLIANT_WIZARD_OIL, SUPERIOR_WIZARD_OIL, WIZARD_OIL,      LESSER_WIZARD_OIL, MINOR_WIZARD_OIL,
        BRILLIANT_MANA_OIL,   SUPERIOR_MANA_OIL,   LESSER_MANA_OIL, MINOR_MANA_OIL};

    static const std::vector<uint32> uPrioritizedManaOilIds = {
        BRILLIANT_MANA_OIL,  SUPERIOR_MANA_OIL, LESSER_MANA_OIL,   MINOR_MANA_OIL,  BRILLIANT_WIZARD_OIL,
        SUPERIOR_WIZARD_OIL, WIZARD_OIL,        LESSER_WIZARD_OIL, MINOR_WIZARD_OIL};

    Item* oil = nullptr;
    uint8 botClass = bot->GetClass();
    int specTab = int(AiFactory::GetPlayerSpecTab(bot));

    std::vector<uint32> const* prioritizedOils = nullptr;
    switch (botClass)
    {
        case CLASS_PRIEST:
            // AC: specTab == 2 (Shadow)
            prioritizedOils = (specTab == SPEC_PRIEST_SHADOW) ? &uPrioritizedWizardOilIds : &uPrioritizedManaOilIds;
            break;
        case CLASS_MAGE:
            prioritizedOils = &uPrioritizedWizardOilIds;
            break;
        case CLASS_DRUID:
            if (specTab == SPEC_DRUID_BALANCE)  // AC tab 0
                prioritizedOils = &uPrioritizedWizardOilIds;
            else if (specTab == SPEC_DRUID_FERAL || specTab == SPEC_DRUID_GUARDIAN)  // AC tab 1 (bear + cat)
                prioritizedOils = nullptr;
            else  // Restoration or any other/unspecified spec
                prioritizedOils = &uPrioritizedManaOilIds;
            break;
        case CLASS_HUNTER:
            prioritizedOils = &uPrioritizedManaOilIds;
            break;
        case CLASS_PALADIN:
            if (specTab == SPEC_PALADIN_PROTECTION)  // AC tab 1
                prioritizedOils = &uPrioritizedWizardOilIds;
            else if (specTab == SPEC_PALADIN_RETRIBUTION)  // AC tab 2
                prioritizedOils = nullptr;
            else  // Holy or any other/unspecified spec
                prioritizedOils = &uPrioritizedManaOilIds;
            break;
        default:
            prioritizedOils = &uPrioritizedManaOilIds;
            break;
    }

    if (prioritizedOils)
    {
        for (uint32 id : *prioritizedOils)
        {
            oil = FindConsumable(bot, id);
            if (oil)
                return oil;
        }
    }

    return oil;
}

// Replaces AC PlayerbotAI::ImbueItem(item, targetInventorySlot) (Bot/PlayerbotAI.cpp:6084-6140);
// see the file header for why the CMSG_USE_ITEM packet is not built here.
void ImbueItem(Player* bot, Item* item, uint8 targetInventorySlot)
{
    if (!item || targetInventorySlot >= EQUIPMENT_SLOT_END)
        return;

    Item* const targetItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, targetInventorySlot);
    if (!targetItem)
        return;

    if (bot->IsNonMeleeSpellCasted(true))
        return;

    if (item->GetTemplate()->Bonding == BIND_ON_USE && !item->IsSoulBound())
    {
        item->SetState(ITEM_CHANGED, bot);
        item->SetBinding(true);
    }

    SpellCastTargets targets;
    targets.SetItemTarget(targetItem);
    if (!sScriptMgr->OnItemUse(bot, item, targets))
        bot->CastItemUseSpell(item, targets, 0, 0);
}
}  // namespace

// Search and apply stone to weapons
ImbueWithStoneAction::ImbueWithStoneAction(PlayerbotAI* botAI) : Action(botAI, "apply stone") {}

bool ImbueWithStoneAction::Execute(Event /*event*/)
{
    if (bot->IsInCombat())
        return false;

    // remove stealth
    if (bot->HasAura(SPELL_AURA_MOD_STEALTH))
        bot->RemoveAurasByType(SPELL_AURA_MOD_STEALTH);

    if (bot->GetStandState() != UNIT_STAND_STATE_STAND)
        bot->SetStandState(UNIT_STAND_STATE_STAND);

    // Mainhand ...
    Item* stone = nullptr;
    Item* weapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (weapon && weapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) == 0)
    {
        stone = FindStoneFor(bot, weapon);
        if (stone)
        {
            ImbueItem(bot, stone, EQUIPMENT_SLOT_MAINHAND);
            botAI->SetNextCheckDelay(5);
        }
    }

    //... and offhand
    weapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (weapon && weapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) == 0)
    {
        stone = FindStoneFor(bot, weapon);
        if (stone)
        {
            ImbueItem(bot, stone, EQUIPMENT_SLOT_OFFHAND);
            botAI->SetNextCheckDelay(5);
        }
    }

    return true;
}

// Search and apply oil to weapons
ImbueWithOilAction::ImbueWithOilAction(PlayerbotAI* botAI) : Action(botAI, "apply oil") {}

bool ImbueWithOilAction::Execute(Event /*event*/)
{
    if (bot->IsInCombat())
        return false;

    // remove stealth
    if (bot->HasAura(SPELL_AURA_MOD_STEALTH))
        bot->RemoveAurasByType(SPELL_AURA_MOD_STEALTH);

    if (bot->GetStandState() != UNIT_STAND_STATE_STAND)
        bot->SetStandState(UNIT_STAND_STATE_STAND);

    Item* oil = nullptr;
    Item* weapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (weapon && weapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) == 0)
    {
        oil = FindOilFor(bot, weapon);
        if (oil)
        {
            ImbueItem(bot, oil, EQUIPMENT_SLOT_MAINHAND);
            botAI->SetNextCheckDelay(5);
        }
    }

    return true;
}
