-- Eastern Plaguelands, Defenders of Darrowshire (27529): the freed Darrowshire Spirit's thank-you can be read.
--
-- Reported in game: the gossip window of a Darrowshire Spirit 11064 closes too fast to read. 2026_11_14_74 made the spirit
-- unselectable on the click (which closes the window at once) and despawned it 4 s later.
--
-- Sourced: both TDBs (4.4.2 and 12.1): the clicked spirit stays selectable and despawns after 10 s.
-- Kept: the credit to the clicker, once per spirit (event flag 1, not repeatable, on the gossip row; the despawn is linked to
--   it); the spawn-in visual; the 60 s life of a spirit nobody clicks.

DELETE FROM `smart_scripts` WHERE `entryorguid` = 11064 AND `source_type` = 0 AND `id` IN (1,2,3);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`target_type`,`comment`) VALUES
(11064,0,1,2,64,0,100,1,0,0,0,0,33,11064,0,7,'Darrowshire Spirit - On Gossip Hello - Kill Credit to the invoker'),
(11064,0,2,0,61,0,100,0,0,0,0,0,41,10000,0,1,'Darrowshire Spirit - On Gossip Hello - Despawn after 10 s');
