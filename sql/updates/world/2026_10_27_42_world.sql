-- The Forsaken Front: Baron Ashbury 45607 is one of the personal actors of Rise, Godfrey (summoned by spell 85156 for the
-- player on the quest), sniffed as a static spawn that stood in the camp for everyone
DELETE FROM `creature_addon` WHERE `guid`=373752;
DELETE FROM `creature` WHERE `guid`=373752 AND `id`=45607;
