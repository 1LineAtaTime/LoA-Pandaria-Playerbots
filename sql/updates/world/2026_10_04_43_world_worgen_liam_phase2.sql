-- Refs #439. NEEDS THE BUILD that carries zone_gilneas.cpp npc_prince_liam_greymane_phase2 (build 8+).
-- Prince Liam Greymane 34913 (Gilneas City, phase 2 after 14078 "Lockdown!") is killed by the Rampaging Worgen 34884
-- around him (40 spawns in phase 2, hostile factions 2174 vs 2173; Liam is level 5 with no AI in this fork), so he is
-- dead most of the time. Evidence: t439 cp2 and cp2b (relog) on builds 6 and 7, npcguid 34913 alive -> none.
-- The intended script for 34913 is npc_prince_liam_greymane_phase2 (mopcore gilneas.cpp:88-205),
--  ported into zone_gilneas.cpp (cosmetic fight, cannot lose to the worgen).
--
-- Rollback:
--   UPDATE `creature_template` SET `ScriptName` = '' WHERE `entry` = 34913;
-- Re-runnable: UPDATE.

UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_prince_liam_greymane_phase2' WHERE `entry` = 34913;
