-- Eastern Plaguelands: the four breadcrumbs to Fiona (Warchief's Command 28577, Hero's Call 28578, Into the Woods 27683,
-- Visitors 27684) are offered only before her chain has begun, Into the Woods and Visitors exclude each other, and neither
-- is offered while a board quest is in the log.
--
--  * Before: no prerequisite or condition on any of the four: Tyrosus offered Visitors to a character who had finished the
--    zone, and a player could take and hand in all four.
--  * Sourced: "unavailable once you progress through her quest chain" / "as soon as you start any chain in the zone"
--    (Eillesthis [+25], QuickBASIC, Tarinae [+14]); Into the Woods and Visitors are alternatives (sapristoire [+18]); not
--    offered while the board quest is in the log, the other way round allowed (daddysayruth [+8], dRAEMtYGER [+6], wiki).
--  * Ours: "the chain has begun" = Gidwin Goldbraids 27367 or Tarenar Sunstrike 27370, Fiona's first two quests, taken or
--    done (CONDITION_QUEST_NONE 14 on both).

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceGroup`=0 AND `SourceEntry` IN (27683,27684,28577,28578);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(19,0,27683,0,0,14,0,27367,0,0,0,0,0,'','Into the Woods: Fiona''s chain not begun'),
(19,0,27683,0,0,14,0,27370,0,0,0,0,0,'','Into the Woods: Fiona''s chain not begun'),
(19,0,27683,0,0,14,0,27684,0,0,0,0,0,'','Into the Woods: not with Visitors'),
(19,0,27683,0,0,9,0,28577,0,0,1,0,0,'','Into the Woods: not while Warchief''s Command is in the log'),
(19,0,27683,0,0,9,0,28578,0,0,1,0,0,'','Into the Woods: not while Hero''s Call is in the log'),
(19,0,27684,0,0,14,0,27367,0,0,0,0,0,'','Visitors: Fiona''s chain not begun'),
(19,0,27684,0,0,14,0,27370,0,0,0,0,0,'','Visitors: Fiona''s chain not begun'),
(19,0,27684,0,0,14,0,27683,0,0,0,0,0,'','Visitors: not with Into the Woods'),
(19,0,27684,0,0,9,0,28577,0,0,1,0,0,'','Visitors: not while Warchief''s Command is in the log'),
(19,0,27684,0,0,9,0,28578,0,0,1,0,0,'','Visitors: not while Hero''s Call is in the log'),
(19,0,28577,0,0,14,0,27367,0,0,0,0,0,'','Warchief''s Command: Eastern Plaguelands: Fiona''s chain not begun'),
(19,0,28577,0,0,14,0,27370,0,0,0,0,0,'','Warchief''s Command: Eastern Plaguelands: Fiona''s chain not begun'),
(19,0,28578,0,0,14,0,27367,0,0,0,0,0,'','Hero''s Call: Eastern Plaguelands: Fiona''s chain not begun'),
(19,0,28578,0,0,14,0,27370,0,0,0,0,0,'','Hero''s Call: Eastern Plaguelands: Fiona''s chain not begun');
