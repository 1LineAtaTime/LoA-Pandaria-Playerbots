-- Kasha 48217 at the Southshore shore: the collapsed Kasha (312260, StandState 7) is hidden once Kasha Will Fly Again
-- 28354 is rewarded, when the healed Kasha (312689, Apply Quest Invis Zone 2, detected through spell_area 89277 from
-- 28354 rewarded) takes off; until now both showed side by side. Our design: neither TDB spawns 48217, Phase.dbc has no
-- Kasha or Southshore phase, and nothing in the sniffs shows how retail swapped them.
-- Phase 200 "Quest Zone-Specific 25" (generic; no spawn, phase_area or condition uses it, and its only granting spell,
-- 77848 Aboard the Krom'gar Wagon, is in Stonetalon), zone-wide in 267 while 28354 is not rewarded (mask 43 = none,
-- complete, in progress, failed). The collapsed Kasha gives and ends no quest (npcflag 0) and no script refers to her:
-- Stormpike Rendezvous 28348 summons its own Kasha 48396 from Orkus 312261 (phase 0), and the later stages use
-- 48467/48471/48576 and spell_area 89277/90115/90116, so nothing else changes.
UPDATE `creature` SET `phaseId`=200, `phaseGroup`=0 WHERE `guid`=312260 AND `id`=48217;

DELETE FROM `phase_area` WHERE `AreaId`=267 AND `PhaseId`=200;
INSERT INTO `phase_area` (`AreaId`,`PhaseId`,`Comment`) VALUES
(267,200,'Hillsbrad Foothills - collapsed Kasha 48217 at the Southshore shore until Kasha Will Fly Again is rewarded');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceGroup`=200 AND `SourceEntry`=267;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(26,200,267,0,0,47,0,28354,43,0,0,0,0,'','Hillsbrad Foothills - phase 200 until Kasha Will Fly Again is rewarded');
