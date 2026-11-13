-- Follow-up for deployments that applied SQL33 before action46 translation.
-- This engine's action114 moves in an actor-relative frame: target_y is forward.
-- Preserve the authored two-yard escape and its original pathfinding MovePoint.
UPDATE `smart_scripts` SET `action_type`=114,`action_param1`=0,`target_y`=2
WHERE `entryorguid`=4264500 AND `source_type`=9 AND `id`=1
AND `action_type`=46 AND `action_param1`=2 AND `target_type`=1;
