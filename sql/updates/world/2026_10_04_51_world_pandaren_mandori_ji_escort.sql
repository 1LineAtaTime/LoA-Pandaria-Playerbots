-- Quest 29792 "Bidden to Greatness" (Wandering Isle, epic #440): the Mandori escort Ji Firepaw (59988, summoned by
-- AreaTrigger_at_mandori together with Aysa 59986 and Jojo 59989) carried ScriptName 'npc_ji_forest_escort'. That
-- C++ AI belongs to the Pei-Wu forest Ji of "Wrecking the Wreck" (it jumps to 429,3674,78 and starts an escort that
-- has no script_waypoint rows for 59988), so the Mandori Ji leapt away from the group and its 16 SmartAI rows
-- (door dialogue, waypoint path 59988, hand-off to Korga) never ran. The MoP 5.4.8 data has
-- 59988 as SmartAI with an empty ScriptName. The objective credits (59946/59947) come from Aysa's SmartAI and are
-- unaffected. SQL-only.
--
-- Rollback: UPDATE `creature_template` SET `ScriptName` = 'npc_ji_forest_escort' WHERE `entry` = 59988;

UPDATE `creature_template` SET `AIName` = 'SmartAI', `ScriptName` = '' WHERE `entry` = 59988;
