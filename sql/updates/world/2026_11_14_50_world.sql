-- Eastern Plaguelands, Light's Hope Chapel: three quests were offered too early.
--  * Argent Call: Northdale 27466 (Tarenar Sunstrike): no prerequisite, so it was offered at level 39 before the Trial of the
--    Crypt. Sourced: the wiki's previous quest is Argent Call: The Noxious Glade 27465; comments 2011-01-15, 2012-01-19 and
--    2013-12-09 ("must complete Argent Call: The Noxious Glade to get this quest"; "once I turned in these two, Tarenar
--    immediately offered Argent Call: Northdale"). Buried Blades is not required: nothing in mop or either TDB links them.
--  * An Opportune Alliance 27457 (Rimblat Earthshatter): offered as soon as Gathering Some Grub(s) 27456 was handed in.
--    Sourced: comments 2011-03-28 with its reply and 2012-01-19 (not offered with 27465 in progress; "completing 27465 made it
--    available again"; "I had to turn in [27465]: the next quests were An Opportune Alliance and Argent Call: Northdale").
--    It keeps PrevQuestID 27456; the condition adds 27465. Its ender is the Fiona of the caravan's stop, at Eastwall or later
--    by then (27465 needs 27455).
--  * Buried Blades 27467 (Master Craftsman Omarion): no prerequisite in mop or either TDB, while comments place it after the
--    Trial ("becomes available after you complete The Trial of the Crypt", 2010-11-24; "you have to do a lot of the Fiona's
--    Caravan quests to unlock this one", 2010-12-03, 2012-07-16) and see it offered alongside 27465 (2011-03-28).
--    Ours: PrevQuestID 27464 (Trial of the Crypt rewarded), the latest point the comments allow; no data source holds the gate.
DELETE FROM `quest_template_addon` WHERE `ID`=27466;
INSERT INTO `quest_template_addon` (`ID`,`PrevQuestID`) VALUES (27466,27465);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceGroup`=0 AND `SourceEntry`=27457;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(19,0,27457,0,0,8,0,27465,0,0,0,0,0,'','An Opportune Alliance is offered once Argent Call: The Noxious Glade is rewarded');

UPDATE `quest_template_addon` SET `PrevQuestID`=27464 WHERE `ID`=27467;
