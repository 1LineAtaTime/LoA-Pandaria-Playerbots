-- #963 (lane b): remaining DBErrors.log items 3-4 that have a source-backed data fix. Idempotent.
-- Sources (local copies, read 2026-10-28):
--   TC335  = TrinityCore TDB_full_world_335.26091_2026_09_09 (release TDB335.26091)
--   TC434  = The-Cataclysm-Preservation-Project TDB_full_world_434.22011_2022_01_09 (release TDB434.22011)
--   TC442  = TrinityCore TDB_full_world_442.26081_2026_08_14 (release TDB442.26081)
--   TC1210 = TrinityCore TDB_full_world_1210.26091_2026_09_09 (release TDB1210.26091)
--   SF548  = ProjectSkyfire SFDB_full_548_26.002 (release sf_db_26)
--   EMU    = the reference 5.4.8 world DB, AC = acore_world, AK = a second 5.4.8 world DB , all on a local DB

-- ---------------------------------------------------------------------------------------------------------------
-- ScriptNames with no core script (item 3). C++ for spell_protection_of_elune and achievement_im_on_a_boat is
-- registered in the same change (boss_archimonde.cpp / boss_gunship_battle.cpp).
-- 56688 Planting Stave Credit carries 'npc_master_shang_xi_thousand_staff_escort' (old/old/2021_12_27_00 and
--   old/test/2023_1_18_00 SQL). That AI is the summoned Master Shang Xi escort: on the static credit bunny it loops
--   Talk(0..5) every 15 s and despawns/respawns (77 "Could not find Text" lines per boot-hour). Quest 29790 is run by
--   SmartAI on 56686 (summoned by 55672). EMU and AK have ScriptName '' on 56688.
UPDATE `creature_template` SET `ScriptName`='' WHERE `entry`=56688 AND `ScriptName`='npc_master_shang_xi_thousand_staff_escort';
-- sat_icy_shadows (spell areatrigger 742, Twin Consorts): the class exists in boss_twin_consorts.cpp but its registration
--   is commented out upstream (another 5.4.8 core/LoA) because the effect is done by npc AI ("Areatrigger have some issue with
--   low duration and LoS", boss_twin_consorts.cpp:1533-1547). The binding is dead; keep the areatrigger, drop the name.
UPDATE `spell_areatrigger_template` SET `ScriptName`='' WHERE `Entry`=742 AND `ScriptName`='sat_icy_shadows';
-- at_well_of_eternity_skip_illidan_intro (areatrigger 7144): registration commented out upstream with
--   "FIXME: if players don't move to first trash pack after event started, pass can be closed and players can't complete
--   instance" (well_of_eternity.cpp:3632). CPP 4.3.4 master has no such script either. Dead binding.
DELETE FROM `areatrigger_scripts` WHERE `entry`=7144 AND `ScriptName`='at_well_of_eternity_skip_illidan_intro';

-- ---------------------------------------------------------------------------------------------------------------
-- npc_spellclick_spells: 54513 is not a vehicle-enter aura. TC335/TC434/TC442/TC1210 all use 46598 (Ride Vehicle
-- Hardcoded) for 29460 Frigid Proto-Drake; SF548 carries the same wrong 54513.
UPDATE `npc_spellclick_spells` SET `spell_id`=46598 WHERE `npc_entry`=29460 AND `spell_id`=54513;
-- vehicle_template_accessory 54499 (The Abominable Greench) has accessories but no spellclick row:
-- TC442 and TC1210 npc_spellclick_spells (54499, 46598, 0, 0).
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=54499 AND `spell_id`=46598;
INSERT INTO `npc_spellclick_spells` (`npc_entry`,`spell_id`,`cast_flags`,`user_type`) VALUES (54499,46598,0,0);

-- ---------------------------------------------------------------------------------------------------------------
-- GO trap data (13 lines): spell-focus 126337-126345 (data2) and goobers 181168/181169 (data12=4), 181575-181578
-- (data12=129) link to traps 4 and 129, which in this DB are type-0 placeholders ('unk name').
-- TC335 + TC434: gameobject_template 4 = type 6 'Bonfire Damage' (Data1 1, Data2 3, Data3 spell 7902);
--                129 = type 6 'Naxx Teleporter trap' (Data1 1, Data3 spell 64446). displayId kept.
UPDATE `gameobject_template` SET `type`=6, `name`='Bonfire Damage', `data0`=0, `data1`=1, `data2`=3, `data3`=7902 WHERE `entry`=4 AND `type`=0;
UPDATE `gameobject_template` SET `type`=6, `name`='Naxx Teleporter trap', `data0`=0, `data1`=1, `data2`=0, `data3`=64446 WHERE `entry`=129 AND `type`=0;

-- ---------------------------------------------------------------------------------------------------------------
-- spell_target_position (6 lines "does not have target TARGET_DEST_DB").
-- 49986 eff1 -> eff0, 65042 eff2 -> eff0: EMU (the live MoP DB) has both at effIndex 0; TC335/TC434/TC442/TC1210 have 65042 at 0.
DELETE FROM `spell_target_position` WHERE `id` IN (49986,65042) AND `effIndex`=0;
UPDATE `spell_target_position` SET `effIndex`=0 WHERE (`id`=49986 AND `effIndex`=1) OR (`id`=65042 AND `effIndex`=2);
-- 100679 Dreadflame summon: SpellMgr.cpp:5131 forces EFFECT_0 TargetA = TARGET_DEST_DEST, the DB position is never used.
-- 66836/66925 (Gilneas worgen summons): absent from TC442/TC1210 and EMU. 105002: absent from every source (EMU, TC*, SF548).
DELETE FROM `spell_target_position` WHERE (`id`=100679 AND `effIndex`=2) OR (`id` IN (66836,66925,105002) AND `effIndex`=0);

-- ---------------------------------------------------------------------------------------------------------------
-- item_enchantment_template: RandomProperty 456 (Native Vest 14096, Native Robe 14109) has only its type-1 (suffix)
-- rows. TC434 has the 29 type-0 rows (identical ench/chance in TC335 and SF548).
DELETE FROM `item_enchantment_template` WHERE `entry`=456 AND `type`=0;
INSERT INTO `item_enchantment_template` (`entry`,`ench`,`chance`,`type`) VALUES
(456,15,1.48,0),(456,19,1.48,0),(456,22,1.48,0),(456,94,4.35,0),(456,95,4.72,0),(456,96,2.41,0),(456,112,4.35,0),
(456,113,4.72,0),(456,114,2.41,0),(456,197,0.1,0),(456,757,5.79,0),(456,758,5.79,0),(456,759,5.79,0),(456,760,5.79,0),
(456,842,3.9,0),(456,843,3.9,0),(456,844,3.9,0),(456,845,3.9,0),(456,1012,4.4,0),(456,1013,4.4,0),(456,1014,4.4,0),
(456,1015,4.4,0),(456,1307,0.1,0),(456,1802,3.48,0),(456,1803,3.48,0),(456,1840,2.42,0),(456,1841,2.42,0),
(456,1954,3.33,0),(456,1955,3.33,0);
-- "Enchantment id #286/#289 used but no ItemRandomProperties": suffix ids filed as type 0 (property).
-- TC434 item_enchantment_template: entry 461 type 0 = property ids 178..1963, type 1 = suffix ids 284-287;
-- entry 462/464 suffix ids are type 1. In this DB entry 461 has the two types swapped and 462/464 file suffix ids
-- 288-291 under type 0. Swap 461 back, move 462/464 288-291 to type 1.
UPDATE `item_enchantment_template` SET `type`=2 WHERE `entry`=461 AND `type`=1 AND `ench` NOT IN (284,285,286,287,360,361,362,363);
UPDATE `item_enchantment_template` SET `type`=1 WHERE `entry`=461 AND `type`=0 AND `ench` IN (284,285,286,287,360,361,362,363);
UPDATE `item_enchantment_template` SET `type`=0 WHERE `entry`=461 AND `type`=2;
UPDATE `item_enchantment_template` SET `type`=1 WHERE `entry` IN (462,464) AND `type`=0 AND `ench` IN (288,289,290,291);

-- ---------------------------------------------------------------------------------------------------------------
-- SmartScript::InstallTemplate "AI Template can not be set more then once" (9458, 9523): the template is installed
-- by an On-Respawn row, so every respawn after the first logs it. TC335/TC1210 replaced the template with a
-- combat-move cast (SMARTCAST_COMBAT_MOVE 0x40, supported here, SmartScriptMgr.h:1450).
-- 9458 Horde Axe Thrower: TC335 smart_scripts 9458/0 'In Combat CMC - Cast Throw'.
DELETE FROM `smart_scripts` WHERE `entryorguid`=9458 AND `source_type`=0 AND `id`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(9458,0,0,0,0,0,100,0,0,0,2300,3900,0,11,10277,64,0,0,0,0,2,0,0,0,0,0,0,0,0,'Horde Axe Thrower - In Combat CMC - Cast ''Throw''');
-- 9523 Kolkar Stormseer: TC335 smart_scripts 9523/1 'In Combat CMC - Cast Lightning Bolt'.
DELETE FROM `smart_scripts` WHERE `entryorguid`=9523 AND `source_type`=0 AND `id`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(9523,0,0,0,0,0,100,0,0,0,3400,4800,0,11,9532,64,0,0,0,0,2,0,0,0,0,0,0,0,0,'Kolkar Stormseer - In Combat CMC - Cast ''Lightning Bolt''');

-- "ProcessAction: Entry 49161 ... Link Event 2 not found": id 1 links to a row that does not exist.
-- TC1210 has the morph without a link chain (aggro -> morph, evade -> demorph).
UPDATE `smart_scripts` SET `link`=0 WHERE `entryorguid`=49161 AND `source_type`=0 AND `id`=1 AND `link`=2;

-- "SmartScript: Entry 30134 has events but no events added to list because of instance flags": all 16 rows of
-- 30134 'Machine' (Storm Peaks, Brann ride) carry event_flags 512. In TC335/TC434/TC1210 512 is
-- SMART_EVENT_FLAG_WHILE_CHARMED; in this core 0x200 is SMART_EVENT_FLAG_DIFFICULTY_RAID40 (SmartScriptMgr.h:1427),
-- so outside instances the whole script is dropped. This core has no WHILE_CHARMED flag and does not gate SmartAI
-- events on charm (SmartAI.cpp has no such check), so clearing the bit gives TC's "also while charmed" behaviour.
-- The same TC-imported bit silently drops rows of every other open-world entry that carries it (spawn maps checked:
-- none of these spawn in the 40-man raids 409/469/531 where 0x200 is meaningful here):
--   8719 Auctioneer Fitch (gossip start + kill credit of "Fencing the Goods" -> the text fix above needs this),
--   9163, 12918, 17702, 19270, 23671, 30134, 31428 Crusader Olakin Sainrith, 31432 Ghostwing, 49479.
UPDATE `smart_scripts` SET `event_flags`=`event_flags` & ~512
WHERE `source_type`=0 AND (`event_flags` & 512) AND `entryorguid` IN (8719,9163,12918,17702,19270,23671,30134,31428,31432,49479);
