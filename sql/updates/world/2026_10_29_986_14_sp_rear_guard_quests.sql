-- ported from PR 15 2026_10_25_51_world.sql (#986)
-- Forsaken Rear Guard: Steel Thunder (27069), Lost in the Darkness (27093) and It's Only Poisonous if You Ingest It (27088) had
-- nothing behind them. Ported from TrinityCore: Admiral Hatchet lends an Orc Sea Pup on accepting Steel Thunder (or through his
-- gossip option) and takes its crates on turn-in (npc_silverpine_admiral_hatchet); a Sea Dog Crate puts an Orc Crate on the
-- player's pup and counts it (spell_silverpine_pick_up_orc_crate, npc_silverpine_orc_sea_pup, npc_silverpine_orc_crate); cutting a
-- Webbed Victim free may free an Orc Sea Dog that follows the player, or let out a foe (npc_silverpine_webbed_victim,
-- spell_silverpine_free_webbed_victim_random, npc_silverpine_orc_sea_dog); and a Mutant Bush Chicken released at a Forest Ettin is
-- eaten and explodes, weakening it (spell_silverpine_release_diseased_mutant_bush_chicken, npc_silverpine_mutant_bush_chicken,
-- npc_silverpine_forest_ettin, which also takes over Bonk and Log Smash from its SmartAI). The camp's talks come along: Hatchet
-- and Warlord Torok, Apothecary Wormcrud and the drunken sea dogs (at_silverpine_forsaken_rear_guard), and "Salty" Rocka and
-- "Salty" Gorgar.
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_admiral_hatchet' WHERE `entry` = 44916;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_apothecary_wormcrud' WHERE `entry` = 44912;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_salty_rocka' WHERE `entry` = 45498;
UPDATE `creature_template` SET `AIName` = '', `unit_flags` = 33288, `VehicleId` = 1060, `ScriptName` = 'npc_silverpine_orc_sea_pup' WHERE `entry` = 44914;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_orc_crate' WHERE `entry` = 44915;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_forest_ettin' WHERE `entry` = 44367;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_mutant_bush_chicken' WHERE `entry` = 44935;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_webbed_victim' WHERE `entry` = 44941;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_orc_sea_dog' WHERE `entry` = 44942;

DELETE FROM `smart_scripts` WHERE `entryorguid` = 44367 AND `source_type` = 0;

DELETE FROM `areatrigger_scripts` WHERE `entry` = 6222;
INSERT INTO `areatrigger_scripts` (`entry`, `ScriptName`) VALUES
(6222, 'at_silverpine_forsaken_rear_guard');

-- Hatchet's option for a new pup, while Steel Thunder is in progress
DELETE FROM `gossip_menu_option` WHERE `MenuID` = 11901;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcflag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES
(11901, 0, 0, 'I seem to have misplaced my sea pup, admiral. Do you have another that you could lend to me?', 45023, 1, 1, 0, 0, 0, 0, NULL, 0, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 11901;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 11901, 0, 0, 0, 9, 0, 27069, 0, 0, 0, 0, 0, '', 'Admiral Hatchet - Show sea pup option while Steel Thunder is in progress');

-- Pick Up Orc Crate on the pup; Bush Chicken Explosion! on the ettin only
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` IN (83838, 83903);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 83838, 0, 0, 31, 0, 3, 44914, 0, 0, 0, 0, '', 'Pick Up Orc Crate - Target Orc Sea Pup'),
(13, 5, 83903, 0, 0, 31, 0, 3, 44367, 0, 0, 0, 0, '', 'Bush Chicken Explosion! - Target Forest Ettin');

DELETE FROM `spell_script_names` WHERE `spell_id` IN (83838, 83902, 83919);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(83838, 'spell_silverpine_pick_up_orc_crate'),
(83902, 'spell_silverpine_release_diseased_mutant_bush_chicken'),
(83919, 'spell_silverpine_free_webbed_victim_random');

DELETE FROM `creature_text` WHERE `CreatureID` IN (45498, 45497, 44916, 44917, 44912, 44913, 44914, 44942);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(45498, 0, 0, 'Hey over there!  Hey!  Hey!  Over there!  How''sh everything going Gororgararar?', 14, 1, 100, 5, 0, 0, 0, 45623, 0, '"Salty" Rocka - Forsaken Rear Guard'),
(45498, 1, 0, 'HA! Shows how much you know.  I left the critter back on the third ship!', 14, 1, 100, 5, 0, 0, 0, 45634, 0, '"Salty" Rocka - Forsaken Rear Guard'),
(45498, 2, 0, 'Yoo hoo!  "Shmaltzy!"  Hey, why come your shupplies... Whe... Oh... Um... You got lots of boxes to keep track of...', 14, 1, 100, 5, 0, 0, 0, 45624, 0, '"Salty" Rocka - Forsaken Rear Guard'),
(45498, 3, 0, 'I didn''t like that pet anyways.  Beshides... People stop by all the time and give me their pets.  If I lost one, it doesn''t matter, I''ve got dozens more to lose!', 14, 1, 100, 5, 0, 0, 0, 45637, 0, '"Salty" Rocka - Forsaken Rear Guard'),
(45498, 4, 0, 'Hey over there!  Hey!  Hey!  Over there!  How''sh everything going Gororgararar?', 14, 1, 100, 5, 0, 0, 0, 45623, 0, '"Salty" Rocka - Forsaken Rear Guard'),
(45498, 5, 0, 'My pet! Oh no!  Uh... I am shure that pet is around here somewhere...', 14, 1, 100, 5, 0, 0, 0, 45633, 0, '"Salty" Rocka - Forsaken Rear Guard'),
(45498, 6, 0, 'Hey, "Salty" when are you going to get a new name? Thish town ain''t big ''nough for two of ussh...', 14, 1, 100, 5, 0, 0, 0, 45621, 0, '"Salty" Rocka - Forsaken Rear Guard'),
(45498, 7, 0, 'Wait... Where is my pet?  It''s gone! No need to panic... Just need a quick drink and I can figure out where he went to.', 14, 1, 100, 5, 0, 0, 0, 45636, 0, '"Salty" Rocka - Forsaken Rear Guard'),
(45498, 8, 0, '"Malty!"  Hey, "Malty!"  I thought you were going to get us shome more booze!  What''sh the hold up?', 14, 1, 100, 5, 0, 0, 0, 45626, 0, '"Salty" Rocka - Forsaken Rear Guard'),
(45498, 9, 0, 'I didn''t like that pet anyways.  Beshides... People stop by all the time and give me their pets.  If I lost one, it doesn''t matter, I''ve got dozens more to lose!', 14, 1, 100, 5, 0, 0, 0, 45637, 0, '"Salty" Rocka - Forsaken Rear Guard'),
(45497, 0, 0, 'Hey, shtop bothering me.  You''re a dishgrace to us all.  You can''t even hold onto your own petsh!', 14, 1, 100, 5, 0, 0, 0, 45631, 0, '"Salty" Gorgar - Forsaken Rear Guard'),
(45497, 1, 0, 'Maybe you should worry about your pet and not me!  I''m taking care of my bushinesh jusht fine!', 14, 1, 100, 5, 0, 0, 0, 45628, 0, '"Salty" Gorgar - Forsaken Rear Guard'),
(45497, 2, 0, 'Hey, shtop bothering me.  You''re a dishgrace to us all.  You can''t even hold onto your own petsh!', 14, 1, 100, 5, 0, 0, 0, 45631, 0, '"Salty" Gorgar - Forsaken Rear Guard'),
(45497, 3, 0, 'It''s "Salty!"  I got here firsht!  I got off the boat and onto land firsht!  I called the name "Salty."  Hey... Where''s your pet at?', 14, 1, 100, 5, 0, 0, 0, 45627, 0, '"Salty" Gorgar - Forsaken Rear Guard'),
(45497, 4, 0, '"Shalty."  That''s my name!  Your name should be "I can''t hold onto petsh."  I can''t believe anyone would trusht you with a beasht.', 14, 1, 100, 5, 0, 0, 0, 45629, 0, '"Salty" Gorgar - Forsaken Rear Guard'),
(44916, 0, 0, 'Who''s not drunk, Torok?', 12, 1, 100, 0, 0, 0, 0, 45018, 0, 'Admiral Hatchet - Forsaken Rear Guard'),
(44916, 1, 0, 'Yes.', 12, 1, 100, 0, 0, 0, 0, 45020, 0, 'Admiral Hatchet - Forsaken Rear Guard'),
(44917, 0, 0, 'Including me?', 12, 1, 100, 0, 0, 0, 0, 45019, 0, 'Warlord Torok - Forsaken Rear Guard'),
(44917, 1, 0, '<hic!> Well... Let''s see... By my count, nobody. The whole crew''s drunk out of their gourds, admiral... ''cept the sea pups... <hic!>', 12, 1, 100, 0, 0, 0, 0, 45021, 0, 'Warlord Torok - Forsaken Rear Guard'),
(44912, 0, 0, 'Touch my chicken and I will kill you in your sleep. Am I understood?', 12, 1, 100, 1, 0, 0, 0, 45069, 0, 'Apothecary Wormcrud - Forsaken Rear Guard'),
(44913, 0, 0, 'What''sh with the huuuuge bush chick-a-doodle?', 12, 1, 100, 1, 0, 0, 0, 45061, 0, 'Orc Sea Dog - Forsaken Rear Guard'),
(44913, 1, 0, 'It''sh makin'' me hungry... why''s it shooo big? You shome kind of idiot or shomething? Makin'' a chicken sho big... Who doesh that?', 12, 1, 100, 1, 0, 0, 0, 45062, 0, 'Orc Sea Dog - Forsaken Rear Guard'),
(44913, 2, 0, 'Forshaken do... that''sh who...It''sh sho big... Look at it''sh big shtupid neck. I jusht want to choke it to death!', 12, 1, 100, 1, 0, 0, 0, 45063, 0, 'Orc Sea Dog - Forsaken Rear Guard'),
(44914, 0, 0, 'Where to going, captain?', 12, 0, 100, 66, 0, 0, 0, 44990, 5, 'Orc Sea Pup - Steel Thunder'),
(44914, 1, 0, 'OOF! Maybe trow little softer, captain?', 12, 1, 100, 0, 0, 0, 0, 44992, 5, 'Orc Sea Pup - Steel Thunder'),
(44914, 2, 0, 'Heavy... no can see notin.', 12, 1, 100, 0, 0, 0, 0, 44993, 5, 'Orc Sea Pup - Steel Thunder'),
(44914, 3, 0, 'Captain, pup need help!', 12, 1, 100, 0, 0, 0, 0, 44996, 5, 'Orc Sea Pup - Steel Thunder'),
(44914, 4, 0, 'Please, captain, I can put box down?', 12, 1, 100, 0, 0, 0, 0, 44999, 5, 'Orc Sea Pup - Steel Thunder'),
(44914, 5, 0, 'NO MORE! DIS IS MAXI... MAXIMO... ME... DIS TOO MUCH!', 12, 1, 100, 0, 0, 0, 0, 45003, 5, 'Orc Sea Pup - Steel Thunder'),
(44914, 6, 0, 'Where to put? Dey falling! HELP!', 12, 1, 100, 0, 0, 0, 0, 45025, 5, 'Orc Sea Pup - Steel Thunder'),
(44914, 7, 0, 'OOF!', 12, 1, 100, 0, 0, 0, 0, 45024, 5, 'Orc Sea Pup - Steel Thunder'),
(44942, 0, 0, 'We gotta find the matriarch of this operation and end her!', 12, 1, 100, 5, 0, 0, 0, 45079, 5, 'Orc Sea Dog - Lost in the Darkness'),
(44942, 0, 1, 'Thank you, friend. Let''s kill us some overgrown... spiders. Sounded better in my head.', 12, 1, 100, 5, 0, 0, 0, 45078, 5, 'Orc Sea Dog - Lost in the Darkness'),
(44942, 0, 2, 'I need a drink!', 12, 1, 100, 5, 0, 0, 0, 45080, 5, 'Orc Sea Dog - Lost in the Darkness'),
(44942, 0, 3, 'Welcome to the party, $g pal:lady;!', 12, 1, 100, 5, 0, 0, 0, 45077, 5, 'Orc Sea Dog - Lost in the Darkness');
