-- #443 Kezan "The Great Bank Heist" (14122): the vault minigame gives the player no visible prompt and no tool use counts.
--  1. Prompt: npc_first_bank_of_kezan_vault (zone_kezan.cpp) only put the hint aura 67495-67499 ("The Great Bank Heist:
--     Bunny - <tool> Aura", SpellEffect 6/aura 4 dummy, no icon/text) on the vault, so the client shows nothing. The retail
--     prompt lines exist in BroadcastText (world.broadcast_text, checked 2026-10-06): 35577 Blastcrackers, 35576 Kaja'mite
--     Drill, 35578 Ear-O-Scope, 35579 Infinifold Lockpick, 35700 Amazing G-Ray ("Use your |cFFFF2222<tool>!|r$B<icon>"),
--     35702 "Correct!", 35704 "Incorrect!". AshamaneCore (kezan.cpp:551-564, 694-698)
--     talks exactly these. Groups 5-11 are new (35486 had groups 0-4 only). Type 42 = CHAT_MSG_RAID_BOSS_WHISPER
--     (SharedDefines.h:3869): centre-screen, sent only to the rider (CreatureTextMgr.cpp:336), the vault is private.
--  2. Tool casts: the vault seat controls the vehicle (VehicleSeat 5326 flags 0x62000806 = CAN_CONTROL), so SpellHandler.cpp:1050
--     lets the vault cast its bar spells 67508/67522/67524/67525/67526; their only effect targets TARGET_UNIT_VEHICLE (94),
--     Spell.cpp:1682 = caster->GetVehicleBase() = none, so the vault's SpellHit never ran. spell_great_bank_heist_vault_tool
--     (new, zone_kezan.cpp) forwards the cast to the vault AI from AfterCast.
-- Needs the matching zone_kezan.cpp change (same build). Re-runnable.
--
-- BACKUP (run first, keep the file):
--   mysqldump -h<host> -u<user> -p<password> --no-create-info --skip-extended-insert world creature_text --where="CreatureID=35486" > creature_text-35486-before-443.sql
--   mysqldump -h<host> -u<user> -p<password> --no-create-info --skip-extended-insert world spell_script_names --where="spell_id IN (67508,67522,67524,67525,67526)" > spell_script_names-vault-tools-before-443.sql
-- ROLLBACK:
--   DELETE FROM `creature_text` WHERE `CreatureID` = 35486 AND `GroupID` BETWEEN 5 AND 11;
--   DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_great_bank_heist_vault_tool';
--   (both backups are empty today: no rows for these keys exist in the test world, checked 2026-10-06)

DELETE FROM `creature_text` WHERE `CreatureID` = 35486 AND `GroupID` BETWEEN 5 AND 11;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(35486, 5, 0, 'Use your |cFFFF2222Blastcrackers!|r$B|TInterface\\Icons\\INV_Misc_Bomb_07.blp:64|t', 42, 0, 100, 0, 0, 0, 0, 35577, 0, 'First Bank of Kezan Vault - prompt Blastcrackers (67508)'),
(35486, 6, 0, 'Use your |cFFFF2222Kaja''mite Drill!|r$B|TInterface\\Icons\\INV_Weapon_ShortBlade_21.blp:64|t', 42, 0, 100, 0, 0, 0, 0, 35576, 0, 'First Bank of Kezan Vault - prompt Kaja''mite Drill (67522)'),
(35486, 7, 0, 'Use your |cFFFF2222Ear-O-Scope!|r$B|TInterface\\Icons\\INV_Misc_Ear_NightElf_02.blp:64|t', 42, 0, 100, 0, 0, 0, 0, 35578, 0, 'First Bank of Kezan Vault - prompt Ear-O-Scope (67524)'),
(35486, 8, 0, 'Use your |cFFFF2222Infinifold Lockpick!|r$B|TInterface\\Icons\\INV_Misc_EngGizmos_swissArmy.blp:64|t', 42, 0, 100, 0, 0, 0, 0, 35579, 0, 'First Bank of Kezan Vault - prompt Infinifold Lockpick (67525)'),
(35486, 9, 0, 'Use your |cFFFF2222Amazing G-Ray!|r$B|TInterface\\Icons\\INV_Misc_EngGizmos_20.blp:64|t', 42, 0, 100, 0, 0, 0, 0, 35700, 0, 'First Bank of Kezan Vault - prompt Amazing G-Ray (67526)'),
(35486, 10, 0, 'Correct!', 42, 0, 100, 0, 0, 0, 0, 35702, 0, 'First Bank of Kezan Vault - right tool'),
(35486, 11, 0, 'Incorrect!', 42, 0, 100, 0, 0, 0, 0, 35704, 0, 'First Bank of Kezan Vault - wrong tool');

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_great_bank_heist_vault_tool';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(67508, 'spell_great_bank_heist_vault_tool'),
(67522, 'spell_great_bank_heist_vault_tool'),
(67524, 'spell_great_bank_heist_vault_tool'),
(67525, 'spell_great_bank_heist_vault_tool'),
(67526, 'spell_great_bank_heist_vault_tool');
