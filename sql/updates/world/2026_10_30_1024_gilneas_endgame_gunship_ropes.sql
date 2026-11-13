-- #1024: 26706 "Endgame". The two ropes 204428 on the top deck of the static gunship (guids 166908, 166909, phase
-- 4194304, SQL 2026_10_09_14) are the player's way down to the rafters. Lorna: "Everyone else, use the ropes to rappel
-- down". In the ship's collision model the top deck has no walkable link to the rafters, and the rafters reach the
-- furnace room (Korm Bonegrind) by a ramp. The template's spell chain (data10 81425 -> 81422 "Jump 2" + 81992 "Rope 2")
-- has no target conditions and no rope-vehicle movement, so a click left the player on the top deck. That kept Korm out
-- of reach and parked Lorna's boarding timeline until his death. go_gilneas_gunship_rope (zone_gilneas.cpp) moves the
-- player to the rope-bottom mark under the clicked rope. NEEDS the build that carries go_gilneas_gunship_rope.
-- Rollback: UPDATE `gameobject_template` SET `ScriptName` = '' WHERE `entry` = 204428;
UPDATE `gameobject_template` SET `ScriptName` = 'go_gilneas_gunship_rope' WHERE `entry` = 204428;
