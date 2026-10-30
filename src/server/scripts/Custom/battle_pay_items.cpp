/*
* This file is part of the Pandaria 5.4.8 Project. See THANKS file for Copyright information
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

/*
 * Battle Pay shop items (#963, #969).
 *
 * Port of alexkulya/pandaria_5.4.8 src/server/scripts/Custom/battle_pay_items.cpp
 * (cited per class below as "alex:<line>"). The ScriptNames are the ones in
 * item_script_names (items 110005-110019, 110100, 110101), which the shop sells
 * through battle_pay_product_items. Every item is consumed on a successful use,
 * like the original. Differences from the original are marked "LoA:".
 *
 * battle_pay_pandaria_token_* (alex:272-297) is not ported: it needs the account
 * `vp` balance (Player::AddVirtualPointsCount), which this core does not have,
 * and no item in our DB uses it. Our coin tokens are wow_token_* (wow_token.cpp).
 */

#include "ScriptMgr.h"
#include "Chat.h"
#include "DBCStores.h"
#include "ItemPrototype.h"
#include "Player.h"
#include "ScriptedGossip.h"
#include "ServiceMgr.h"
#include "World.h"
#include <sstream>

namespace
{
    // alex:27-43
    enum BattlePayGold : int64
    {
        BATTLE_PAY_GOLD_1K      = 10000000LL,
        BATTLE_PAY_GOLD_5K      = 50000000LL,
        BATTLE_PAY_GOLD_10K     = 100000000LL,
        BATTLE_PAY_GOLD_30K     = 300000000LL,
        BATTLE_PAY_GOLD_80K     = 800000000LL,
        BATTLE_PAY_GOLD_150K    = 1500000000LL,
    };

    // alex:45-62
    enum BattlePaySkills
    {
        BP_SKILL_FIRST_AID      = 129,
        BP_SKILL_BLACKSMITHING  = 164,
        BP_SKILL_LEATHERWORKING = 165,
        BP_SKILL_ALCHEMY        = 171,
        BP_SKILL_HERBALISM      = 182,
        BP_SKILL_COOKING        = 185,
        BP_SKILL_MINING         = 186,
        BP_SKILL_TAILORING      = 197,
        BP_SKILL_ENGINEERING    = 202,
        BP_SKILL_ENCHANTING     = 333,
        BP_SKILL_FISHING        = 356,
        BP_SKILL_SKINNING       = 393,
        BP_SKILL_JEWELCRAFTING  = 755,
        BP_SKILL_INSCRIPTION    = 773,
        BP_SKILL_ARCHAEOLOGY    = 794
    };

    enum BattlePayMisc
    {
        BP_GOSSIP_MENU_PROFESSIONS      = 51002, // gossip_menu_option rows 0-14, one per skill below
        BP_NPC_TEXT_PROFESSION          = 20010,
        BP_NPC_TEXT_PROFESSION_SMALL    = 20011,
        BP_GOSSIP_ACTION_CLOSE          = 1,
        BP_SPELL_ERROR_STUN             = 27880, // alex uses it on every error path
    };

    struct BattlePayProfession
    {
        uint32 Skill;
        uint32 Spell600;     // the rank spell that raises the skill cap to 600 (alex:64-81)
        uint32 GossipOption; // option id in gossip menu 51002
    };

    // alex:311-340 (gossip rows) + alex:64-81 (600 rank spells)
    BattlePayProfession const BattlePayProfessions[] =
    {
        { BP_SKILL_FIRST_AID,       110406,  0 },
        { BP_SKILL_BLACKSMITHING,   110396,  1 },
        { BP_SKILL_LEATHERWORKING,  110423,  2 },
        { BP_SKILL_ALCHEMY,         105206,  3 },
        { BP_SKILL_HERBALISM,       110413,  4 },
        { BP_SKILL_COOKING,         104381,  5 },
        { BP_SKILL_MINING,          102161,  6 },
        { BP_SKILL_TAILORING,       110426,  7 },
        { BP_SKILL_ENGINEERING,     110403,  8 },
        { BP_SKILL_ENCHANTING,      110400,  9 },
        { BP_SKILL_FISHING,         110410, 10 },
        { BP_SKILL_SKINNING,        102216, 11 },
        { BP_SKILL_JEWELCRAFTING,   110420, 12 },
        { BP_SKILL_INSCRIPTION,     110417, 13 },
        { BP_SKILL_ARCHAEOLOGY,     110393, 14 },
    };

    BattlePayProfession const* GetBattlePayProfession(uint32 skill)
    {
        for (BattlePayProfession const& prof : BattlePayProfessions)
            if (prof.Skill == skill)
                return &prof;
        return nullptr;
    }

    // alex:90-93 and the same check in every class
    bool BattlePayCanUseToken(Player* player)
    {
        if (player->IsInCombat() || player->InArena() || player->InBattleground())
        {
            player->GetSession()->SendNotification("You may not use this token whilst you are in combat or present in an arena or battleground.");
            return false;
        }
        return true;
    }

    void BattlePaySendText(Player* player, char const* text)
    {
        ChatHandler(player->GetSession()).SendSysMessage(text);
    }

    // LoA: the total cap ModifyCurrency would clamp to (Player::GetCurrencyTotalCap is private).
    uint32 BattlePayCurrencyTotalCap(CurrencyTypesEntry const* currency)
    {
        uint32 cap = currency->TotalCap;
        if (currency->ID == CURRENCY_TYPE_HONOR_POINTS && sWorld->getIntConfig(CONFIG_CURRENCY_MAX_HONOR_POINTS))
            cap = sWorld->getIntConfig(CONFIG_CURRENCY_MAX_HONOR_POINTS);
        else if (currency->ID == CURRENCY_TYPE_JUSTICE_POINTS && sWorld->getIntConfig(CONFIG_CURRENCY_MAX_JUSTICE_POINTS))
            cap = sWorld->getIntConfig(CONFIG_CURRENCY_MAX_JUSTICE_POINTS);
        return cap;
    }
}

// alex:83-185 (battle_pay_currency_honor/justice/valor/conquest_1000), merged into one template.
// LoA: all four grant the full named amount (ignore multipliers and the weekly cap, as alex
// already did for justice and valor); the token is refused and kept instead of being
// consumed for a partial grant when the total cap would cut it.
template<uint32 CurrencyId, uint32 Amount>
class battle_pay_currency : public ItemScript
{
public:
    battle_pay_currency(char const* scriptName) : ItemScript(scriptName) { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        if (!BattlePayCanUseToken(player))
            return true;

        if (!player->HasItemCount(item->GetEntry(), 1, true))
        {
            BattlePaySendText(player, "You do not have the necessary token.");
            return true;
        }

        CurrencyTypesEntry const* currency = sCurrencyTypesStore.LookupEntry(CurrencyId);
        if (!currency)
            return true;

        uint32 precision = (currency->Flags & CURRENCY_FLAG_HIGH_PRECISION) ? CURRENCY_PRECISION : 1;
        uint32 count = Amount * precision;

        uint32 totalCap = BattlePayCurrencyTotalCap(currency);
        if (totalCap && player->GetCurrency(CurrencyId, false) + count > totalCap)
        {
            BattlePaySendText(player, "You can not hold that many more points of this currency. The token was not used.");
            return true;
        }

        player->ModifyCurrency(CurrencyId, int32(count), true, true, true);
        player->DestroyItemCount(item->GetEntry(), 1, true);

        std::ostringstream msg;
        msg << "Thanks for supporting the project, you just received " << Amount << " " << currency->Name << ".";
        BattlePaySendText(player, msg.str().c_str());
        player->SaveToDB();
        return true;
    }
};

// alex:187-216
template<int64 Gold>
class battle_pay_gold : public ItemScript
{
public:
    battle_pay_gold(char const* scriptName) : ItemScript(scriptName) { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        if (!BattlePayCanUseToken(player))
            return true;

        if (!player->HasItemCount(item->GetEntry(), 1, true))
        {
            BattlePaySendText(player, "You do not have the necessary token.");
            return true;
        }

        // alex:199 compares against MAX_MONEY_AMOUNT; ModifyMoney does the same check here
        // and refuses (with the client error) instead of clamping.
        if (!player->ModifyMoney(Gold))
        {
            BattlePaySendText(player, "Maximum allowed gold limit exceeded.");
            return true;
        }

        player->DestroyItemCount(item->GetEntry(), 1, true);

        std::ostringstream msg;
        msg << "Thanks for supporting the project, you just received " << Gold / GOLD << " gold.";
        BattlePaySendText(player, msg.str().c_str());
        player->SaveToDB();
        return true;
    }
};

// alex:218-243
// LoA: the level-up goes through the same steps as the .character level command
// (GiveLevel + InitTalentForLevel + reset XP), and is refused above MaxPlayerLevel.
template<uint8 Level>
class battle_pay_level : public ItemScript
{
public:
    battle_pay_level(char const* scriptName) : ItemScript(scriptName) { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        if (!BattlePayCanUseToken(player))
            return true;

        if (Level <= player->GetLevel())
        {
            BattlePaySendText(player, "Your current character level is too high.");
            return true;
        }

        if (Level > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        {
            BattlePaySendText(player, "This level is not available on this realm.");
            return true;
        }

        if (!player->HasItemCount(item->GetEntry(), 1, true))
        {
            BattlePaySendText(player, "You do not have the necessary token.");
            return true;
        }

        player->GiveLevel(Level);
        player->InitTalentForLevel();
        player->SetUInt32Value(PLAYER_FIELD_XP, 0);
        player->DestroyItemCount(item->GetEntry(), 1, true);

        std::ostringstream msg;
        msg << "Thanks for supporting the project, your character is now level " << uint32(Level) << ".";
        BattlePaySendText(player, msg.str().c_str());
        player->SaveToDB();
        return true;
    }
};

// alex:245-270
// Sets the core's own at-login flag; the change is done on the character screen at the
// next login (CharacterHandler rename / customize / race / faction change handlers).
// LoA: only a pending character-service flag blocks a second token (alex tested every
// at-login flag, so a leftover reset-talents/first-login flag also blocked it).
template<AtLoginFlags FlagAtLogin>
class battle_pay_service : public ItemScript
{
public:
    battle_pay_service(char const* scriptName) : ItemScript(scriptName) { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        if (!BattlePayCanUseToken(player))
            return true;

        if (player->HasAtLoginFlag(AtLoginFlags(AT_LOGIN_RENAME | AT_LOGIN_CUSTOMIZE | AT_LOGIN_CHANGE_FACTION | AT_LOGIN_CHANGE_RACE)))
        {
            BattlePaySendText(player, "You have already activated a character service. Log out to the character screen to use it first.");
            return true;
        }

        if (!player->HasItemCount(item->GetEntry(), 1, true))
        {
            BattlePaySendText(player, "You do not have the necessary token.");
            return true;
        }

        player->SetAtLoginFlag(FlagAtLogin);
        player->DestroyItemCount(item->GetEntry(), 1, true);
        BattlePaySendText(player, "Thanks for supporting the project, you have just activated the function to change your character. Please log out to the character screen to use it.");
        player->SaveToDB();
        return true;
    }
};

// alex:299-580 (battle_pay_boost_profession and battle_pay_boost_profession_small)
// Full: learns the 600 rank of a known profession (or sets fishing to 600) and fills the skill.
// Small: fills a known profession up to its current cap.
template<bool Full>
class battle_pay_boost_profession_base : public ItemScript
{
public:
    battle_pay_boost_profession_base(char const* scriptName) : ItemScript(scriptName) { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        ClearGossipMenuFor(player);
        player->SaveToDB();

        for (BattlePayProfession const& prof : BattlePayProfessions)
            if (player->HasSkill(prof.Skill))
                player->ADD_GOSSIP_ITEM_DB(BP_GOSSIP_MENU_PROFESSIONS, prof.GossipOption, GOSSIP_SENDER_MAIN, prof.Skill);

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "|TInterface/PaperDollInfoFrame/UI-GearManager-LeaveItem-Transparent:28|t Close", GOSSIP_SENDER_MAIN, BP_GOSSIP_ACTION_CLOSE);
        SendGossipMenuFor(player, Full ? BP_NPC_TEXT_PROFESSION : BP_NPC_TEXT_PROFESSION_SMALL, item->GetGUID());
        return true;
    }

    void OnGossipSelect(Player* player, Item* item, uint32 /*sender*/, uint32 action) override
    {
        ClearGossipMenuFor(player);

        if (action == BP_GOSSIP_ACTION_CLOSE)
        {
            CloseGossipMenuFor(player);
            return;
        }

        BattlePayProfession const* prof = GetBattlePayProfession(action);
        if (!prof || !player->HasSkill(prof->Skill) || !player->HasItemCount(item->GetEntry(), 1, true))
        {
            BattlePaySendText(player, "An error has occurred.");
            player->CastSpell(player, BP_SPELL_ERROR_STUN, true);
            CloseGossipMenuFor(player);
            return;
        }

        uint32 newValue = 0;
        if (Full)
        {
            // alex:359-464: only when the 600 rank is not known yet; fishing is set straight to 600.
            if (player->HasSpell(prof->Spell600))
            {
                BattlePaySendText(player, "An error has occurred.");
                player->CastSpell(player, BP_SPELL_ERROR_STUN, true);
                CloseGossipMenuFor(player);
                return;
            }

            player->LearnSpell(prof->Spell600, false);

            if (prof->Skill == BP_SKILL_FISHING)
            {
                player->SetSkill(prof->Skill, player->GetSkillStep(prof->Skill), 600, 600);
                newValue = 600;
            }
        }

        if (!newValue)
        {
            // alex:466-471 / alex:559-562
            uint16 maxValue = player->GetMaxSkillValue(prof->Skill);
            if (player->GetSkillValue(prof->Skill) < maxValue)
                player->SetSkill(prof->Skill, player->GetSkillStep(prof->Skill), maxValue, maxValue);
            newValue = maxValue;
        }

        player->DestroyItemCount(item->GetEntry(), 1, true);
        BattlePaySendText(player, "Thanks for supporting the project!");

        std::ostringstream infoSkill;
        infoSkill << "Skill: " << prof->Skill << " Value: " << newValue;
        sServiceMgr->ExecutedServices(player->GetGUID().GetCounter(), Full ? SERVICE_TYPE_BOOST_PROFESSION : SERVICE_TYPE_BOOST_PROFESSION_SMALL,
            std::string("Boosted name: ") + player->GetName(), infoSkill.str());

        CloseGossipMenuFor(player);
        player->SaveToDB();
    }
};

// alex:582-605
void AddSC_battle_pay_items()
{
    new battle_pay_currency<CURRENCY_TYPE_HONOR_POINTS, 1000>("battle_pay_currency_honor_1000");
    new battle_pay_currency<CURRENCY_TYPE_JUSTICE_POINTS, 1000>("battle_pay_currency_justice_1000");
    new battle_pay_currency<CURRENCY_TYPE_VALOR_POINTS, 1000>("battle_pay_currency_valor_1000");
    new battle_pay_currency<CURRENCY_TYPE_CONQUEST_POINTS, 1000>("battle_pay_currency_conquest_1000");
    new battle_pay_gold<BATTLE_PAY_GOLD_1K>("battle_pay_gold_1k");
    new battle_pay_gold<BATTLE_PAY_GOLD_5K>("battle_pay_gold_5k");
    new battle_pay_gold<BATTLE_PAY_GOLD_10K>("battle_pay_gold_10k");
    new battle_pay_gold<BATTLE_PAY_GOLD_30K>("battle_pay_gold_30k");
    new battle_pay_gold<BATTLE_PAY_GOLD_80K>("battle_pay_gold_80k");
    new battle_pay_gold<BATTLE_PAY_GOLD_150K>("battle_pay_gold_150k");
    new battle_pay_level<90>("battle_pay_service_level_90");
    new battle_pay_service<AT_LOGIN_RENAME>("battle_pay_service_rename");
    new battle_pay_service<AT_LOGIN_CHANGE_FACTION>("battle_pay_service_change_faction");
    new battle_pay_service<AT_LOGIN_CHANGE_RACE>("battle_pay_service_change_race");
    new battle_pay_service<AT_LOGIN_CUSTOMIZE>("battle_pay_service_customize");
    new battle_pay_boost_profession_base<true>("battle_pay_boost_profession");
    new battle_pay_boost_profession_base<false>("battle_pay_boost_profession_small");
}
