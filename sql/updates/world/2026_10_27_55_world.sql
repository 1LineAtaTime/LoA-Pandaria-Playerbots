-- Pyrewood's Fall (27550): Pyrewood Village only burns once the player has burned it
-- The quest's reward spell, Detect: Quest Invis Zone 16 (85598), lets the player see invisibility type 24 at strength 1, which
-- is what Apply Quest Invis Zone 16 (85599) puts on things, but nothing in the village had it: the sniffed fires (Fire 205577
-- across the village, 205578 in the chapel, inn and town hall) burned for everyone from the start, though the quest has the
-- player set the freshly taken village alight. No sniff has the fires' invisibility; give them type 24 at strength 1 so they
-- show once Pyrewood's Fall is turned in.
DELETE FROM `gameobject_addon` WHERE `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` IN (205577,205578));
INSERT INTO `gameobject_addon` (`guid`,`invisibilityType`,`invisibilityValue`)
SELECT `guid`,24,1 FROM `gameobject` WHERE `id` IN (205577,205578);
