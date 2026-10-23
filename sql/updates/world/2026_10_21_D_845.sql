-- Refs #845 (reopens the #711 shelved port): ScriptNames for the escort NPCs whose C++ AI is ported in
-- zone_hinterlands / zone_redridge_mountains / zone_stormwind_city / zone_swamp_of_sorrows / zone_western_plaguelands / zone_wetlands.cpp
-- (AC / TC sources; the MoP 5.4.8 data names the same ScriptNames). Apply together with the build that contains those scripts.
-- 3849 / 3850 (old Shadowfang Keep prisoners; need TYPE_FREE_NPC instance data the Cata-era SFK script lacks) and 6575 (Scarlet Trainee,
-- only summoned by the old Herod encounter) have no C++ and no AC script_waypoint rows (AC drives them with SmartAI / not at all): their
-- orphan script_waypoint rows are dropped, which silences the last 3 of the 10 DBErrors "no ScriptName" groups (269 lines).
-- Undo: UPDATE creature_template SET ScriptName='' WHERE entry IN (349,1754,1755,4962,4963,5391,7766,7780,8856,17238);
--       the deleted script_waypoint rows are in sql/base or the backup of the DB the PM takes before applying.
UPDATE `creature_template` SET `ScriptName` = 'npc_corporal_keeshan'      WHERE `entry` = 349   AND `ScriptName` = '';
UPDATE `creature_template` SET `ScriptName` = 'npc_lord_gregor_lescovar'  WHERE `entry` = 1754  AND `ScriptName` = '';
UPDATE `creature_template` SET `ScriptName` = 'npc_marzon_silent_blade'   WHERE `entry` = 1755  AND `ScriptName` = '';
UPDATE `creature_template` SET `ScriptName` = 'npc_tapoke_slim_jahn'      WHERE `entry` = 4962  AND `ScriptName` = '';
UPDATE `creature_template` SET `ScriptName` = 'npc_mikhail'               WHERE `entry` = 4963  AND `ScriptName` = '';
UPDATE `creature_template` SET `ScriptName` = 'npc_galen_goodward'        WHERE `entry` = 5391  AND `ScriptName` = '';
UPDATE `creature_template` SET `ScriptName` = 'npc_tyrion'                WHERE `entry` = 7766  AND `ScriptName` = '';
UPDATE `creature_template` SET `ScriptName` = 'npc_rinji'                 WHERE `entry` = 7780  AND `ScriptName` = '';
UPDATE `creature_template` SET `ScriptName` = 'npc_tyrion_spybot'         WHERE `entry` = 8856  AND `ScriptName` = '';
UPDATE `creature_template` SET `ScriptName` = 'npc_anchorite_truuen'      WHERE `entry` = 17238 AND `ScriptName` = '';
DELETE FROM `script_waypoint` WHERE `entry` IN (3849, 3850, 6575);
