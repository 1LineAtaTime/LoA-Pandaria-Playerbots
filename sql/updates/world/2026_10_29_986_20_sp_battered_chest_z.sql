-- ported from PR 15 2026_10_26_11_world.sql (#986)
-- Battered Chest at the Forsaken Rear Guard: the old Ivar Patch spawn kept its pre-Cataclysm height and floated above the ground
UPDATE `gameobject` SET `position_z` = 27.943 WHERE `guid` = 1468;
