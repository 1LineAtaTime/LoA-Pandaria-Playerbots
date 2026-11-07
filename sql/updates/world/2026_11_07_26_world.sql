-- Dun Garok: Captain Ironhill's Ghost wanders, staying inside trigger 6571. Sourced: "patrolling near the entrance", "wandering around there" (Wowhead 28635 comments, 2010) and four Wowhead
-- pins spread over about 13 x 20 yards (he is friendly, so they are not chases). Both TDBs have him static at the trigger's edge (12.1 yards), a spot neither pins nor comments single out.
-- Ours: random movement, the new home and the radius. The home is 6.2 yards from the trigger's centre (-1248.06,-1156.41,39.66) and the radius 5, so he stays within about 11.3 of its 12 yards
-- (3D, sampled on the terrain): walking right up to him means entering the trigger, which is what completes 28635. His area is smaller than the pins' spread for that reason.
UPDATE `creature` SET `position_x`=-1250,`position_y`=-1150.5,`position_z`=38.67,`MovementType`=1,`wander_distance`=5 WHERE `guid`=175948 AND `id`=49240;
