-- Triage (6622): remove the 14 static Injured, Badly Injured and Critically Injured Soldiers lying on Doctor Gregory Victor's bunks in Hammerfall.
-- Sourced: neither TDB 442 nor TDB 1210 spawns 12923-12925; npc_doctor summons a patient on each of these six bunks (HordeCoords in npcs_special.cpp) during the quest.
-- The static copies have no DoctorGUID, so they bleed out and respawn forever and count for nothing when bandaged. No addon, SmartAI, event, pool, formation or linked-respawn rows refer to them.
DELETE FROM `creature` WHERE `guid` BETWEEN 312400 AND 312413 AND `id` IN (12923,12924,12925);
