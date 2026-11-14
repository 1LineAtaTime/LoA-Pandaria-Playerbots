-- Eastern Plaguelands: Light's Hope Chapel's two flight masters each serve their own side.
--
-- Khaelyn Steelwing 12617 (Gryphon Master) and Georgia 12636 (Bat Handler) stand side by side and both offered a flight to
-- players of either side (reported in game). Their gossip menus already hold the retail texts for the other side:
--   menu 12236 (Khaelyn): 7778 "Where would you like to fly to?" and 17188 "I'm afraid I cannot assist you, $r, but I am
--                         certain Georgia can."
--   menu 12237 (Georgia): 7778 and 17187 "I'm afraid I cannot help you, $r, but I am sure Khaelyn Steelwing can."
-- but nothing chose between them and the flight option was shown to everyone.
--
-- Sourced: the two refusal texts (broadcast 47420 / 47419) and the masters' subnames say who serves whom; Light's Hope has
--   one taxi node per side in TaxiNodes.dbc (67 Alliance, 68 Horde).
-- Ours: the condition rows (neither TDB dump has conditions for these menus).

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=14 AND `SourceGroup` IN (12236,12237);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup` IN (12236,12237);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`Comment`) VALUES
(14,12236, 7778,0,0,6,0,469,0,0,0,'Khaelyn Steelwing - flight greeting for the Alliance'),
(14,12236,17188,0,0,6,0, 67,0,0,0,'Khaelyn Steelwing - sends the Horde to Georgia'),
(15,12236,    0,0,0,6,0,469,0,0,0,'Khaelyn Steelwing - flight option for the Alliance'),
(14,12237, 7778,0,0,6,0, 67,0,0,0,'Georgia - flight greeting for the Horde'),
(14,12237,17187,0,0,6,0,469,0,0,0,'Georgia - sends the Alliance to Khaelyn Steelwing'),
(15,12237,    0,0,0,6,0, 67,0,0,0,'Georgia - flight option for the Horde');
