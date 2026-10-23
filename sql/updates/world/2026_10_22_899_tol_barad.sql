-- #899 Tol Barad battlefield port: world DB. DRAFT, idempotent (DELETE-then-INSERT / scoped UPDATE).
-- Source: TrinityCore sql/old/6.x/world/04_2016_10_17/2016_07_15_00_world.sql (Tol Barad) for texts, script names, spell 85123;
--         group numbering follows the LoA port (BattlefieldTB.h enum TolBaradText), which swaps TC's Slagworks/Vigil groups.

-- 1) creature_text for the DEBUG Announcer stalker 43679, groups 1-26 (group 0 is the existing Shadowfang row: untouched).
--    TC 2016_07_15_00 lines 274-301. Port: Vigil = 11-14, Slagworks = 15-18 (TC has them the other way round).
DELETE FROM `creature_text` WHERE `CreatureID` = 43679 AND `GroupID` BETWEEN 1 AND 26;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(43679, 1, 0, 'The East Spire has been heavily damaged!', 41, 0, 100, 0, 0, 0, 0, 45453, 2, 'Tol Barad announcer'),
(43679, 2, 0, 'The East Spire has been destroyed!', 41, 0, 100, 0, 0, 0, 0, 45454, 2, 'Tol Barad announcer'),
(43679, 3, 0, 'The West Spire has been heavily damaged!', 41, 0, 100, 0, 0, 0, 0, 45456, 2, 'Tol Barad announcer'),
(43679, 4, 0, 'The West Spire has been destroyed!', 41, 0, 100, 0, 0, 0, 0, 45457, 2, 'Tol Barad announcer'),
(43679, 5, 0, 'The South Spire has been heavily damaged!', 41, 0, 100, 0, 0, 0, 0, 45458, 2, 'Tol Barad announcer'),
(43679, 6, 0, 'The South Spire has been destroyed!', 41, 0, 100, 0, 0, 0, 0, 45459, 2, 'Tol Barad announcer'),
(43679, 7, 0, 'The Alliance has gained control of the Ironclad Garrison!', 41, 0, 100, 0, 0, 0, 0, 45463, 2, 'Tol Barad announcer'),
(43679, 8, 0, 'The Alliance has lost control of the Ironclad Garrison!', 41, 0, 100, 0, 0, 0, 0, 45462, 2, 'Tol Barad announcer'),
(43679, 9, 0, 'The Horde has taken the Ironclad Garrison!', 41, 0, 100, 0, 0, 0, 0, 45464, 2, 'Tol Barad announcer'),
(43679, 10, 0, 'The Horde has lost the Ironclad Garrison!', 41, 0, 100, 0, 0, 0, 0, 45465, 2, 'Tol Barad announcer'),
(43679, 11, 0, 'The Alliance has gained control of Warden''s Vigil!', 41, 0, 100, 0, 0, 0, 0, 45471, 2, 'Tol Barad announcer'),
(43679, 12, 0, 'The Alliance has lost control of Warden''s Vigil!', 41, 0, 100, 0, 0, 0, 0, 45470, 2, 'Tol Barad announcer'),
(43679, 13, 0, 'The Horde has taken Warden''s Vigil!', 41, 0, 100, 0, 0, 0, 0, 45472, 2, 'Tol Barad announcer'),
(43679, 14, 0, 'The Horde has lost Warden''s Vigil!', 41, 0, 100, 0, 0, 0, 0, 45473, 2, 'Tol Barad announcer'),
(43679, 15, 0, 'The Alliance has gained control of the Slagworks!', 41, 0, 100, 0, 0, 0, 0, 45467, 2, 'Tol Barad announcer'),
(43679, 16, 0, 'The Alliance has lost control of the Slagworks!', 41, 0, 100, 0, 0, 0, 0, 45466, 2, 'Tol Barad announcer'),
(43679, 17, 0, 'The Horde has taken the Slagworks!', 41, 0, 100, 0, 0, 0, 0, 45468, 2, 'Tol Barad announcer'),
(43679, 18, 0, 'The Horde has lost the Slagworks!', 41, 0, 100, 0, 0, 0, 0, 45469, 2, 'Tol Barad announcer'),
(43679, 19, 0, 'The Alliance has successfully defended Tol Barad!', 41, 0, 100, 0, 0, 8455, 0, 45174, 2, 'Tol Barad announcer'),
(43679, 20, 0, 'The Horde has successfully defended Tol Barad!', 41, 0, 100, 0, 0, 8454, 0, 45188, 2, 'Tol Barad announcer'),
(43679, 21, 0, 'The Alliance has successfully taken Tol Barad!', 41, 0, 100, 0, 0, 8455, 0, 45189, 2, 'Tol Barad announcer'),
(43679, 22, 0, 'The Horde has successfully taken Tol Barad!', 41, 0, 100, 0, 0, 8454, 0, 45190, 2, 'Tol Barad announcer'),
(43679, 23, 0, 'Warning: This vehicle is not permitted to operate outside of Tol Barad. Turn back now.', 41, 0, 100, 0, 0, 0, 0, 45526, 0, 'Tol Barad announcer'),
(43679, 24, 0, 'Preparations for Tol Barad will begin in 5 minutes!', 41, 0, 100, 0, 0, 0, 0, 50887, 2, 'Tol Barad announcer'),
(43679, 25, 0, 'Preparations for Tol Barad will begin in 2 minutes!', 41, 0, 100, 0, 0, 0, 0, 50888, 2, 'Tol Barad announcer'),
(43679, 26, 0, 'Preparations for Tol Barad will begin in 1 minute!', 41, 0, 100, 0, 0, 0, 0, 50889, 2, 'Tol Barad announcer');

-- 2) ScriptNames. Spirit guides 45066-45079 -> npc_tb_spiritguide (TC sets its spirit-guide script on 45068-45079; the port also
--    spawns the Baradin Hold pair 45066/45067 through the same graveyard code). Queue battlemages 47320 Kaellin Tarvane (Alliance)
--    and 47321 Zugra Flamefist (Horde), subname Tol Barad, gossip menu 12240: npc_tol_barad_battlemage (entries inferred, see report).
UPDATE `creature_template` SET `ScriptName` = 'npc_tb_spiritguide' WHERE `entry` BETWEEN 45066 AND 45079;
UPDATE `creature_template` SET `ScriptName` = 'npc_tol_barad_battlemage' WHERE `entry` IN (47320, 47321);

-- 3) Spell script 85123 Siege Cannon (TC 2016_07_15_00 lines 62-67) + target condition (TC: targets NPC 45561 Tower Cannon Target).
DELETE FROM `spell_script_names` WHERE `spell_id` = 85123;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES (85123, 'spell_siege_cannon');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 85123 AND `SourceGroup` = 3;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13, 3, 85123, 0, 0, 31, 0, 3, 45561, 0, 0, 0, 0, '', 'Siege Cannon targets NPC_TOWER_CANNON_TARGET');

-- 4) Remove static map-732 spawns of everything BattlefieldTB spawns itself (TC removes battlefield-owned static spawns; leaving them
--    duplicates every NPC/GO). Entries = the SpawnCreature/SpawnGameObject/InitStalker call sites in BattlefieldTB.cpp:
--    stalker 43679; spirit guides 45066-45079; quest infantry 47595 47598-47600 47607-47610; guards 51165 51166; faction NPCs
--    47324 47325 47335 47336 50173 50167 48066 48039 48061 48074 48069 48062 48070 48071; Abandoned Siege Engine 45344.
--    GOs: spires 204588-204590, capture flagpoles 205068 205096 205101 205103 205138 205139, banners 207391 207400, portals 208226 208227.
--    map = 732 only: 43679 also has a Shadowfang copy (map 33) and 47324/47325/47335/47336 have Stormwind/Orgrimmar copies (maps 0/1): kept.
DELETE FROM `creature_addon`        WHERE `guid` IN (SELECT `guid` FROM `creature` WHERE `map` = 732 AND `id` IN (43679,45066,45067,45068,45069,45070,45071,45072,45073,45074,45075,45076,45077,45078,45079,47595,47598,47599,47600,47607,47608,47609,47610,51165,51166,47324,47325,47335,47336,50173,50167,48066,48039,48061,48074,48069,48062,48070,48071,45344));
DELETE FROM `game_event_creature`   WHERE `guid` IN (SELECT `guid` FROM `creature` WHERE `map` = 732 AND `id` IN (43679,45066,45067,45068,45069,45070,45071,45072,45073,45074,45075,45076,45077,45078,45079,47595,47598,47599,47600,47607,47608,47609,47610,51165,51166,47324,47325,47335,47336,50173,50167,48066,48039,48061,48074,48069,48062,48070,48071,45344));
DELETE FROM `pool_creature`         WHERE `guid` IN (SELECT `guid` FROM `creature` WHERE `map` = 732 AND `id` IN (43679,45066,45067,45068,45069,45070,45071,45072,45073,45074,45075,45076,45077,45078,45079,47595,47598,47599,47600,47607,47608,47609,47610,51165,51166,47324,47325,47335,47336,50173,50167,48066,48039,48061,48074,48069,48062,48070,48071,45344));
DELETE FROM `linked_respawn`        WHERE `guid` IN (SELECT `guid` FROM `creature` WHERE `map` = 732 AND `id` IN (43679,45066,45067,45068,45069,45070,45071,45072,45073,45074,45075,45076,45077,45078,45079,47595,47598,47599,47600,47607,47608,47609,47610,51165,51166,47324,47325,47335,47336,50173,50167,48066,48039,48061,48074,48069,48062,48070,48071,45344));
DELETE FROM `creature` WHERE `map` = 732 AND `id` IN (43679,45066,45067,45068,45069,45070,45071,45072,45073,45074,45075,45076,45077,45078,45079,47595,47598,47599,47600,47607,47608,47609,47610,51165,51166,47324,47325,47335,47336,50173,50167,48066,48039,48061,48074,48069,48062,48070,48071,45344);
DELETE FROM `gameobject_addon`      WHERE `guid` IN (SELECT `guid` FROM `gameobject` WHERE `map` = 732 AND `id` IN (204588,204589,204590,205068,205096,205101,205103,205138,205139,207391,207400,208226,208227));
DELETE FROM `game_event_gameobject` WHERE `guid` IN (SELECT `guid` FROM `gameobject` WHERE `map` = 732 AND `id` IN (204588,204589,204590,205068,205096,205101,205103,205138,205139,207391,207400,208226,208227));
DELETE FROM `pool_gameobject`       WHERE `guid` IN (SELECT `guid` FROM `gameobject` WHERE `map` = 732 AND `id` IN (204588,204589,204590,205068,205096,205101,205103,205138,205139,207391,207400,208226,208227));
DELETE FROM `gameobject` WHERE `map` = 732 AND `id` IN (204588,204589,204590,205068,205096,205101,205103,205138,205139,207391,207400,208226,208227);
