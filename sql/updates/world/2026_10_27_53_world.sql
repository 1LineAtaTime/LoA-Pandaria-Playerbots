-- Lord Godfrey 45878, Lord Walden 45879 and Baron Ashbury 45880 give and end the Pyrewood quests. Every Horde character got them
-- (Summon Godfrey/Walden/Ashbury on spell_area with no quest) in Pyrewood and the two base camps only, and a SmartAI event killed
-- them anywhere else, also inside the Pyrewood houses and in the Battlefront marsh where their quests are done.
-- Battlefront Summon Master Aura (85516) now keeps them with the player in Silverpine Forest from Taking the Battlefront (27542)
-- until On Her Majesty's Secret Service (27594) is turned in; its script summons the player's own trio whenever one is missing.
DELETE FROM `spell_area` WHERE `spell` IN (85516, 85517, 85518, 85519);
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `autocast`, `quest_start_status`, `quest_end_status`) VALUES
(85516, 130, 27542, 27594, 0, 33555378, 2, 1, 64, 11);

DELETE FROM `spell_script_names` WHERE `spell_id` = 85516;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(85516, 'spell_silverpine_battlefront_summon_master_aura');

-- "IC or OOC - Die" outside areas 204, 5480 and 5481 (their "On Death - Despawn" stays)
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (45878, 45879, 45880) AND `source_type` = 0 AND `id` = 0;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceGroup` = 1 AND `SourceEntry` IN (45878, 45879, 45880) AND `SourceId` = 0;
