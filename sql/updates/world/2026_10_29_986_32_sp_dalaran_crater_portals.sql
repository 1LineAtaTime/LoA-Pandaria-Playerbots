-- ported from PR 15 2026_10_27_46_world.sql (#986)
-- Dalaran Crater: the portals between the Sepulcher and the crater (Relios the Relic Keeper, Ley Energies, Practical
-- Vengeance) teleport to a database destination that was missing, so both put the player back where they stood.
-- No sniff has the destinations; send each portal to the other one's spawn point.
DELETE FROM `spell_target_position` WHERE `id` IN (85317,85318);
INSERT INTO `spell_target_position` (`id`,`effIndex`,`target_map`,`target_position_x`,`target_position_y`,`target_position_z`,`target_orientation`) VALUES
(85317,0,0,294.061,377.439,-63.9923,0),       -- Portal to Dalaran Crater: at the Portal to the Sepulcher (45732, guid 312106)
(85318,0,0,452.427,1465.43,125.141,1.06465);  -- Portal to the Sepulcher: at the Portal to Dalaran Crater (45731, guid 312118), by Dalar Dawnweaver
