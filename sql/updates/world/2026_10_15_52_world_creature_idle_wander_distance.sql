-- Refs #186 (misc boot-time data mismatches: idle creatures with a wander distance)
-- ERROR CLASS: "Table `creature` have creature (GUID: N Entry: 43704) with `movement_type`=0 (idle) have `wander_distance`<>0, set to 0." (ObjectMgr.cpp) - 6 lines.
-- The core already zeroes the value at load; this writes the same value to the table so the line disappears (no runtime change).
-- ROWS: 6 creature rows (guid 170667, 170670, 170671, 170674, 170676, 170677; all entry 43704, MovementType 0).
-- VERIFY: SELECT guid,MovementType,wander_distance FROM creature WHERE guid IN (170667,170670,170671,170674,170676,170677); -- wander_distance 0
-- EXPECTED DBErrors.log DROP: 6 lines.
-- UNDO: none needed (the loader forces 0 anyway); original values were non-zero wander_distance on idle spawns.
UPDATE `creature` SET `wander_distance`=0 WHERE `guid` IN (170667,170670,170671,170674,170676,170677) AND `MovementType`=0 AND `wander_distance`<>0;
