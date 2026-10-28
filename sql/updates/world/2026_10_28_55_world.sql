-- Ambermill (area 233): show Daschla raising Archmage Ataeric inside the other dimension, and the aftermath outside it only.
-- The aftermath phase 279 (2026_10_27_43) showed two Daschlas once Transdimensional Warfare: Chapter III was completed: the
-- one hovering over the raised Ataeric 45803 (45802) and the one with the Forsaken army (45775). The first is the end of the
-- fight inside the other dimension; the second is what the Forsaken Front sees outside it. Now:
--   190 "Quest Zone-Specific 15" (unused generic phase, ours): Daschla 45802 and the raised Ataeric 45803, while the player
--       still has Transdimensional Shift 85361 and Chapter III is completed;
--   279 the aftermath (army, catapults, Daschla 45775) only without Transdimensional Shift.
-- Transdimensional Shift now lasts until Chapter III is handed in (it ended on completion, so the raising was never seen).
-- The assignment is ours; no sniff shows these layers.
UPDATE `creature` SET `phaseId`=190, `phaseGroup`=0 WHERE `guid` IN (312242,312243) AND `id` IN (45802,45803);

DELETE FROM `spell_area` WHERE `spell`=85361 AND `area`=233;
INSERT INTO `spell_area` (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`autocast`,`quest_start_status`,`quest_end_status`) VALUES
(85361,233,27513,27518,0,0,2,1,74,43);

DELETE FROM `phase_area` WHERE `AreaId`=233 AND `PhaseId`=190;
INSERT INTO `phase_area` (`AreaId`,`PhaseId`,`Comment`) VALUES
(233,190,'Ambermill - Daschla raises Archmage Ataeric (inside the other dimension, Chapter III completed)');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceEntry`=233 AND `SourceGroup`=190;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceEntry`=233 AND `SourceGroup`=279 AND `ConditionTypeOrReference`=1;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(26,190,233,0,0,1,0,85361,0,0,0,0,0,'','Ambermill - phase 190 with Transdimensional Shift'),
(26,190,233,0,0,47,0,27518,2,0,0,0,0,'','Ambermill - phase 190 once Transdimensional Warfare: Chapter III is completed'),
(26,279,233,0,0,1,0,85361,0,0,1,0,0,'','Ambermill - phase 279 only outside the other dimension (no Transdimensional Shift)');

-- The Forsaken Front val'kyr (Agatha, Daschla, Arthura) hover in the sniff, a few yards above the ground, but had no
-- movement row, so the core moved them to the ground and they stood clipped into it. Disable gravity, as for the
-- Sepulcher Daschla 44609.
DELETE FROM `creature_template_movement` WHERE `CreatureId` IN (45583,45584,45585,45625,45626,45627,45761,45775,45802);
INSERT INTO `creature_template_movement` (`CreatureId`,`Ground`,`Swim`,`Flight`,`Rooted`,`Chase`,`Random`,`InteractionPauseTimer`) VALUES
(45583,0,0,1,0,NULL,NULL,NULL), -- Agatha
(45584,0,0,1,0,NULL,NULL,NULL), -- Daschla
(45585,0,0,1,0,NULL,NULL,NULL), -- Arthura
(45625,0,0,1,0,NULL,NULL,NULL), -- Agatha
(45626,0,0,1,0,NULL,NULL,NULL), -- Daschla
(45627,0,0,1,0,NULL,NULL,NULL), -- Arthura
(45761,0,0,1,0,NULL,NULL,NULL), -- Daschla (Ataeric's stand)
(45775,0,0,1,0,NULL,NULL,NULL), -- Daschla (Ambermill aftermath)
(45802,0,0,1,0,NULL,NULL,NULL); -- Daschla (raising Ataeric)
