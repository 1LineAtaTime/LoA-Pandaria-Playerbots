-- Northern Barrens, Echeyakee (quest 881): the lair stands where it is in 5.x.
-- Problem: the quest marker is right (client data, around 47.8, 44.0), but the lair object where the Horn of Echeyakee
--   works (164651, guid 66257) and the summon point of Echeyakee (event 2980) are still at the old lair, 590 yards east.
-- Change: lair object to the giant kodo skeletons at 47.8, 44.0 (world 124.6 -2544.4, ground 91.67, height measured in
--   game); summon point 33 yards from it, as far as it was from the old lair (the exact point is our choice).
-- Source: Wowhead MoP Classic quest 881, 5.x comment ("the white lion has moved house to 47.8, 44.0. You're looking for
--   a couple of large skeletons near the intersection of the road from Org and the North Gold Road ..."); quest text
--   ("among the bones of giant Kodo"); Wowhead map pin of npc 3475 at 47.8, 44.0.
-- Rollback:
--   UPDATE `gameobject` SET `position_x`=459.8,`position_y`=-3036,`position_z`=91.3 WHERE `guid`=66257 AND `id`=164651 AND `map`=1;
--   UPDATE `event_scripts` SET `x`=436.258,`y`=-3058,`z`=92.434,`o`=3.952 WHERE `id`=2980 AND `command`=10 AND `datalong`=3475;
UPDATE `gameobject` SET `position_x`=124.6,`position_y`=-2544.4,`position_z`=91.67 WHERE `guid`=66257 AND `id`=164651 AND `map`=1;
UPDATE `event_scripts` SET `x`=100.6,`y`=-2566.4,`z`=91.67,`o`=0.74 WHERE `id`=2980 AND `command`=10 AND `datalong`=3475;
