-- Western Plaguelands: remove pre-Cataclysm quest objects outside Andorhal, and a fire under Darrowmere Lake.
-- Sourced: neither TDB (4.4.2, 1210) spawns any of these in the zone, and the quests they served have no starter left in mop:
--  * Blood of Heroes 176213 x11 (Weeping Cave, Felstone Field, Gahrron's Withering, the Writhing Haunt, Dalson's Farm, Northridge, Caer
--    Darrow and the fields): a chest whose only loot is Blood of Heroes 12938, an objective of the retired 5166, 5167 and 7483.
--  * Scarlet Crusade Forward Camp 176088 (39404, Charred Outpost): the spell focus of the retired Scarlet Diversions 5096.
--  * Northridge Lumber Mill Crate 177492 (5645): the spell focus of the retired A Plague Upon Thee 5903.
--  * Fire 165587 (184154): 89 yd under the bed of Darrowmere Lake (z -60).
-- None is pooled, in a game event or has an addon. The Blood of Heroes in Andorhal and the Eastern Plaguelands are left alone.
DELETE FROM `gameobject` WHERE (`guid`,`id`) IN ((7774,176213),(7775,176213),(7777,176213),(7778,176213),(7783,176213),(7784,176213),(7786,176213),
(7787,176213),(8186,176213),(9818,176213),(40452,176213),(39404,176088),(5645,177492),(184154,165587));
