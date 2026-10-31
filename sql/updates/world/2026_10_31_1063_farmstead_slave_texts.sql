-- Kun-Lai Summit, Farmhand Freedom: the freed Farmstead Slaves (59577) say one of these lines (group 0, 50%).
-- They had no creature_text rows, so every freed slave logged 'Could not find Text'. Lines and broadcast texts
-- from the reference 5.4.8 data and the client's broadcast_text.
DELETE FROM `creature_text` WHERE `CreatureID` = 59577;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(59577,0,0,'Death to the yaungol!',12,0,50,0,0,0,58875,0,'Farmstead Slave - Farmhand Freedom'),
(59577,0,1,'Good riddance.',12,0,50,0,0,0,58879,0,'Farmstead Slave - Farmhand Freedom'),
(59577,0,2,'Let''s go to Binan Village.',12,0,50,0,0,0,58878,0,'Farmstead Slave - Farmhand Freedom'),
(59577,0,3,'Our hero!',12,0,50,0,0,0,58876,0,'Farmstead Slave - Farmhand Freedom'),
(59577,0,4,'Quick, let''s get out of here.',12,0,50,0,0,0,58874,0,'Farmstead Slave - Farmhand Freedom'),
(59577,0,5,'We''re free!',12,0,50,0,0,0,58802,0,'Farmstead Slave - Farmhand Freedom'),
(59577,0,6,'We''re saved!',12,0,50,0,0,0,58877,0,'Farmstead Slave - Farmhand Freedom');

-- Undo: DELETE FROM `creature_text` WHERE `CreatureID` = 59577;
