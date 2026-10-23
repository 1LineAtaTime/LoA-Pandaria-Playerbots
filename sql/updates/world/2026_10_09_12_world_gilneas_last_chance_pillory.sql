-- Refs #504, #439. 14375 "Last Chance at Humanity": the Duskhaven pillory scene. NEEDS the build that carries
-- npc_krennan_aranas_pillory and spell_gilneas_forcecast_worgen_intro_completion (zone_gilneas.cpp, build 27+).
--
-- Intended (Wowpedia "Last Chance at Humanity", MediaWiki API read 2026-10-09): the player wakes locked in a pillory
-- before Krennan Aranas, Lord Godfrey and King Genn Greymane; Krennan "I am not giving up on you...", Godfrey "Give it
-- up, Krennan...", Genn "Tell me, Godfrey..." / "...double the dosage."; after the quest the three disappear and the
-- player is released.
--
-- Cause (fork today): the 14222 reward (72799 -> 72788) only teleports the player to the pillory. Nothing casts 68630
-- "Curse of the Worgen" (teleport + linked 68631 stun/invisibility + 69123 summon Krennan 36331 as a personal spawn,
-- SummonProperties 3054 flags 0x22918), 36331 has no AI, and the 14375 reward 68639 has a SCRIPT_EFFECT (value 68630)
-- that nothing handles. Intended: 36331 = npc_trigger_event_first_c3 (mopcore gilneas.cpp:2700-2829
-- is the same lineage), 69123 = spell_curse_of_the_worgen_summon; no row in the MoP 5.4.8 data casts 68630 either.
--
-- Fix:
--   * spell_area: autocast 68630 in area 4786 (Duskhaven, where the 14222 reward teleports the player) from 14222
--     rewarded (status mask 64) until 14375 rewarded (allowed 14375 states NONE|COMPLETE|INCOMPLETE = 11, the mask
--     the existing Gilneas rows use). Player::UpdateAreaDependentAuras casts it on arrival, and removes it if the
--     player leaves the area. The spell's own teleport puts him in the pillory.
--   * 36331 -> npc_krennan_aranas_pillory (the dialogue; personal Godfrey summon; leaves when the curse goes).
--   * 68639 -> spell_gilneas_forcecast_worgen_intro_completion (effect 1 removes 68630 = frees the player).
-- The fork's static Krennan 371633 (phase 1) is left alone: the Duskhaven player is in phase 4096.
--
-- Backup (state before this file, run before applying):
--   mysqldump -h<host> -u<user> -p<password> world spell_area spell_script_names --where="spell=68630 OR spell_id=68639" (empty today: 0 rows)
--   mysqldump -h<host> -u<user> -p<password> world creature_template --where="entry=36331" (ScriptName '' today)
-- Rollback:
--   DELETE FROM `spell_area` WHERE `spell` = 68630 AND `area` = 4786;
--   DELETE FROM `spell_script_names` WHERE `spell_id` = 68639 AND `ScriptName` = 'spell_gilneas_forcecast_worgen_intro_completion';
--   UPDATE `creature_template` SET `ScriptName` = '' WHERE `entry` = 36331;
-- Re-runnable: DELETE-then-INSERT + UPDATE.

DELETE FROM `spell_area` WHERE `spell` = 68630 AND `area` = 4786;
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `autocast`, `quest_start_status`, `quest_end_status`) VALUES
(68630, 4786, 14222, 14375, 0, 0, 2, 1, 64, 11);

UPDATE `creature_template` SET `ScriptName` = 'npc_krennan_aranas_pillory' WHERE `entry` = 36331;

DELETE FROM `spell_script_names` WHERE `spell_id` = 68639 AND `ScriptName` = 'spell_gilneas_forcecast_worgen_intro_completion';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(68639, 'spell_gilneas_forcecast_worgen_intro_completion');
