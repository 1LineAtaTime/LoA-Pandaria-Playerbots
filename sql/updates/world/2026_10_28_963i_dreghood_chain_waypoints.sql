-- #963 (2026-10-28, run v9 DBErrors.log):
-- * 16937 Dreghood Geomancer / 16938 Dreghood Brute (2026_10_28_963d chain): row 5 used SMART_ACTION_EVADE (24). On this core
--   EnterEvadeMode resets the SmartAI after the UPDATE_TEMPLATE to 19477, so the rest of the linked chain (say, random
--   movement, despawn) was lost: "Entry 16937 ... Event 5, Link Event 6 not found or invalid" and the fleeing warrior
--   never despawned. Use SMART_ACTION_COMBAT_STOP (203 on this core, target self) to end combat without the reset.
-- * 11256 Lieutenant Valorcall (2612) / 111375 Putridus the Ancient (32487): MovementType 2 without a waypoint path
--   ("doesn't have waypoint path id: 0") -> stationary, as 963b / 963e did (AC has paths for these under other guids;
--   porting them by position is a #963 follow-up).
-- Idempotent. Undo: action_type 24 on rows 16937/5 and 16938/5; MovementType 2 on the two spawns.
UPDATE `smart_scripts` SET `action_type` = 203, `target_type` = 1 WHERE `entryorguid` IN (16937, 16938) AND `source_type` = 0 AND `id` = 5 AND `action_type` = 24;
UPDATE `creature` SET `MovementType` = 0 WHERE `guid` IN (11256, 111375) AND `MovementType` = 2;
