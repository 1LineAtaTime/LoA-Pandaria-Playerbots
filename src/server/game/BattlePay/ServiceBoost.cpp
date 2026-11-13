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

#include "ServiceBoost.h"
#include "WorldSession.h"
#include "BattlePayMgr.h"
#include "Player.h"
#include "ServiceMgr.h"
#include "Realm.h"
#include "BattlePayBoostQueries.h"

void LoadBoostItems()
{
    uint32 oldMSTime = getMSTime();

    mBoostItemsMap.clear();                                // need for reload case

    //                                                0                 1       2
    QueryResult result = WorldDatabase.Query("SELECT specialization, slot - 1, itemId FROM battle_pay_boost_items ORDER BY specialization, slot ASC");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 boost items. DB table `battle_pay_boost_items` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        BoostItems* items = new BoostItems();

        items->spec = fields[0].GetUInt32();
        items->slot = fields[1].GetUInt32();
        items->itemId = fields[2].GetUInt32();

        if (!sObjectMgr->GetItemTemplate(items->itemId))
        {
            TC_LOG_ERROR("sql.sql", "Item %u specified in `battle_pay_boost_items` does not exist, skipped.", items->itemId);
            continue;
        }

        mBoostItemsMap.push_back(items);
        count++;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u boost items in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

CharacterBooster::CharacterBooster(WorldSession* session) : m_session(session), m_timer(0), m_boosting(false), m_sendPacket(false) { }

void SetBoosting(WorldSession* session, uint32 accountId, bool boost)
{
    if (!accountId && session)
        accountId = session->GetAccountId();
    if (!accountId)
        return;
    if (boost)
        LoginDatabase.DirectPExecute("INSERT INTO account_boost(id,realmid,counter) VALUES(%u,%u,1) ON DUPLICATE KEY UPDATE counter=IF(counter<4294967295,counter+1,counter)", accountId, realm.Id.Realm);
    else
        LoginDatabase.DirectPExecute("UPDATE account_boost SET counter=counter-1 WHERE id=%u AND realmid=%u AND counter>0", accountId, realm.Id.Realm);
    // Zero rows are retained so a concurrent grant/reservation cannot be lost
    // to DELETE/REPLACE. Session flags are only a view of durable entitlements.
    QueryResult remaining = LoginDatabase.PQuery("SELECT COUNT(*) FROM account_boost WHERE id=%u AND realmid IN (-1,%u) AND counter>0", accountId, realm.Id.Realm);
    if (session && remaining)
        session->SetBoost(remaining->Fetch()[0].GetUInt64() != 0);
}

void CharacterBooster::_GetCharBoostItems(PreparedItemsMap& itemsToMail, PreparedItemsMap& itemsToEquip) const
{
    switch (m_charBoostInfo.specialization)
    {
        case SPEC_MAGE_ARCANE:
        case SPEC_MAGE_FIRE:
        case SPEC_MAGE_FROST:
        case SPEC_PRIEST_DISCIPLINE:
        case SPEC_PRIEST_HOLY:
        case SPEC_PRIEST_SHADOW:
        case SPEC_WARLOCK_AFFLICTION:
        case SPEC_WARLOCK_DEMONOLOGY:
        case SPEC_WARLOCK_DESTRUCTION:
            itemsToMail.emplace(0, 82590);
            break;
    }  

    if(m_charBoostInfo.allianceFaction)
        itemsToMail.emplace(1, 25472);
    else
        itemsToMail.emplace(1, 25474);

    for (auto&& item : mBoostItemsMap)
        if (item->spec == m_charBoostInfo.specialization)
            itemsToEquip.emplace(item->slot, item->itemId);
}

void CharacterBooster::SendCharBoostPacket(PreparedItemsMap items) const
{
    ObjectGuid guid = m_charBoostInfo.charGuid;
    WorldPacket data(SMSG_CHARACTER_UPGRADE_COMPLETE, 8 + 3 + items.size());

    data.WriteBit(guid[2]);
    data.WriteBit(guid[0]);
    data.WriteBit(guid[7]);
    data.WriteBit(guid[5]);
    data.WriteBit(guid[3]);
    data.WriteBit(guid[4]);
    data.WriteBit(guid[1]);
    data.WriteBits(items.size(), 22);
    data.WriteBit(guid[6]);

    data.FlushBits();

    data.WriteByteSeq(guid[7]);
    data.WriteByteSeq(guid[2]);
    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid[5]);

    for (auto&& item : items)
        data << uint32(item.second);

    data.WriteByteSeq(guid[0]);
    data.WriteByteSeq(guid[1]);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[4]);


    GetSession()->SendPacket(&data);
}

void CharacterBooster::LearnNonExistedSpell(CharacterDatabaseTransaction trans, uint32 spell) const
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_SPELL);
    stmt->setUInt32(0, m_charBoostInfo.charGuid.GetCounter());
    stmt->setUInt32(1, spell);
    if (!CharacterDatabase.Query(stmt))
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHAR_SPELL);
        stmt->setUInt32(0, m_charBoostInfo.charGuid.GetCounter());
        stmt->setUInt32(1, spell);
        stmt->setUInt32(2, 1);
        stmt->setUInt32(3, 0);
        trans->Append(stmt);
    }
}

void CharacterBooster::LearnNonExistedSkill(CharacterDatabaseTransaction trans, uint32 skill) const
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_SKILL_BOOST);
    stmt->setUInt32(0, m_charBoostInfo.charGuid.GetCounter());
    stmt->setUInt32(1, skill);

    if (!CharacterDatabase.Query(stmt))
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHAR_SKILLS);
        stmt->setUInt32(0, m_charBoostInfo.charGuid.GetCounter());
        stmt->setUInt32(1, skill);
        stmt->setUInt32(2, 600);
        stmt->setUInt32(3, 600);
        trans->Append(stmt);
    }
}

uint32 CharacterBooster::_PrepareMail(CharacterDatabaseTransaction trans, std::string const subject, std::string const body) const
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_MAIL);
    uint32 mailId = sObjectMgr->GenerateMailID();

    stmt->setUInt32(0, mailId);
    stmt->setUInt8(1, MAIL_NORMAL);
    stmt->setInt8(2, MAIL_STATIONERY_DEFAULT);
    stmt->setUInt16(3, 0);
    stmt->setUInt32(4, m_charBoostInfo.charGuid.GetCounter());
    stmt->setUInt32(5, m_charBoostInfo.charGuid.GetCounter());
    stmt->setString(6, subject);
    stmt->setString(7, body);
    stmt->setBool(8, true);
    stmt->setUInt64(9, time(NULL) + 180 * DAY);
    stmt->setUInt64(10, time(NULL));
    stmt->setUInt32(11, 0);
    stmt->setUInt32(12, 0);
    stmt->setUInt8(13, 0);
    trans->Append(stmt);

    return mailId;
}

void CharacterBooster::_SendMail(CharacterDatabaseTransaction trans, PreparedItemsMap items) const
{
    if (items.empty())
        return;

    MailTemplateEntry const* mailTemplateEntry = sMailTemplateStore.LookupEntry(MAIL_CHARRACTER_BOOST_EQUIPED_ITEMS_BODY);
    if (!mailTemplateEntry) // should never happen
        return;

    uint32 mailId = _PrepareMail(trans, mailTemplateEntry->subject, mailTemplateEntry->content);
    CharacterDatabasePreparedStatement* stmt = NULL;

    for (auto&& itr : items)
    {
        if (std::unique_ptr<Item> item{Item::CreateItem(itr.second, 1, nullptr)})
        {
            item->SetOwnerGUID(m_charBoostInfo.charGuid);
            item->SaveToDB(trans);

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_MAIL_ITEM);
            stmt->setUInt32(0, mailId);
            stmt->setUInt32(1, item->GetGUID().GetCounter());
            stmt->setUInt32(2, m_charBoostInfo.charGuid.GetCounter());
            trans->Append(stmt);
        }
        else
        {
            m_buildFailed = true;
            TC_LOG_ERROR("sql.sql", "Can't create item %u for _SendMail in Boost. Skip.", itr.second);
        }
    }
}

void CharacterBooster::_PrepareInventory(CharacterDatabaseTransaction trans) const
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_INVENTORY_BOOST);
    stmt->setUInt32(0, m_charBoostInfo.charGuid.GetCounter());
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    MailTemplateEntry const* mailTemplateEntry = sMailTemplateStore.LookupEntry(MAIL_CHARRACTER_BOOST_EQUIPED_ITEMS_BODY);
    if (!mailTemplateEntry)
    {
        m_buildFailed = true;
        return;
    }

    QueryResult inventoryCount = CharacterDatabase.PQuery("SELECT COUNT(*) FROM character_inventory inv1 LEFT JOIN character_inventory inv2 ON inv1.bag=inv2.item LEFT JOIN item_instance i ON inv1.item=i.guid WHERE inv1.slot<39 AND inv1.guid=%u AND (inv2.slot<39 OR inv1.bag=0) AND i.itemEntry!=6948", m_charBoostInfo.charGuid.GetCounter());
    if (!inventoryCount || (!result && inventoryCount->Fetch()[0].GetUInt64()))
    {
        m_buildFailed = true;
        return;
    }
    if (result)
    {
    uint32 mailId = _PrepareMail(trans, mailTemplateEntry->subject, mailTemplateEntry->content);
    uint32 itemCount = 0;
    do
    {
        if (itemCount > 11)
        {
            itemCount = 0;
            mailId = _PrepareMail(trans, mailTemplateEntry->subject, mailTemplateEntry->content);
        }

        uint32 itemGuid = (*result)[0].GetUInt32();

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_MAIL_ITEM);
        stmt->setUInt32(0, mailId);
        stmt->setUInt32(1, itemGuid);
        stmt->setUInt32(2, m_charBoostInfo.charGuid.GetCounter());
        trans->Append(stmt);

        itemCount++;
    } while (result->NextRow());
    }

    // unequip after sending
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_INVENTORY_BOOST);
    stmt->setUInt32(0, m_charBoostInfo.charGuid.GetCounter());
    trans->Append(stmt);

    // move or create hearthstone to first slot
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_HEARTHSTONE_BOOST);
    stmt->setUInt32(0, m_charBoostInfo.charGuid.GetCounter());
    PreparedQueryResult hearthstone = CharacterDatabase.Query(stmt);
    QueryResult hearthstoneCount = CharacterDatabase.PQuery("SELECT COUNT(*) FROM item_instance WHERE owner_guid=%u AND itemEntry=6948", m_charBoostInfo.charGuid.GetCounter());
    if (!hearthstoneCount || (!hearthstone && hearthstoneCount->Fetch()[0].GetUInt64()))
    {
        m_buildFailed = true;
        return;
    }
    if (!hearthstone)
    {
        if (std::unique_ptr<Item> item{Item::CreateItem(ITEM_HEARTHSTONE, 1, nullptr)})
        {
            item->SetOwnerGUID(m_charBoostInfo.charGuid);
            item->SaveToDB(trans);

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHAR_INVENTORY);
            stmt->setUInt32(0, m_charBoostInfo.charGuid.GetCounter());
            stmt->setUInt32(1, 0);
            stmt->setUInt8(2, INVENTORY_SLOT_ITEM_START);
            stmt->setUInt32(3, item->GetGUID().GetCounter());
            trans->Append(stmt);
        }
        else
            m_buildFailed = true;
    }
    else
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_HEARTHSTONE_BOOST);
        stmt->setUInt32(0, m_charBoostInfo.charGuid.GetCounter());
        trans->Append(stmt);
    }

    // insert food to second slot
    if (std::unique_ptr<Item> item{Item::CreateItem(ITEM_LEMON_FLAVOUR_PUDING, 20, nullptr)})
    {
        item->SetOwnerGUID(m_charBoostInfo.charGuid);
        item->SaveToDB(trans);

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHAR_INVENTORY);
        stmt->setUInt32(0, m_charBoostInfo.charGuid.GetCounter());
        stmt->setUInt32(1, 0);
        stmt->setUInt8(2, INVENTORY_SLOT_ITEM_START + 1);
        stmt->setUInt32(3, item->GetGUID().GetCounter());
        trans->Append(stmt);
    }
    else
        m_buildFailed = true;

    // insert bag in inventory slots
    uint8 slot = INVENTORY_SLOT_BAG_START;
    for (uint8 i = 0; i < 4; i++)
    {
        if (std::unique_ptr<Item> item{Item::CreateItem(ITEM_EMBERSILK_BAG, 1, nullptr)})
        {
            item->SetOwnerGUID(m_charBoostInfo.charGuid);
            item->SaveToDB(trans);

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHAR_INVENTORY);
            stmt->setUInt32(0, m_charBoostInfo.charGuid.GetCounter());
            stmt->setUInt32(1, 0);
            stmt->setUInt8(2, slot);
            stmt->setUInt32(3, item->GetGUID().GetCounter());
            trans->Append(stmt);
        }
        else
            m_buildFailed = true;
        slot++;
    }
}

std::string CharacterBooster::_SetSpecialization(CharacterDatabaseTransaction trans, uint8 const classId) const
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_TALENT);
    stmt->setUInt32(0, m_charBoostInfo.charGuid.GetCounter());
    PreparedQueryResult result = CharacterDatabase.Query(stmt);
    QueryResult talentCount = CharacterDatabase.PQuery("SELECT COUNT(*) FROM character_talent WHERE guid=%u", m_charBoostInfo.charGuid.GetCounter());
    if (!talentCount || (!result && talentCount->Fetch()[0].GetUInt64()))
    {
        m_buildFailed = true;
        return {};
    }
    if (result)
    {
        do
        {
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_SPELL_BY_SPELL);
            stmt->setUInt32(0, (*result)[0].GetUInt32());
            stmt->setUInt32(1, m_charBoostInfo.charGuid.GetCounter());
            trans->Append(stmt);
        } while (result->NextRow());
    }

    for (auto&& spec : dbc::GetClassSpecializations(classId))
    {
        
        if (std::vector<SpecializationSpellsEntry const*> const* specSpells = sDBCManager.GetSpecializationSpells(spec))
        {
            for (size_t j = 0; j < specSpells->size(); ++j)
            {
                SpecializationSpellsEntry const* specSpell = specSpells->at(j);
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_SPELL_BY_SPELL);
                stmt->setUInt32(0, specSpell->SpellID);
                stmt->setUInt32(1, m_charBoostInfo.charGuid.GetCounter());
                trans->Append(stmt);
            }
        }
        
    }

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_TALENT);
    stmt->setUInt32(0, m_charBoostInfo.charGuid.GetCounter());
    trans->Append(stmt);

    std::ostringstream talentTree;
    talentTree << m_charBoostInfo.specialization << " 0 ";
    return talentTree.str();
}

void CharacterBooster::_LearnSpells(CharacterDatabaseTransaction trans) const
{
    std::vector<uint32> spellsToLearn = { SPELL_ARTISAN_RIDING, SPELL_COLD_WHEATHER_FLYING, SPELL_FLIGHT_MASTER_LICENSE, SPELL_WISDOM_OF_FOUR_WINDS };
    spellsToLearn.push_back(m_charBoostInfo.allianceFaction ? SPELL_SWIFT_PURPLE_GRYPGON : SPELL_SWIFT_PURPLE_WIND_RIDER);

    for (auto&& spell : spellsToLearn)
        LearnNonExistedSpell(trans, spell);
}

void CharacterBooster::_GetBoostedCharacterData(uint8& raceId, uint8& classId, uint8& level) const
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_RACE);
    stmt->setUInt32(0, m_charBoostInfo.charGuid.GetCounter());

    if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
    {
        raceId = (*result)[0].GetUInt8();
        if (raceId == RACE_PANDAREN_NEUTRAL)
            raceId = m_charBoostInfo.allianceFaction ? RACE_PANDAREN_ALLIANCE : RACE_PANDAREN_HORDE;
    }

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_CLASS);
    stmt->setUInt32(0, m_charBoostInfo.charGuid.GetCounter());

    if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
        classId = (*result)[0].GetUInt8();

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_LEVEL);
    stmt->setUInt32(0, m_charBoostInfo.charGuid.GetCounter());

    if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
        level = (*result)[0].GetUInt8();
}

std::string CharacterBooster::_EquipItems(CharacterDatabaseTransaction trans, PreparedItemsMap itemsToEquip) const
{
    std::ostringstream items;
    CharacterDatabasePreparedStatement* stmt;
    for (uint32 i = 0; i < EQUIPMENT_SLOT_END; ++i)
    {
        auto itr = itemsToEquip.find(i);
        if (itr != itemsToEquip.end())
        {
            if (std::unique_ptr<Item> item{Item::CreateItem(itr->second, 1, nullptr)})
            {
                item->SetOwnerGUID(m_charBoostInfo.charGuid);
                item->SaveToDB(trans);

                stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHAR_INVENTORY);
                stmt->setUInt32(0, m_charBoostInfo.charGuid.GetCounter());
                stmt->setUInt32(1, 0);
                stmt->setUInt8(2, itr->first);
                stmt->setUInt32(3, item->GetGUID().GetCounter());
                trans->Append(stmt);

                items << (itr->second) << " 0 ";
            }
            else
            {
                m_buildFailed = true;
                TC_LOG_ERROR("sql.sql", "Can't create item %u for _EquipItems in Boost. Skip.", itr->second);

                items << "0 0 ";
            }
        }
        else
            items << "0 0 ";
    }

    for (uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        items << ITEM_EMBERSILK_BAG << " 0 ";

    return items.str();
}

void CharacterBooster::_SaveBoostedChar(CharacterDatabaseTransaction trans, std::string items, uint8 const raceId, uint8 const classId) const
{
    float const* position = m_charBoostInfo.allianceFaction ? startPosition[1] : startPosition[0];

    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHARACTER_FOR_BOOST);
    stmt->setUInt8(0, raceId);
    stmt->setFloat(1, position[0]);
    stmt->setFloat(2, position[1]);
    stmt->setFloat(3, position[2]);
    stmt->setFloat(4, position[3]);
    stmt->setUInt16(5, MAP_VALE_OF_ETERNAL_BLOSSOMS);
    stmt->setString(6, _SetSpecialization(trans, classId));
    stmt->setUInt16(7, AT_LOGIN_FIRST);
    stmt->setString(8, items);
    stmt->setUInt32(9, m_charBoostInfo.charGuid.GetCounter());
    trans->Append(stmt);
}

void CharacterBooster::_LearnVeteranBonuses(CharacterDatabaseTransaction trans, uint8 const classId) const
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_SKILLS_BOOST);
    stmt->setUInt32(0, m_charBoostInfo.charGuid.GetCounter());

    std::vector<uint32> primarySkills;
    bool fistAidBoosted = false;
    bool cookingBoosted = false;
    bool fishingBoosted = false;
    bool archaeologyBoosted = false;
    PreparedQueryResult result = CharacterDatabase.Query(stmt);
    QueryResult skillCount = CharacterDatabase.PQuery("SELECT COUNT(*) FROM character_skills WHERE guid=%u", m_charBoostInfo.charGuid.GetCounter());
    if (!skillCount || (!result && skillCount->Fetch()[0].GetUInt64()))
    {
        m_buildFailed = true;
        return;
    }
    if (result)
    {
        do
        {
            uint32 skillId = (*result)[0].GetUInt16();
            if (skillId == SKILL_BLACKSMITHING || skillId == SKILL_LEATHERWORKING || skillId == SKILL_ALCHEMY || skillId == SKILL_HERBALISM ||
                skillId == SKILL_MINING || skillId == SKILL_TAILORING || skillId == SKILL_ENGINEERING || skillId == SKILL_ENCHANTING ||
                skillId == SKILL_SKINNING || skillId == SKILL_JEWELCRAFTING || skillId == SKILL_INSCRIPTION)
                primarySkills.push_back(skillId);
            if (skillId == SKILL_FIRST_AID)
                fistAidBoosted = true;
            if (skillId == SKILL_COOKING)
                cookingBoosted = true;
            if (skillId == SKILL_FISHING)
                fishingBoosted = true;
            if (skillId == SKILL_ARCHAEOLOGY)
                archaeologyBoosted = true;
        } while (result->NextRow());
    }

    for (auto&& skill : primarySkills)
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_UDP_CHAR_SKILLS);
        stmt->setUInt32(0, 600);
        stmt->setUInt32(1, 600);
        stmt->setUInt32(2, m_charBoostInfo.charGuid.GetCounter());
        stmt->setUInt32(3, skill);
        trans->Append(stmt);

        uint32 spell = 0;
        switch (skill)
        {
            case SKILL_BLACKSMITHING:  spell = SPELL_BLACKSMITHING;  break;
            case SKILL_LEATHERWORKING: spell = SPELL_LEATHERWORKING; break;
            case SKILL_ALCHEMY:        spell = SPELL_ALCHEMY;        break;
            case SKILL_HERBALISM:      spell = SPELL_HERBALISM;      break;
            case SKILL_MINING:         spell = SPELL_MINING;         break;
            case SKILL_TAILORING:      spell = SPELL_TAILORING;      break;
            case SKILL_ENGINEERING:    spell = SPELL_ENGINEERING;    break;
            case SKILL_ENCHANTING:     spell = SPELL_ENCHANTING;     break;
            case SKILL_SKINNING:       spell = SPELL_SKINNING;       break;
            case SKILL_JEWELCRAFTING:  spell = SPELL_JEWELCRAFTING;  break;
            case SKILL_INSCRIPTION:    spell = SPELL_INSCRIPTION;    break;
        }
        LearnNonExistedSpell(trans, spell);
    }

    if (fistAidBoosted)
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_UDP_CHAR_SKILLS);
        stmt->setUInt32(0, 600);
        stmt->setUInt32(1, 600);
        stmt->setUInt32(2, m_charBoostInfo.charGuid.GetCounter());
        stmt->setUInt32(3, SKILL_FIRST_AID);
        trans->Append(stmt);
        LearnNonExistedSpell(trans, SPELL_FIRST_AID);
    }

    if (cookingBoosted)
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_UDP_CHAR_SKILLS);
        stmt->setUInt32(0, 600);
        stmt->setUInt32(1, 600);
        stmt->setUInt32(2, m_charBoostInfo.charGuid.GetCounter());
        stmt->setUInt32(3, SKILL_COOKING);
        trans->Append(stmt);
        LearnNonExistedSpell(trans, SPELL_COOKING);
    }

    if (fishingBoosted)
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_UDP_CHAR_SKILLS);
        stmt->setUInt32(0, 600);
        stmt->setUInt32(1, 600);
        stmt->setUInt32(2, m_charBoostInfo.charGuid.GetCounter());
        stmt->setUInt32(3, SKILL_FISHING);
        trans->Append(stmt);
        LearnNonExistedSpell(trans, SPELL_FISHING);
    }

    if (archaeologyBoosted)
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_UDP_CHAR_SKILLS);
        stmt->setUInt32(0, 600);
        stmt->setUInt32(1, 600);
        stmt->setUInt32(2, m_charBoostInfo.charGuid.GetCounter());
        stmt->setUInt32(3, SKILL_ARCHAEOLOGY);
        trans->Append(stmt);
        LearnNonExistedSpell(trans, SPELL_ARCHAEOLOGY);
    }

    if (primarySkills.empty())
    {
        std::vector<std::pair<uint32, uint32>> skillsAndSpells;
        switch (classId)
        {
            case CLASS_PRIEST:
            case CLASS_MAGE:
            case CLASS_WARLOCK:
                skillsAndSpells.emplace_back(SKILL_ENCHANTING, SPELL_ENCHANTING);
                skillsAndSpells.emplace_back(SKILL_TAILORING, SPELL_TAILORING);
                break;
            case CLASS_ROGUE:
            case CLASS_MONK:
            case CLASS_DRUID:
            case CLASS_HUNTER:
            case CLASS_SHAMAN:
                skillsAndSpells.emplace_back(SKILL_LEATHERWORKING, SPELL_LEATHERWORKING);
                skillsAndSpells.emplace_back(SKILL_SKINNING, SPELL_SKINNING);
                break;
            case CLASS_WARRIOR:
            case CLASS_PALADIN:
            case CLASS_DEATH_KNIGHT:
                skillsAndSpells.emplace_back(SKILL_BLACKSMITHING, SPELL_BLACKSMITHING);
                skillsAndSpells.emplace_back(SKILL_MINING, SPELL_MINING);
                break;
        }
        for (auto&& itr : skillsAndSpells)
        {
            LearnNonExistedSkill(trans, itr.first);
            LearnNonExistedSpell(trans, itr.second);
        }
    }

    if (!fistAidBoosted)
    {
        LearnNonExistedSkill(trans, SKILL_FIRST_AID);
        LearnNonExistedSpell(trans, SPELL_FIRST_AID);
    }

    if (!cookingBoosted)
    {
        LearnNonExistedSkill(trans, SKILL_COOKING);
        LearnNonExistedSpell(trans, SPELL_COOKING);
    }

    if (!fishingBoosted)
    {
        LearnNonExistedSkill(trans, SKILL_FISHING);
        LearnNonExistedSpell(trans, SPELL_FISHING);
    }

    if (!archaeologyBoosted)
    {
        LearnNonExistedSkill(trans, SKILL_ARCHAEOLOGY);
        LearnNonExistedSpell(trans, SPELL_ARCHAEOLOGY);
    }
}

bool CharacterBooster::_HandleCharacterBoost()
{
    if (!m_useId || GetSession()->GetPlayer() || GetSession()->PlayerLoading())
        return false;
    uint32 const account = GetSession()->GetAccountId();
    uint32 const target = m_charBoostInfo.charGuid.GetCounter();
    QueryResult use = LoginDatabase.PQuery("SELECT state FROM battlepay_boost_use WHERE id=%llu AND account_id=%u AND realm_id=%u AND character_guid=%u", m_useId, account, realm.Id.Realm, target);
    if (!use)
        return false;
    if (use->Fetch()[0].GetUInt8() == 2)
    {
        PreparedItemsMap itemsToMail, itemsToEquip;
        _GetCharBoostItems(itemsToMail, itemsToEquip);
        SendCharBoostPacket(itemsToEquip);
        return true;
    }
    if (use->Fetch()[0].GetUInt8() != 1)
        return false;

    QueryResult receipt = CharacterDatabase.PQuery("SELECT state FROM battlepay_boost_delivery WHERE use_id=%llu AND account_id=%u AND character_guid=%u", m_useId, account, target);
    if (!receipt)
    {
        QueryResult owner = CharacterDatabase.PQuery("SELECT COUNT(*) FROM characters WHERE guid=%u AND account=%u AND level<90", target, account);
        if (!owner)
            return false;
        if (owner->Fetch()[0].GetUInt64() != 1)
        {
            CharacterDatabase.DirectPExecute("INSERT IGNORE INTO battlepay_boost_delivery(use_id,account_id,character_guid,state) VALUES(%llu,%u,%u,0)", m_useId, account, target);
            receipt = CharacterDatabase.PQuery("SELECT state FROM battlepay_boost_delivery WHERE use_id=%llu", m_useId);
        }
    }
    if (receipt && receipt->Fetch()[0].GetUInt8() == 0)
    {
        // The durable character-side fence makes restoration safe even when a
        // second process was trying to apply this boost concurrently.
        LoginDatabase.DirectPExecute(BattlePayBoostQueries::Restore, m_useId, account, realm.Id.Realm);
        QueryResult restored = LoginDatabase.PQuery("SELECT state FROM battlepay_boost_use WHERE id=%llu AND account_id=%u", m_useId, account);
        if (restored && restored->Fetch()[0].GetUInt8() == 3)
        {
            m_useId = 0;
            m_boosting = false;
            m_charBoostInfo = CharacterBoostData();
            QueryResult remaining = LoginDatabase.PQuery("SELECT COUNT(*) FROM account_boost WHERE id=%u AND realmid IN (-1,%u) AND counter>0", account, realm.Id.Realm);
            if (remaining)
                GetSession()->SetBoost(remaining->Fetch()[0].GetUInt64() != 0);
        }
        return false;
    }

    if (!receipt)
    {

    uint8 raceId = 0, classId = 0, level = 0;
    _GetBoostedCharacterData(raceId, classId, level);
    ChrSpecializationEntry const* specEntry = sChrSpecializationStore.LookupEntry(m_charBoostInfo.specialization);
    if (!raceId || raceId >= MAX_RACES || !classId || !level || !specEntry || classId != specEntry->classId)
        return false;

    PreparedItemsMap itemsToMail, itemsToEquip;
    _GetCharBoostItems(itemsToMail, itemsToEquip);
    if (itemsToEquip.empty() || !sMailTemplateStore.LookupEntry(MAIL_CHARRACTER_BOOST_EQUIPED_ITEMS_BODY))
        return false;
    for (auto const& item : itemsToEquip)
        if (!sObjectMgr->GetItemTemplate(item.second))
            return false;
    for (auto const& item : itemsToMail)
        if (!sObjectMgr->GetItemTemplate(item.second))
            return false;
    for (uint32 item : { uint32(ITEM_HEARTHSTONE), uint32(ITEM_LEMON_FLAVOUR_PUDING), uint32(ITEM_EMBERSILK_BAG) })
        if (!sObjectMgr->GetItemTemplate(item))
            return false;

    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
    std::string claim = Trinity::StringFormat("INSERT INTO battlepay_boost_delivery(use_id,account_id,character_guid,state) SELECT %llu,%u,guid,1 FROM characters WHERE guid=%u AND account=%u AND level<90 AND online=0", m_useId, account, target, account);
    trans->AppendChecked(claim.c_str(), 1);
    m_buildFailed = false;
    _PrepareInventory(trans);
    _SendMail(trans, itemsToMail);
    _LearnSpells(trans);
    _SaveBoostedChar(trans, _EquipItems(trans, itemsToEquip), raceId, classId);
    if (level >= 60)
        _LearnVeteranBonuses(trans, classId);
    if (raceId == RACE_GOBLIN || raceId == RACE_WORGEN || raceId == RACE_PANDAREN_ALLIANCE || raceId == RACE_PANDAREN_HORDE || classId == CLASS_DEATH_KNIGHT)
        sServiceMgr->AddSpecificPlayerData(target, 0, raceId, classId, nullptr, true, false, trans);
    if (m_buildFailed)
        return false;
    if (sWorld->getBoolConfig(CONFIG_EXECUTED_SERVICES_LOG))
        trans->PAppend("INSERT INTO executed_services(type,guid,old_data,new_data,execute_date) VALUES(%u,%u,'BattlePay boost','','%s')", uint32(SERVICE_TYPE_BOOST), target, TimeToTimestampStr(time(nullptr)));
    CharacterDatabase.AsyncCommitTransaction(trans).m_future.get();
    receipt = CharacterDatabase.PQuery("SELECT state FROM battlepay_boost_delivery WHERE use_id=%llu AND account_id=%u AND character_guid=%u", m_useId, account, target);
    if (!receipt || receipt->Fetch()[0].GetUInt8() != 1)
        return false;
    }

    LoginDatabase.DirectPExecute("UPDATE battlepay_boost_use SET state=2 WHERE id=%llu AND account_id=%u AND realm_id=%u AND state=1", m_useId, account, realm.Id.Realm);
    QueryResult completed = LoginDatabase.PQuery("SELECT state FROM battlepay_boost_use WHERE id=%llu AND account_id=%u", m_useId, account);
    if (!completed || completed->Fetch()[0].GetUInt8() != 2)
        return false;
    PreparedItemsMap itemsToMail, itemsToEquip;
    _GetCharBoostItems(itemsToMail, itemsToEquip);
    SendCharBoostPacket(itemsToEquip);
    QueryResult remaining = LoginDatabase.PQuery("SELECT COUNT(*) FROM account_boost WHERE id=%u AND realmid IN (-1,%u) AND counter>0", account, realm.Id.Realm);
    if (remaining)
        GetSession()->SetBoost(remaining->Fetch()[0].GetUInt64() != 0);
    return true;
}

void CharacterBooster::HandleCharacterBoost()
{
    if (!m_charBoostInfo.charGuid)
        return;

    switch (m_charBoostInfo.action)
    {
        case CHARACTER_BOOST_ITEMS:
            sBattlePayMgr->SendBattlePayDistributionUpdate(GetSession(), BATTLE_PAY_SERVICE_BOOST, m_charBoostInfo.action);
            m_charBoostInfo.action = CHARACTER_BOOST_APPLIED;
            m_timer = 500;
            m_sendPacket = true;
            break;
        case CHARACTER_BOOST_APPLIED:
            if (!m_completed)
                break;
            sBattlePayMgr->SendBattlePayDistributionUpdate(GetSession(), BATTLE_PAY_SERVICE_BOOST, m_charBoostInfo.action);
            m_charBoostInfo = CharacterBoostData();
            m_boosting = false;
            m_useId = 0;
            m_completed = false;
            break;
        default:
            break;
    }
}

bool CharacterBooster::HasRecoverySchema() const
{
    QueryResult auth = LoginDatabase.Query("SELECT COUNT(*) FROM information_schema.TABLES WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME IN ('account_boost','battlepay_boost_use') AND ENGINE='InnoDB'");
    QueryResult chars = CharacterDatabase.Query("SELECT COUNT(*) FROM information_schema.TABLES WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME IN ('characters','character_inventory','item_instance','mail','mail_items','character_spell','character_skills','character_talent','character_queststatus','character_queststatus_rewarded','battlepay_boost_delivery') AND ENGINE='InnoDB'");
    QueryResult boostKey = LoginDatabase.Query("SELECT COUNT(*) FROM (SELECT INDEX_NAME FROM information_schema.STATISTICS WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME='account_boost' GROUP BY INDEX_NAME,NON_UNIQUE HAVING NON_UNIQUE=0 AND GROUP_CONCAT(COLUMN_NAME ORDER BY SEQ_IN_INDEX) IN ('id,realmid','realmid,id')) AS boost_keys");
    if (sWorld->getBoolConfig(CONFIG_EXECUTED_SERVICES_LOG))
    {
        QueryResult log = CharacterDatabase.Query("SELECT COUNT(*) FROM information_schema.TABLES WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME='executed_services' AND ENGINE='InnoDB'");
        if (!log || log->Fetch()[0].GetUInt64() != 1)
            return false;
    }
    return auth && chars && boostKey && boostKey->Fetch()[0].GetUInt64() > 0 && auth->Fetch()[0].GetUInt64() == 2 && chars->Fetch()[0].GetUInt64() == 11;
}

bool CharacterBooster::SetBoostedCharInfo(ObjectGuid guid, uint32 action, uint32 specialization, bool allianceFaction)
{
    // Boosts operate on offline, authenticated-account characters only. Do not
    // accept client faction/spec fields before checking the character itself.
    if (m_useId || m_boosting || !guid.IsPlayer() || GetSession()->GetPlayer() || GetSession()->PlayerLoading() || !HasRecoverySchema())
        return false;
    uint32 const account = GetSession()->GetAccountId();
    QueryResult character = CharacterDatabase.PQuery("SELECT race,class,level,online FROM characters WHERE guid=%u AND account=%u", guid.GetCounter(), account);
    if (!character)
        return false;
    Field* fields = character->Fetch();
    uint8 race = fields[0].GetUInt8(), classId = fields[1].GetUInt8();
    ChrSpecializationEntry const* spec = sChrSpecializationStore.LookupEntry(specialization);
    if (!race || race >= MAX_RACES || !spec || spec->classId != classId || fields[2].GetUInt8() >= 90 || fields[3].GetUInt8())
        return false;
    if (race != RACE_PANDAREN_NEUTRAL && allianceFaction != (Player::TeamForRace(race) == ALLIANCE))
        return false;

    // The unique character key and atomic counter/state transition turn an
    // interrupted application into a recoverable reservation, never a new debit.
    LoginDatabase.DirectPExecute("INSERT IGNORE INTO battlepay_boost_use(account_id,realm_id,entitlement_realm,character_guid,specialization,alliance) SELECT id,%u,realmid,%u,%u,%u FROM account_boost WHERE id=%u AND realmid IN (-1,%u) AND counter>0 ORDER BY realmid DESC LIMIT 1", realm.Id.Realm, guid.GetCounter(), specialization, uint32(allianceFaction), account, realm.Id.Realm);
    QueryResult use = LoginDatabase.PQuery("SELECT id,state,specialization,alliance FROM battlepay_boost_use WHERE account_id=%u AND realm_id=%u AND character_guid=%u", account, realm.Id.Realm, guid.GetCounter());
    if (!use || use->Fetch()[1].GetUInt8() > 1 || use->Fetch()[2].GetUInt32() != specialization || bool(use->Fetch()[3].GetUInt8()) != allianceFaction)
        return false;
    uint64 id = use->Fetch()[0].GetUInt64();
    LoginDatabase.DirectPExecute(BattlePayBoostQueries::Reserve, id, account, realm.Id.Realm);
    use = LoginDatabase.PQuery("SELECT state FROM battlepay_boost_use WHERE id=%llu AND account_id=%u", id, account);
    if (!use || use->Fetch()[0].GetUInt8() != 1)
        return false;

    m_useId = id;
    m_completed = false;
    m_boosting = true;
    m_charBoostInfo.charGuid = guid;
    m_charBoostInfo.action = action;
    m_charBoostInfo.specialization = specialization;
    m_charBoostInfo.allianceFaction = allianceFaction;
    return true;
}

void CharacterBooster::RecoverBoost()
{
    if (GetSession()->GetPlayer() || GetSession()->PlayerLoading() || !HasRecoverySchema())
        return;
    uint32 const account = GetSession()->GetAccountId();
    QueryResult result = LoginDatabase.PQuery("SELECT id,character_guid,specialization,alliance FROM battlepay_boost_use WHERE account_id=%u AND realm_id=%u AND state=1 ORDER BY id LIMIT 1", account, realm.Id.Realm);
    if (!result)
        return;
    m_useId = result->Fetch()[0].GetUInt64();
    m_charBoostInfo.charGuid = ObjectGuid::Create<HighGuid::Player>(result->Fetch()[1].GetUInt32());
    m_charBoostInfo.specialization = result->Fetch()[2].GetUInt32();
    m_charBoostInfo.allianceFaction = result->Fetch()[3].GetUInt8() != 0;
    m_charBoostInfo.action = CHARACTER_BOOST_APPLIED;
    m_boosting = true;
    m_completed = false;
    m_timer = 500;
    m_sendPacket = true;
}

void CharacterBooster::Update(uint32 diff)
{
    if (GetSession()->IsBot())
        return;
    if (!m_useId)
    {
        if (m_recoveryTimer <= diff)
        {
            m_recoveryTimer = 30000;
            RecoverBoost();
        }
        else
            m_recoveryTimer -= diff;
    }
    if (m_sendPacket)
    {
        if (m_timer <= diff)
        {
            m_sendPacket = false;
            m_completed = _HandleCharacterBoost();
            if (!m_completed && m_useId)
            {
                m_timer = 30000;
                m_sendPacket = true;
            }
        }
        else
            m_timer -= diff;
    }
}
