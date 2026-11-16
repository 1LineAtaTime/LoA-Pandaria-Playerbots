-- Wandering Isle: undo the phase change of the previous Aysa update. Aysa Cloudsinger spawn 563254 is the Aysa who
-- offers Risking It All (30767) after An Ancient Evil (29798) is turned in: the turn-in puts the player in the phase-4
-- aura (59074), and in this core a phase aura replaces the default phase, so the spawn has to stay in phase 4. The
-- 29798 turn-in problem the previous update was meant to fix was the quest's phase-2 aura ending too early, fixed by
-- 2026_11_16_00_world_wandering_isle_ancient_evil_phase.sql.
UPDATE `creature` SET `phaseMask` = 4 WHERE `guid` = 563254 AND `id` = 56416 AND `phaseMask` = 1;
