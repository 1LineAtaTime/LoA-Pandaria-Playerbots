-- Hillsbrad Foothills, the Sludge Fields: four risen laborers stood or walked about 1.7 yd under the ground near the Shovel
-- of Do the Right Thing, out of sight but still able to fight. The 4.4.2 sniff caught these ghouls on Human Seedlings 47872
-- ("the ghouls periodically jump to the nearest human and devour its head"), so their spawn or two of their path points
-- have the x, y and z of a buried seedling (every seedling stands 1.69-1.75 yd below the terrain on purpose). The z is set
-- to the ground (.map terrain, no WMO here): Farmer 176879 and Peasant 176875 (random movers, so also their home on evade),
-- and points 3 and 4 of the paths of Peasants 176876 and 176880, whose repeating paths (1-2-3-4-1) pass under the ground on
-- every lap. 176880's spawn floated 2.38 yd above the ground and is put on it too. Absolute values, safe to re-run.
UPDATE `creature` SET `position_z`=46.599 WHERE `guid`=176879 AND `id`=47859;
UPDATE `creature` SET `position_z`=46.218 WHERE `guid`=176875 AND `id`=47861;
UPDATE `creature` SET `position_z`=46.388 WHERE `guid`=176880 AND `id`=47861;
UPDATE `waypoint_data` SET `position_z`=46.978 WHERE `id`=176876 AND `point` IN (3,4);
UPDATE `waypoint_data` SET `position_z`=46.818 WHERE `id`=176880 AND `point` IN (3,4);
