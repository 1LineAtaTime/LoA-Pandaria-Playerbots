-- Eastern Plaguelands: a Shallow Grave in Zul'Mashar raises its Infected Mossflayer for a player 4 yd away, not 8.
--
-- Reported in game: the Infected Mossflayer spawns feel very excessive. Each Shallow Grave 177677 (a trap) raises one troll
-- for the first player in reach and re-arms after 5 minutes, as built; but 22 of the 38 graves lie in one patch of 30 by 30 yd,
-- 4 to 6 yd apart, and the reach was 8 yd, so a few steps into the patch raised most of it.
--
-- Sourced: "every grave you step on spawns one" (Wowhead, quest 27481, 2010-12-03 [+9]): one grave, one troll, for the grave
--   stepped on. The template's data2 is 8 in both TDBs; TrinityCore reads that field as a diameter and halves it
--   (GameObject.cpp: trap.diameter / 2), which gives 4 yd. This core uses the value as the radius (core issue C22).
-- Ours: the value 4 on this template instead of a core change that would halve every trap in the world.
-- Kept: the 300 s re-arm (both TDBs have 120 s; the longer one is the less crowded).

UPDATE `gameobject_template` SET `data2`=4 WHERE `entry`=177677 AND `type`=6;
