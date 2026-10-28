-- Brazie's Farmstead, Hillsbrad Foothills: Brazie the Botanist's "Plants vs. Ghouls" minigame (zone_hillsbrad_brazie.cpp).
-- Accepting one of Brazie's quests makes the player cast Create Botanist's Vehicle (91683) and ride Brazie (49176), who plants
-- on 40 spots of the lawn while zombies walk down five lanes. Levels 0-1 are ported from TCPP (custom_2018_06_22_00_world.sql);
-- the rows for levels 2-4 and the daily, and the rows that differ from TCPP, say what is ours.

-- Brazie the Botanist (49687): accepting any of his quests starts the game (TCPP: SmartAI). Ours: npc_brazie_the_botanist, whose
-- gossip option starts the game again while one of his quests is in progress and no game runs (after a logout, a shared quest).
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_brazie_the_botanist' WHERE `entry`=49687;
DELETE FROM `smart_scripts` WHERE `entryorguid`=49687 AND `source_type`=0;
DELETE FROM `gossip_menu_option` WHERE `MenuID`=12590 AND `OptionID`=0;
INSERT INTO `gossip_menu_option` (`MenuID`,`OptionID`,`OptionIcon`,`OptionText`,`OptionBroadcastTextID`,`OptionType`,`OptionNpcflag`,`ActionMenuID`,`ActionPoiID`,`BoxCoded`,`BoxMoney`,`BoxText`,`BoxBroadcastTextID`,`VerifiedBuild`) VALUES
(12590,0,0,'We need to give this another try.',50035,1,1,0,0,0,0,'',0,0);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=12590 AND `SourceEntry`=0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(15,12590,0,0,0,9,0,28733,0,0,0,0,0,'','Brazie the Botanist - Show option while Basic Botany is in progress'),
(15,12590,0,0,0,1,0,91683,1,0,1,0,0,'','Brazie the Botanist - Show option while no game runs'),
(15,12590,0,0,1,9,0,28617,0,0,0,0,0,'','Brazie the Botanist - Show option while Flower Power is in progress'),
(15,12590,0,0,1,1,0,91683,1,0,1,0,0,'','Brazie the Botanist - Show option while no game runs'),
(15,12590,0,0,2,9,0,28744,0,0,0,0,0,'','Brazie the Botanist - Show option while Ghouls Hate My Grains is in progress'),
(15,12590,0,0,2,1,0,91683,1,0,1,0,0,'','Brazie the Botanist - Show option while no game runs'),
(15,12590,0,0,3,9,0,28747,0,0,0,0,0,'','Brazie the Botanist - Show option while Someone Setup the Pumpkin Bomb is in progress'),
(15,12590,0,0,3,1,0,91683,1,0,1,0,0,'','Brazie the Botanist - Show option while no game runs'),
(15,12590,0,0,4,9,0,28748,0,0,0,0,0,'','Brazie the Botanist - Show option while Lawn of the Dead is in progress'),
(15,12590,0,0,4,1,0,91683,1,0,1,0,0,'','Brazie the Botanist - Show option while no game runs'),
(15,12590,0,0,5,9,0,28751,0,0,0,0,0,'','Brazie the Botanist - Show option while Tending the Garden is in progress'),
(15,12590,0,0,5,1,0,91683,1,0,1,0,0,'','Brazie the Botanist - Show option while no game runs');

-- A Freezya Seeds sack in Orgrimmar (sniff artifact of the minigame's summon); it would run the sack's script and spellclick
DELETE FROM `creature_addon` WHERE `guid`=372568;
DELETE FROM `creature` WHERE `guid`=372568 AND `id`=49604;

-- Ours: group members see each other's suns and seed sacks; only the game's own player may click them up
DELETE FROM `spell_script_names` WHERE `ScriptName`='spell_brazie_pick_up';
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(91687,'spell_brazie_pick_up'),
(92434,'spell_brazie_pick_up'),
(92339,'spell_brazie_pick_up');

-- Brazie the Botanist (49176, the vehicle): rogue class for an energy (Solar Power) bar, no client-side power regeneration
UPDATE `creature_template` SET `unit_class`=4, `unit_flags2`=0, `ScriptName`='npc_brazie_the_botanist_vehicle' WHERE `entry`=49176;
-- Fertilize-o-tron 2000
UPDATE `creature_template` SET `ScriptName`='npc_brazie_fertilize_o_tron_2000' WHERE `entry`=49192;
-- Spitter Target, Goal Marker, Place it here!, Solar Power, Freezya Seeds, Strangler Vine Seeds
UPDATE `creature_template` SET `ScriptName`='npc_brazie_vehicle_notifier' WHERE `entry` IN (49385,49210,51098,49194,49604,49560);
-- Empty Spot. Ours: no VehicleId (mop has 1391 from the sniff; the spot turns into the plant by UpdateEntry, no seat is used)
UPDATE `creature_template` SET `ScriptName`='npc_brazie_spot', `unit_flags`=256, `BaseAttackTime`=500, `RegenHealth`=0, `VehicleId`=0 WHERE `entry`=49190;
-- Spitter, Freezya
UPDATE `creature_template` SET `minlevel`=20, `maxlevel`=20, `unit_flags`=256, `faction`=2263, `BaseAttackTime`=500, `RegenHealth`=0 WHERE `entry` IN (49200,49618);
-- Sunflower; ours, the same for Rocknut, Strangler Vine and Pumpkin Bomb
UPDATE `creature_template` SET `minlevel`=20, `maxlevel`=20, `unit_flags`=256, `faction`=2263, `RegenHealth`=0 WHERE `entry` IN (49193,49205,49207,49474);
-- Zombie, Ghoul (TCPP's DamageModifier 3 and 6 are set by the script); ours: Abomination, Warden Stillwater
UPDATE `creature_template` SET `RegenHealth`=0, `ScriptName`='npc_brazie_zombie' WHERE `entry` IN (49209,49232,49694,49819);
-- Obelisk (ours)
UPDATE `creature_template` SET `RegenHealth`=0, `ScriptName`='npc_brazie_obelisk' WHERE `entry`=49911;

DELETE FROM `creature_template_addon` WHERE `entry` IN (51098,49194,49604,49560);
INSERT INTO `creature_template_addon` (`entry`,`auras`) VALUES
(51098,'94025'),       -- Place it here!: Highlight (DND)
(49194,'91707'),       -- Solar Power: Sun Sphere
(49604,'92373 92406'), -- Freezya Seeds: Sack Visual, Cosmetic - Loot Sparkles
(49560,'92373 92406'); -- Strangler Vine Seeds: as the Freezya Seeds (ours)

-- Reach of the plants and zombies (TCPP); Solar Power and the seed sacks (36401) already have 200 so they can be clicked from
-- the vehicle. Ours: the same for Strangler Vine, Pumpkin Bomb and Broken Rocknut; Abomination and Warden Stillwater (2.5/11
-- and 3/18) get a smaller size and reach so they stay in their lane.
UPDATE `creature_model_info` SET `combat_reach`=3, `bounding_radius`=1.05 WHERE `modelid` IN (36407,36888,36890,36557,36891,36889,36870,36494,36607);
UPDATE `creature_model_info` SET `combat_reach`=1.33, `bounding_radius`=0.4655 WHERE `modelid`=37155;
UPDATE `creature_model_info` SET `combat_reach`=3, `bounding_radius`=1.5 WHERE `modelid` IN (36886,36650);

-- Plant spells hit an Empty Spot; Plague Gas (the lawnmower) kills the undead and plants in its lane, but not Warden Stillwater
-- (ours, 100% of his health). The drops and Warden Stillwater come at the nearest lane end (Spitter Target; TCPP moves the
-- Freezya sack there by script, this core calls no destination hook for these targets). Pumpkin Explosion hits the undead and
-- obelisks (ours).
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry` IN (91649,91646,92440,91704,91710,92157,92084,92340,92435,92767,92159);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,2,91649,0,0,31,0,3,49190,0,0,0,0,'','Plant Spitter - Target Empty Spot'),
(13,2,91646,0,0,31,0,3,49190,0,0,0,0,'','Plant Sunflower - Target Empty Spot'),
(13,2,92440,0,0,31,0,3,49190,0,0,0,0,'','Plant Freezya - Target Empty Spot'),
(13,2,91704,0,0,31,0,3,49190,0,0,0,0,'','Plant Rocknut - Target Empty Spot'),
(13,2,91710,0,0,31,0,3,49190,0,0,0,0,'','Plant Strangler Vine - Target Empty Spot'),
(13,2,92157,0,0,31,0,3,49190,0,0,0,0,'','Plant Pumpkin Bomb - Target Empty Spot'),
(13,1,92084,0,0,31,0,3,49209,0,0,0,0,'','Plague Gas - Target Zombie'),
(13,1,92084,0,1,31,0,3,49232,0,0,0,0,'','Plague Gas - Target Ghoul'),
(13,1,92084,0,2,31,0,3,49694,0,0,0,0,'','Plague Gas - Target Abomination'),
(13,1,92084,0,3,31,0,3,49911,0,0,0,0,'','Plague Gas - Target Obelisk'),
(13,2,92084,0,0,31,0,3,49193,0,0,0,0,'','Plague Gas - Target Sunflower'),
(13,2,92084,0,1,31,0,3,49200,0,0,0,0,'','Plague Gas - Target Spitter'),
(13,2,92084,0,2,31,0,3,49618,0,0,0,0,'','Plague Gas - Target Freezya'),
(13,2,92084,0,3,31,0,3,49205,0,0,0,0,'','Plague Gas - Target Rocknut'),
(13,2,92084,0,4,31,0,3,49207,0,0,0,0,'','Plague Gas - Target Strangler Vine'),
(13,2,92084,0,5,31,0,3,49474,0,0,0,0,'','Plague Gas - Target Pumpkin Bomb'),
(13,1,92340,0,0,31,0,3,49385,0,0,0,0,'','Create Random Strangler Vine - Target Spitter Target'),
(13,1,92435,0,0,31,0,3,49385,0,0,0,0,'','Create Random Freezya Sack - Target Spitter Target'),
(13,1,92767,0,0,31,0,3,49385,0,0,0,0,'','Create Warden Stillwater - Target Spitter Target'),
(13,1,92159,0,0,31,0,3,49209,0,0,0,0,'','Pumpkin Explosion - Target Zombie'),
(13,1,92159,0,1,31,0,3,49232,0,0,0,0,'','Pumpkin Explosion - Target Ghoul'),
(13,1,92159,0,2,31,0,3,49694,0,0,0,0,'','Pumpkin Explosion - Target Abomination'),
(13,1,92159,0,3,31,0,3,49819,0,0,0,0,'','Pumpkin Explosion - Target Warden Stillwater'),
(13,1,92159,0,4,31,0,3,49911,0,0,0,0,'','Pumpkin Explosion - Target Obelisk'),
(13,2,92159,0,0,31,0,3,49209,0,0,0,0,'','Pumpkin Explosion - Target Zombie'),
(13,2,92159,0,1,31,0,3,49232,0,0,0,0,'','Pumpkin Explosion - Target Ghoul'),
(13,2,92159,0,2,31,0,3,49694,0,0,0,0,'','Pumpkin Explosion - Target Abomination'),
(13,2,92159,0,3,31,0,3,49819,0,0,0,0,'','Pumpkin Explosion - Target Warden Stillwater'),
(13,2,92159,0,4,31,0,3,49911,0,0,0,0,'','Pumpkin Explosion - Target Obelisk');

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_brazie_create_botanists_vehicle','spell_brazie_summon_plant','spell_brazie_spit');
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(91683,'spell_brazie_create_botanists_vehicle'),
(91649,'spell_brazie_summon_plant'),
(91646,'spell_brazie_summon_plant'),
(92440,'spell_brazie_summon_plant'),
(91704,'spell_brazie_summon_plant'),
(91710,'spell_brazie_summon_plant'),
(92157,'spell_brazie_summon_plant'),
(91748,'spell_brazie_spit'),
(92441,'spell_brazie_spit');

-- Venom Spit and Freezya Blast hit along the lane (TCPP sets this in SpellMgr). Ours: the player's auras of the game (infinite
-- duration) are not saved, as the player logs out (SaveToDB) before leaving the vehicle; a stale progress bar would stay.
DELETE FROM `spell_custom_attribute` WHERE `spell` IN (91748,92441,91683,92259,91651,92321,92597,92598);
INSERT INTO `spell_custom_attribute` (`spell`,`attribute`,`comment`) VALUES
(91748,'SPELL_ATTR0_CU_CONE_LINE','Venom Spit (Plants vs. Ghouls)'),
(92441,'SPELL_ATTR0_CU_CONE_LINE','Freezya Blast (Plants vs. Ghouls)'),
(91683,'SPELL_ATTR0_CU_DONT_SAVE_AURA_TO_DB','Create Botanist''s Vehicle (Plants vs. Ghouls)'),
(92259,'SPELL_ATTR0_CU_DONT_SAVE_AURA_TO_DB','Progress Bar (Plants vs. Ghouls)'),
(91651,'SPELL_ATTR0_CU_DONT_SAVE_AURA_TO_DB','Progress Bar (Plants vs. Ghouls)'),
(92321,'SPELL_ATTR0_CU_DONT_SAVE_AURA_TO_DB','Progress Bar (Plants vs. Ghouls)'),
(92597,'SPELL_ATTR0_CU_DONT_SAVE_AURA_TO_DB','Progress Bar (Plants vs. Ghouls)'),
(92598,'SPELL_ATTR0_CU_DONT_SAVE_AURA_TO_DB','Progress Bar (Plants vs. Ghouls: Warden Stillwater)');

-- Solar Power and the seed sacks are picked up by clicking them from the vehicle. The Strangler Vine Seeds cast the drop spell
-- (92340) again on a click in mop; their pickup is Get Strangler Seeds (92339), as Get Freezya Seeds for the Freezya sack.
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` IN (49194,49604,49560);
INSERT INTO `npc_spellclick_spells` (`npc_entry`,`spell_id`,`cast_flags`,`user_type`) VALUES
(49194,91687,1,0),
(49604,92434,1,0),
(49560,92339,1,0);

-- TCPP's texts; ours: the lost-game emote only reaches the player (TextRange 5), every actor being theirs alone. Warden
-- Stillwater's arrival and death lines (broadcast_text), also to the player alone.
DELETE FROM `creature_text` WHERE `CreatureID` IN (49176,49819);
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(49176,0,0,'You gained Freeza seeds!',42,0,100,0,0,0,0,49686,0,'Brazie the Botanist to Player'),
(49176,1,0,'You gained Strangler Vine seeds!',42,0,100,0,0,0,0,49630,0,'Brazie the Botanist to Player'),
(49176,2,0,'Zombies ate your brains!',41,0,100,0,0,14560,0,49692,5,'Brazie the Botanist to Player'),
(49819,0,0,'Enough fooling around. It''s time for some hands-on management!',12,0,100,0,0,0,0,49986,5,'Warden Stillwater - Lawn of the Dead arrival'),
(49819,1,0,'What?! No! This lawn... was... mine...',12,0,100,0,0,0,0,49987,5,'Warden Stillwater - Lawn of the Dead death');
