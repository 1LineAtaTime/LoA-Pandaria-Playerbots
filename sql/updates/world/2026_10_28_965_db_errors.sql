-- #965 (lane #963b): remaining DBErrors.log types assigned from #965. Idempotent.
-- Sources (local copies, read 2026-10-28): TC335 = TrinityCore TDB_full_world_335.26091; TC434 = CPP
-- TDB_full_world_434.22011; TC442 = TrinityCore TDB_full_world_442.26081; TC1210 = TrinityCore TDB_full_world_1210.26091;
-- SF548 = ProjectSkyfire SFDB_full_548_26.002; EMU = the reference 5.4.8 world DB; AC = acore_world; AK = a second 5.4.8 world DB (another 5.4.8 core).

-- ---------------------------------------------------------------------------------------------------------------
-- creature_queststarter / creature_questender
-- 7062 "The Reason for the Season": starter is 1365 Goli Krumn in TC442 and TC1210 (1362 Gothor Brumn is an armorer
--   without the questgiver flag in every source).
DELETE FROM `creature_queststarter` WHERE `id`=1362 AND `quest`=7062;
INSERT IGNORE INTO `creature_queststarter` (`id`,`quest`) VALUES (1365,7062);
-- 31713 "The Big Brew-haha": ender is 58506 only (TC1210, SF548); 58507 Ace Longpaw has no questgiver flag.
DELETE FROM `creature_questender` WHERE `id`=58507 AND `quest`=31713;
-- 33138 "Why Do We Fight?": TC1210 has the same ender 73136 Lorewalker Cho with npcflag 2 (questgiver); ours is 0.
UPDATE `creature_template` SET `npcflag`=`npcflag`|2 WHERE `entry`=73136;
-- 32592 (Wrathion 69782): quest_template has no row 32592 in this DB, EMU, SF548 or AK (TC1210 has it, a newer-schema row
--   that cannot be ported as is) -> the two relation rows are dead; the missing quest is reported.
DELETE FROM `creature_queststarter` WHERE `id`=69782 AND `quest`=32592;
DELETE FROM `creature_questender` WHERE `id`=69782 AND `quest`=32592;

-- ---------------------------------------------------------------------------------------------------------------
-- Sayge 14822: spell 23770 "Sayge's Carnie Buff" does not exist in this client's data ("Aura condition has non existing
-- spell 23770" x3, "SmartAIMgr: Entry 14822 ... non-existent Spell entry 23770"). TC1210 and SF548 have no
-- smart_scripts 14822/14; their fortune casts ids 1-12 have link 0 (ours link to 14). The three conditions
-- on aura 23770 (TC335/TC442/TC1210 still carry them, for older clients) can never be true/false here -> dead.
DELETE FROM `smart_scripts` WHERE `entryorguid`=14822 AND `source_type`=0 AND `id`=14 AND `action_param1`=23770;
UPDATE `smart_scripts` SET `link`=0 WHERE `entryorguid`=14822 AND `source_type`=0 AND `id` BETWEEN 1 AND 12 AND `link`=14;
DELETE FROM `conditions` WHERE `ConditionTypeOrReference`=1 AND `ConditionValue1`=23770 AND `SourceGroup`=6186;

-- ---------------------------------------------------------------------------------------------------------------
-- SmartAI
-- 31238 Hira Snowdawn: "invalid action type (0)" - the columns are shifted by one (action_type 0, action_param1 80,
--   action_param2 3123800). TC335/AC: action_type 80, action_param1 3123800 / 3123801.
UPDATE `smart_scripts` SET `action_type`=80, `action_param1`=3123800, `action_param2`=0 WHERE `entryorguid`=31238 AND `source_type`=0 AND `id`=0 AND `action_type`=0;
UPDATE `smart_scripts` SET `action_type`=80, `action_param1`=3123801, `action_param2`=0 WHERE `entryorguid`=31238 AND `source_type`=0 AND `id`=1 AND `action_type`=0;
-- 170600 (Defias Prisoner action list): action 124 (LOAD_EQUIPMENT) is not implemented by this core
--   ("Not handled action_type(124)"); AC has the same row because AC implements it. Drop the row here.
DELETE FROM `smart_scripts` WHERE `entryorguid`=170600 AND `source_type`=9 AND `id`=9 AND `action_type`=124;
-- 170600 id 10 starts SmartAI waypoint path 1706 ("non-existent WaypointPath id 1706"): AC acore_world waypoints
--   entry 1706 (103 points, 'Defias Prisoner').
DELETE FROM `waypoints` WHERE `entry`=1706;
INSERT INTO `waypoints` (`entry`,`pointid`,`position_x`,`position_y`,`position_z`,`orientation`,`delay`,`point_comment`) VALUES
(1706,1,-8641.35,1324.81,5.233,NULL,0,'Defias Prisoner'),
(1706,2,-8640.57,1323.19,5.25525,NULL,0,'Defias Prisoner'),
(1706,3,-8631.9,1306.63,5.2324,NULL,0,'Defias Prisoner'),
(1706,4,-8619.75,1282.73,5.2324,NULL,0,'Defias Prisoner'),
(1706,5,-8608.1,1259.83,5.2324,NULL,0,'Defias Prisoner'),
(1706,6,-8600.4,1244.67,5.70763,NULL,0,'Defias Prisoner'),
(1706,7,-8592.91,1229.94,5.2311,NULL,0,'Defias Prisoner'),
(1706,8,-8587.89,1220.72,5.63664,NULL,0,'Defias Prisoner'),
(1706,9,-8585.38,1215.41,5.12153,NULL,0,'Defias Prisoner'),
(1706,10,-8575.2,1193.65,5.69794,NULL,0,'Defias Prisoner'),
(1706,11,-8574.88,1186.66,11.3194,NULL,0,'Defias Prisoner'),
(1706,12,-8574.82,1178.47,17.9033,NULL,0,'Defias Prisoner'),
(1706,13,-8573.39,1175.27,17.7088,NULL,0,'Defias Prisoner'),
(1706,14,-8575.38,1163.84,17.7088,NULL,0,'Defias Prisoner'),
(1706,15,-8575.58,1161.6,18.4894,NULL,0,'Defias Prisoner'),
(1706,16,-8575.66,1159.75,17.9452,NULL,0,'Defias Prisoner'),
(1706,17,-8576.77,1149.38,18.0082,NULL,0,'Defias Prisoner'),
(1706,18,-8578.37,1135.47,17.949,NULL,0,'Defias Prisoner'),
(1706,19,-8581.11,1125.33,17.9446,NULL,0,'Defias Prisoner'),
(1706,20,-8583.3,1117.22,17.9447,NULL,0,'Defias Prisoner'),
(1706,21,-8585.34,1111.78,21.284,NULL,0,'Defias Prisoner'),
(1706,22,-8587.77,1105.29,25.3006,NULL,0,'Defias Prisoner'),
(1706,23,-8590.51,1096.39,30.0475,NULL,0,'Defias Prisoner'),
(1706,24,-8590.12,1087.09,33.4427,NULL,0,'Defias Prisoner'),
(1706,25,-8585.84,1075.02,36.1143,NULL,0,'Defias Prisoner'),
(1706,26,-8580.91,1068.56,37.7707,NULL,0,'Defias Prisoner'),
(1706,27,-8573.15,1058.37,44.4035,NULL,0,'Defias Prisoner'),
(1706,28,-8563.31,1045.45,52.8141,NULL,0,'Defias Prisoner'),
(1706,29,-8557.03,1038.48,57.4878,NULL,0,'Defias Prisoner'),
(1706,30,-8553.93,1036.44,59.34,NULL,0,'Defias Prisoner'),
(1706,31,-8546.93,1036.27,59.4142,NULL,0,'Defias Prisoner'),
(1706,32,-8537.82,1034.34,59.6157,NULL,0,'Defias Prisoner'),
(1706,33,-8531.56,1025.92,59.7549,NULL,0,'Defias Prisoner'),
(1706,34,-8521.24,1015.53,59.8643,NULL,0,'Defias Prisoner'),
(1706,35,-8513.08,1011.04,59.6285,NULL,0,'Defias Prisoner'),
(1706,36,-8505.13,1004.29,59.4794,NULL,0,'Defias Prisoner'),
(1706,37,-8498.66,997.183,62.9424,NULL,0,'Defias Prisoner'),
(1706,38,-8489.09,987.094,72.7379,NULL,0,'Defias Prisoner'),
(1706,39,-8486.16,983.431,72.7379,NULL,0,'Defias Prisoner'),
(1706,40,-8482.47,986.317,72.7379,NULL,0,'Defias Prisoner'),
(1706,41,-8476.89,990.667,78.7541,NULL,0,'Defias Prisoner'),
(1706,42,-8470.06,993.107,79.0886,NULL,0,'Defias Prisoner'),
(1706,43,-8464.97,986.427,79.023,NULL,0,'Defias Prisoner'),
(1706,44,-8470.04,982.66,79.0014,NULL,0,'Defias Prisoner'),
(1706,45,-8477.26,976.788,85.2469,NULL,0,'Defias Prisoner'),
(1706,46,-8483.62,971.622,91.8688,NULL,0,'Defias Prisoner'),
(1706,47,-8487.29,968.64,95.8566,NULL,0,'Defias Prisoner'),
(1706,48,-8498.21,961.956,95.9496,NULL,0,'Defias Prisoner'),
(1706,49,-8512.78,954.621,95.85,NULL,0,'Defias Prisoner'),
(1706,50,-8522.01,955.868,95.8665,NULL,0,'Defias Prisoner'),
(1706,51,-8536.29,961.026,96.0033,NULL,0,'Defias Prisoner'),
(1706,52,-8547.29,964.994,96.4981,NULL,0,'Defias Prisoner'),
(1706,53,-8555.11,973.677,96.2403,NULL,0,'Defias Prisoner'),
(1706,54,-8563.69,983.193,96.351,NULL,0,'Defias Prisoner'),
(1706,55,-8570.72,990.993,96.2609,NULL,0,'Defias Prisoner'),
(1706,56,-8576.13,987.894,96.309,NULL,0,'Defias Prisoner'),
(1706,57,-8577.95,986.507,97.1472,NULL,0,'Defias Prisoner'),
(1706,58,-8590.71,976.303,97.8743,NULL,0,'Defias Prisoner'),
(1706,59,-8607.21,963.424,99.4452,NULL,0,'Defias Prisoner'),
(1706,60,-8621.38,957.955,99.4452,NULL,0,'Defias Prisoner'),
(1706,61,-8638.03,952.562,99.4669,NULL,0,'Defias Prisoner'),
(1706,62,-8654.67,947.142,100.68,NULL,0,'Defias Prisoner'),
(1706,63,-8667.72,939.37,101.191,NULL,0,'Defias Prisoner'),
(1706,64,-8685.76,928.624,101.247,NULL,0,'Defias Prisoner'),
(1706,65,-8702.43,917.789,101.241,NULL,0,'Defias Prisoner'),
(1706,66,-8714.44,908.49,101.241,NULL,0,'Defias Prisoner'),
(1706,67,-8727.33,898.504,100.934,NULL,0,'Defias Prisoner'),
(1706,68,-8740.41,890.905,101.429,NULL,0,'Defias Prisoner'),
(1706,69,-8752.08,891.604,101.898,NULL,0,'Defias Prisoner'),
(1706,70,-8762.36,893.742,101.534,NULL,0,'Defias Prisoner'),
(1706,71,-8771.17,901.526,100.75,NULL,0,'Defias Prisoner'),
(1706,72,-8780.8,913.277,100.047,NULL,0,'Defias Prisoner'),
(1706,73,-8792.14,924.999,100.402,NULL,0,'Defias Prisoner'),
(1706,74,-8800.06,935.066,101.241,NULL,0,'Defias Prisoner'),
(1706,75,-8808.02,945.188,101.241,NULL,0,'Defias Prisoner'),
(1706,76,-8815.25,954.374,100.751,NULL,0,'Defias Prisoner'),
(1706,77,-8819.64,951.866,100.791,NULL,0,'Defias Prisoner'),
(1706,78,-8826.17,946.929,103.049,NULL,0,'Defias Prisoner'),
(1706,79,-8834.62,940.698,105.162,NULL,0,'Defias Prisoner'),
(1706,80,-8839.53,937.589,105.285,NULL,0,'Defias Prisoner'),
(1706,81,-8845.4,933.774,103.745,NULL,0,'Defias Prisoner'),
(1706,82,-8849.75,931.067,102.551,NULL,0,'Defias Prisoner'),
(1706,83,-8851.53,929.383,102.024,NULL,0,'Defias Prisoner'),
(1706,84,-8847.38,922.771,101.697,NULL,0,'Defias Prisoner'),
(1706,85,-8842.95,915.887,99.9569,NULL,0,'Defias Prisoner'),
(1706,86,-8836.27,906.291,97.8529,NULL,0,'Defias Prisoner'),
(1706,87,-8828.95,895.775,98.1249,NULL,0,'Defias Prisoner'),
(1706,88,-8821.26,886.969,98.4819,NULL,0,'Defias Prisoner'),
(1706,89,-8813.65,878.104,98.7189,NULL,0,'Defias Prisoner'),
(1706,90,-8804.11,866.37,98.9565,NULL,0,'Defias Prisoner'),
(1706,91,-8799.76,861.024,98.8853,NULL,0,'Defias Prisoner'),
(1706,92,-8797.76,858.557,97.6348,NULL,0,'Defias Prisoner'),
(1706,93,-8798.21,851.501,97.6348,NULL,0,'Defias Prisoner'),
(1706,94,-8798.12,843.382,97.6348,NULL,0,'Defias Prisoner'),
(1706,95,-8798.34,832.884,97.6348,NULL,0,'Defias Prisoner'),
(1706,96,-8797.24,828.395,97.6348,NULL,0,'Defias Prisoner'),
(1706,97,-8791.14,827.631,97.6406,NULL,0,'Defias Prisoner'),
(1706,98,-8787.67,827.162,97.6451,NULL,0,'Defias Prisoner'),
(1706,99,-8785.28,828.929,97.5397,NULL,0,'Defias Prisoner'),
(1706,100,-8780.81,832.646,95.3486,NULL,0,'Defias Prisoner'),
(1706,101,-8772.74,839.364,91.3895,NULL,0,'Defias Prisoner'),
(1706,102,-8764.67,846.081,87.4378,NULL,0,'Defias Prisoner'),
(1706,103,-8759.14,850.244,84.8424,NULL,0,'Defias Prisoner');
-- 31432 Ghostwing starts SmartAI path 31432 ("non-existent WaypointPath id 31432"): TC434 waypoints entry 31432
--   (2 points; TC335/TC1210 use the same two points under path 251457).
DELETE FROM `waypoints` WHERE `entry`=31432;
INSERT INTO `waypoints` (`entry`,`pointid`,`position_x`,`position_y`,`position_z`,`orientation`,`delay`,`point_comment`) VALUES
(31432,1,6644.43,3222.91,823.07,NULL,0,'Ghostwing'),
(31432,2,6690.65,3177.28,860.57,NULL,0,'Ghostwing');
-- 3304100 (Aetherion action list): a SMART_EVENT 75 row inside an action list (source_type 9 rows are timed, event 0)
--   -> "uses min/max params wrong (5/0)", never loaded. Only EMU has it (EMU: distance 50); no TC/AC/SF row. Dead.
DELETE FROM `smart_scripts` WHERE `entryorguid`=3304100 AND `source_type`=9 AND `id`=0 AND `event_type`=75;
-- 38526 B.C. Eliminator: ScriptName npc_canon_gob (C++ creature_script, zone_lost_isles_volcano.cpp:716) owns the AI,
--   so its single SmartAI row (On Respawn - action 103) never runs ("has SmartAI scripts, but its AIName is not 'SmartAI'").
DELETE FROM `smart_scripts` WHERE `entryorguid`=38526 AND `source_type`=0;

-- ---------------------------------------------------------------------------------------------------------------
-- spell_linked_spell: triggers 200002/200003 and effect 203754 exist in no spell store here (spell_dbc has 200004/200005
-- but not 200002/200003/203754); AK and EMU carry the same three rows without definitions. Dead.
DELETE FROM `spell_linked_spell` WHERE (`spell_trigger`=200002 AND `spell_effect`=200004) OR (`spell_trigger`=200003 AND `spell_effect`=200005) OR (`spell_trigger`=123262 AND `spell_effect`=203754);
-- spell_area 67789: quest_start 40328/40329 do not exist in this DB (nor EMU/SF548/AK); the rows are rejected at load.
DELETE FROM `spell_area` WHERE `spell`=67789 AND `quest_start` IN (40328,40329);

-- ---------------------------------------------------------------------------------------------------------------
-- Sha of Anger 60491: lootid 60491 has no creature_loot_template rows but a condition references (60491, 89317)
-- (Claw of Anger, quest 31809). SF548 creature_loot_template (60491, 89317, -5.47213, lootmode 1, group 0, 1, 1).
DELETE FROM `creature_loot_template` WHERE `entry`=60491 AND `item`=89317;
INSERT INTO `creature_loot_template` (`entry`,`item`,`ChanceOrQuestChance`,`lootmode`,`groupid`,`mincountOrRef`,`maxcount`) VALUES
(60491,89317,-5.47213,1,0,1,1);

-- ---------------------------------------------------------------------------------------------------------------
-- "AreaTrigger (ID: 1282) does not exist in AreaTrigger.dbc": 'sat_proving_grounds_berserking' is a spell areatrigger
-- script (atrigger_script, proving_grounds.cpp:3580) filed in areatrigger_scripts. EMU has it in
-- spell_areatrigger_template (1282, Flags 0, CollisionType 1, Radius 1.5, ScaleX 0, ScaleY 0).
DELETE FROM `areatrigger_scripts` WHERE `entry`=1282 AND `ScriptName`='sat_proving_grounds_berserking';
DELETE FROM `spell_areatrigger_template` WHERE `Entry`=1282;
INSERT INTO `spell_areatrigger_template` (`Entry`,`Flags`,`CollisionType`,`Radius`,`ScaleX`,`ScaleY`,`ScriptName`) VALUES
(1282,0,1,1.5,0,0,'sat_proving_grounds_berserking');
