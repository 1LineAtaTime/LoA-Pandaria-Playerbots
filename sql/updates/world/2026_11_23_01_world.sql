-- Baron Longshore (3467), Northern Barrens: both gossip options (menu 10567) are offered to everyone, also without the quest
-- The Baron's Demands (14046), whose quest text says "talk to Baron Longshore to free him from his manacles".
-- Adds CONDITION_QUESTTAKEN (9) on option 0 ("Never, you two-timing dog!") and option 1 ("<Break Longshore's manacles and free him.>"),
-- so both options only show while the quest is in the log and not yet complete.
-- Rollback:
--   DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=10567 AND `SourceEntry` IN (0,1) AND `ConditionTypeOrReference`=9 AND `ConditionValue1`=14046;

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=10567 AND `SourceEntry` IN (0,1) AND `ConditionTypeOrReference`=9;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(15,10567,0,0,0,9,0,14046,0,0,0,0,0,'','Baron Longshore - refuse option only with The Baron''s Demands taken'),
(15,10567,1,0,0,9,0,14046,0,0,0,0,0,'','Baron Longshore - free him only with The Baron''s Demands taken');
