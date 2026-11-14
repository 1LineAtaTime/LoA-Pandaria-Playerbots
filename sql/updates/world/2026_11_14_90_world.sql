-- Eastern Plaguelands: creatures posed as dead no longer wander.
--
-- 28 spawns in the zone lie dead (Permanent Feign Death 29266 on the spawn, or StandState 7 on the spawn or its template) and
-- had MovementType 1 with a wander distance of 3 yd, so the bodies slid about: the four nerubians beside the cocooned Gidwin
-- Goldbraids (Just Encased), eight dead Wretched at Quel'Lithien Lodge, eleven cosmetic Stitched Horror and Cannibal Ghoul
-- corpses, and five Corpsebeast remains in the Plaguewood. Reported in game for Crypt Horror 212069.
--
-- Sourced: the TDB spawns of all ten entries are idle (MovementType 0, wander 0; both dumps hold the same rows). Ours: nothing.

UPDATE `creature` SET `MovementType`=0, `wander_distance`=0 WHERE `guid` IN (
    212067, 212068, 212069, 212070,                                             -- Crypt Stalker, Slayer, Horror, Walker by Gidwin
    174595, 174761, 175045, 175074, 175115, 175205, 175097, 175098,             -- Wretched Ranger, Wretched Pathstrider
    173887, 173888, 173889, 173892, 173896, 174023,                             -- Stitched Horror (Cosmetic)
    173890, 173891, 173893, 173894, 173895,                                     -- Cannibal Ghoul (Cosmetic)
    172891, 172923, 172765, 172894, 172920)                                     -- Corpsebeast (Dog), Corpsebeast (Blob)
    AND `id` IN (8555, 8556, 8557, 8558, 8564, 8565, 41177, 41195, 45893, 45895);
