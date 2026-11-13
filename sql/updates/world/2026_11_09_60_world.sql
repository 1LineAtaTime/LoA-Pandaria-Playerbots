-- Rescue OOX-09/HL! (836): Homing Robot OOX-09/HL speaks its own lines
-- Sourced: creature_text 7806 groups 0-3 (start, aggro, ambush, end) with their broadcast text ids, as in TDB 442 and 1210.
-- mop had OOX-17/TN's Tanaris line "No one challenges the Wastewander nomads" as group 3 and the end line as group 4; the
-- script (npc_00x09hl) now says group 2 at each ambush and group 3 at the end.
DELETE FROM `creature_text` WHERE `CreatureID`=7806;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(7806,0,0,'Emergency power activated!  Initializing ambulatory motor!  CLUCK!',12,0,100,0,0,0,0,3819,0,'Homing Robot OOX-09/HL'),
(7806,1,0,'Physical threat detected!  Evasive action!  CLUCK!',12,0,100,0,0,0,0,3795,0,'Homing Robot OOX-09/HL'),
(7806,1,1,'Threat analyzed!  Activating combat plan beta!  CLUCK!',12,0,100,0,0,0,0,3796,0,'Homing Robot OOX-09/HL'),
(7806,2,0,'CLUCK!  Sensors detect spatial anomaly -- danger imminent!  CLUCK!',12,0,100,0,0,0,0,3821,0,'Homing Robot OOX-09/HL'),
(7806,3,0,'Flight systems online!  CLUCK!  Engaging rockets for transport to Booty Bay!',12,0,100,0,0,0,0,3820,0,'Homing Robot OOX-09/HL');
