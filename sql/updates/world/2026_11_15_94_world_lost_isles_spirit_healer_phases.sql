-- Lost Isles / Kezan: the 15 Spirit Healers (39660) on map 648 were phase 1 only. A player's spawn phase
-- falls back to phase 1 only while no phase definition is active, and every Lost Isles quest phase is
-- one, so a ghost could not see a healer there. Make them visible in every phase, as entry 6491 already is.
UPDATE `creature` SET `phaseMask` = 4294967295 WHERE `id` = 39660 AND `map` = 648 AND `phaseMask` = 1;
