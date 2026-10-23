-- Refs #439, #473. Duskhaven: Duskhaven Watchman (36211) vs Forsaken Invader (34511) staged street fight
-- around quests 14347 "Hold the Line" / 14382 "Two By Sea". Audit row WQ003 (loa_port=YES, unit U-GILNEAS-SPARRING).
-- Companion C++ (NEEDS THE CORE BUILD): src/server/scripts/EasternKingdoms/zone_gilneas.cpp
-- npc_duskhaven_watchman / npc_forsaken_invader / npc_forsaken_footsoldier, ported from alexkulya
-- pandaria_5.4.8 commit 24ff2ce. Without that build the names are unknown and the NPCs keep no AI.
-- Fork today: both entries have ScriptName='' and AIName='' (no SmartAI, no creature_sparring_template), and
-- 40+ watchman/invader pairs are spawned 3-4 yd apart, i.e. the staged fight is clearly intended.
-- 36236 Forsaken Footsoldier is intentionally not bound (the reference DB does not bind it either; it is a
-- kill-credit source for 14369 "Unleash the Beast").
--
-- Rollback:
--   UPDATE `creature_template` SET `ScriptName` = '' WHERE `entry` IN (34511, 36211);
-- Re-runnable: plain UPDATEs.

UPDATE `creature_template` SET `ScriptName` = 'npc_forsaken_invader' WHERE `entry` = 34511;
UPDATE `creature_template` SET `ScriptName` = 'npc_duskhaven_watchman' WHERE `entry` = 36211;
