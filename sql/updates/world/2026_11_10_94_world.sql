-- Araj the Summoner (26925/27164): Summon Berserkers 83701 raises Stickbone Berserkers instead of doing nothing.
-- Sourced: the TDB (4.4.2, 12.1.0) SmartAI (Frostbolt 9672, Frost Nova 75062 within 8 yd, 83701 after 18-23 s and every
-- 50-65 s, Araj's Phylactery 18661 on death), now ported to npc_wpl_araj_the_summoner; 83701 is a bare dummy (Wowhead:
-- "server-side script"); 82875 Summon Stickbone Berserker: Missile (random spot within 10 yd, triggers 82861: 44329, 40 s).
-- Wowhead comments: "When you fight Araj he/she will summon skeletons periodically", "when his first batch of Berserkers were
-- summoned, I tossed a grenade, killing them all", "it's looting before he respawns, with all those skeletons beating on you".
-- Respawn 120 s as in both TDBs and the TCPP sniff ("Loot it quickly the respawn is quick").
-- A retail video the user found shows Araj summoning his berserkers much like this.
-- Ours: 83701 casting 82875, 3 berserkers per cast, a safety cap of 6 (never reached: a batch lives 40 s, the next comes
-- 50-65 s later), the berserkers joining his fight, outliving his death and despawning when he evades.

UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_wpl_araj_the_summoner' WHERE `entry`=1852;
DELETE FROM `smart_scripts` WHERE `entryorguid`=1852 AND `source_type`=0;
UPDATE `creature` SET `spawntimesecs`=120 WHERE `guid`=213178 AND `id`=1852;
