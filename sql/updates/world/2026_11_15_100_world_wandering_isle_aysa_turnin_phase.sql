-- Wandering Isle, An Ancient Evil (29798): the quest is turned in to Aysa Cloudsinger (56416) at the Vordraka fight
-- site. Of her two spawns there, the phase-2 one fights Vordraka during the quest; the other is the one a player
-- turns in to afterwards, but it sat in phase 4, which a player at this point of the chain is not in, so after the
-- fight no Aysa could be seen and the quest could not be turned in. That spawn is phase 1 in the 5.4.8 reference data.
UPDATE `creature` SET `phaseMask` = 1 WHERE `guid` = 563254 AND `id` = 56416 AND `phaseMask` = 4;
-- Undo: UPDATE `creature` SET `phaseMask` = 4 WHERE `guid` = 563254 AND `id` = 56416 AND `phaseMask` = 1;
