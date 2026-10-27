-- Ambermill (area 233): phase the other dimension of Transdimensional Warfare and its aftermath
-- Everything was spawned in the default phase, so everybody saw the Dalaran mage town of the other dimension, the
-- Forsaken army that storms it, its aftermath and three Archmage Ataerics at once, and spell_area put the Transdimensional
-- Shift screen effect on everyone in Ambermill who had not completed Chapter III.
-- Phase.dbc names 279, 280 and 281 "Amber Mill" and PhaseXPhaseGroup shows 280 and 281 together. No 5.4.8 spell grants
-- them and no sniff shows the layers, so the assignment is ours:
--   280  the mage town of the other dimension (Chapter II, Chapter III)   while the player has Transdimensional Shift
--   281  Archmage Ataeric's stand against the Forsaken (Chapter III)      Transdimensional Shift and Chapter III in progress
--   279  the aftermath: the Forsaken army, Daschla and the raised Ataeric  Chapter III completed or rewarded
-- Transdimensional Shift itself is given in Ambermill from taking Chapter II until Chapter III is completed (the
-- Ambermill Dimensional Portal takes the player there; walking in works too). The portal only opens for players on
-- Chapter II or Chapter III.

UPDATE `creature` SET `phaseId`=280, `phaseGroup`=0 WHERE `map`=0 AND `areaId`=233 AND `id` IN (1888,1889,1913,1914,1920,3577,3578);
UPDATE `creature` SET `phaseId`=281, `phaseGroup`=0 WHERE `map`=0 AND `areaId`=233 AND `id` IN (2120,45761,45762,45763,45764);
UPDATE `creature` SET `phaseId`=279, `phaseGroup`=0 WHERE `map`=0 AND `areaId`=233 AND `id` IN (45775,45777,45778,45781,45802,45803);

-- Doomskull is an Ambermill Witchalok's summon (spell 87225) sniffed as a spawn; Ataeric's corpse 45779 lay under the
-- raised Ataeric 45803. The Archmage Target 45764 in front of Ataeric is the target of his Dimensional Beam and stays.
DELETE FROM `creature_addon` WHERE `guid` IN (312540,312484);
DELETE FROM `creature` WHERE (`guid`,`id`) IN ((312540,46846),(312484,45779));

DELETE FROM `spell_area` WHERE `spell`=85361 AND `area`=233;
INSERT INTO `spell_area` (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`autocast`,`quest_start_status`,`quest_end_status`) VALUES
(85361,233,27513,27518,0,0,2,1,74,41);

DELETE FROM `phase_area` WHERE `AreaId`=233 AND `PhaseId` IN (279,280,281);
INSERT INTO `phase_area` (`AreaId`,`PhaseId`,`Comment`) VALUES
(233,280,'Ambermill - the mage town of the other dimension'),
(233,281,'Ambermill - Archmage Ataeric''s stand'),
(233,279,'Ambermill - after Transdimensional Warfare: Chapter III');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceEntry`=233 AND `SourceGroup` IN (279,280,281);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=12054 AND `SourceEntry`=0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(26,280,233,0,0,1,0,85361,0,0,0,0,0,'','Ambermill - phase 280 with Transdimensional Shift'),
(26,281,233,0,0,1,0,85361,0,0,0,0,0,'','Ambermill - phase 281 with Transdimensional Shift'),
(26,281,233,0,0,47,0,27518,8,0,0,0,0,'','Ambermill - phase 281 while Transdimensional Warfare: Chapter III is in progress'),
(26,279,233,0,0,47,0,27518,66,0,0,0,0,'','Ambermill - phase 279 once Transdimensional Warfare: Chapter III is completed'),
(15,12054,0,0,0,47,0,27513,8,0,0,0,0,'','Ambermill Dimensional Portal - gossip option with Transdimensional Warfare: Chapter II in progress'),
(15,12054,0,0,1,47,0,27518,8,0,0,0,0,'','Ambermill Dimensional Portal - or with Transdimensional Warfare: Chapter III in progress');
