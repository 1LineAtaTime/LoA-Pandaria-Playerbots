-- Echo Isles, quest 24814 "An Ancient Enemy": Vol'jin (38225) dealt no damage to Zar'jira (38306).
--
-- Why: the event is scripted twice. The complete SmartAI version (Vol'jin, Zar'jira, Zuni 38423, Vanira 38437, the
-- Fires of the Seas 38542, the Sea Witch Channel Bunny 38452, their action lists 38225xx/38306xx/38423xx/38437xx,
-- creature_text, waypoints 38225/382250/38437 and the spell conditions) is already in this database, but Vol'jin,
-- Zar'jira and the Fires still carried the older C++ ScriptNames, and a ScriptName wins over AIName. So the C++ AIs ran
-- for those three while Zuni, Vanira and the bunny ran SmartAI that waits for SetData calls the C++ never sends.
-- In the C++ version Zar'jira makes Vol'jin cast Deluge of Shadow (72044) when the fight starts. 72044 is a channelled
-- spell with infinite duration whose target the conditions limit to the Sea Witch Channel Bunny, and in the C++ flow
-- nothing ever kills the bunny. So Vol'jin channels for the whole fight, and every DoCastVictim(Shoot/Shadow Shock)
-- is skipped because he is casting (UnitAI::DoCastVictim returns while UNIT_STATE_CASTING is set).
-- The SmartAI version runs the event as it is described on Wowpedia ("An Ancient Enemy": the battle, the
-- Manifestations, the ice and the drain from the fires, stamping out the braziers, Zuni's death and the outro). Vol'jin
-- shoots and Shadow Shocks Zar'jira in phase 2, and his channel ends when the bunny dies after the fires are stamped out.
--
-- Fix 1: clear the three ScriptNames so that the existing SmartAI runs (the C++ classes in zone_azshara.cpp stay
-- registered but are no longer bound).
UPDATE `creature_template` SET `ScriptName` = '' WHERE `entry` = 38225 AND `AIName` = 'SmartAI' AND `ScriptName` = 'npc_voljin_ancient_enemy';
UPDATE `creature_template` SET `ScriptName` = '' WHERE `entry` = 38306 AND `AIName` = 'SmartAI' AND `ScriptName` = 'npc_zarjira';
UPDATE `creature_template` SET `ScriptName` = '' WHERE `entry` = 38542 AND `AIName` = 'SmartAI' AND `ScriptName` = 'npc_fire_of_the_seas';

-- Fix 2: Zar'jira's "Between 0-2% Health" chain was broken. Row 29 had no link and row 30 had event type 21 (Reached
-- Home) instead of 61 (Link), although its comment reads "Between 0-2% Health (Link) - Start Script". So action list
-- 3830604 (kill the channel bunny, which ends Vol'jin's second Deluge of Shadow, and face Zuni) ran only when she
-- reached home after an evade.
UPDATE `smart_scripts` SET `link` = 30 WHERE `entryorguid` = 38306 AND `source_type` = 0 AND `id` = 29 AND `link` = 0 AND `event_type` = 2;
UPDATE `smart_scripts` SET `event_type` = 61 WHERE `entryorguid` = 38306 AND `source_type` = 0 AND `id` = 30 AND `event_type` = 21 AND `action_type` = 80 AND `action_param1` = 3830604;

-- Undo:
-- UPDATE `creature_template` SET `ScriptName` = 'npc_voljin_ancient_enemy' WHERE `entry` = 38225;
-- UPDATE `creature_template` SET `ScriptName` = 'npc_zarjira' WHERE `entry` = 38306;
-- UPDATE `creature_template` SET `ScriptName` = 'npc_fire_of_the_seas' WHERE `entry` = 38542;
-- UPDATE `smart_scripts` SET `link` = 0 WHERE `entryorguid` = 38306 AND `source_type` = 0 AND `id` = 29;
-- UPDATE `smart_scripts` SET `event_type` = 21 WHERE `entryorguid` = 38306 AND `source_type` = 0 AND `id` = 30;
