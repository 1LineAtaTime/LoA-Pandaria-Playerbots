-- Arathi Highlands: lift the Drywhisker Digger 160074 out of the cave floor in Drywhisker Gorge
-- It stood at z 141.71, 2.9 yd under the floor of the gorge cave (Md_Mushroomcave08.wmo, 144.4-145.0 within 3 yd in the
-- 5.4.8 vmaps). Both TDBs have the same z (their 285983), so the sniffed height is off. Ours: z 144.7.
UPDATE `creature` SET `position_z`=144.7 WHERE `guid`=160074 AND `id`=2574;
