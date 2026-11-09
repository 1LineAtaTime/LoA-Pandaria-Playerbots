-- Felstone Field: the Alliance farm until Derrington plagues it in Lower the Boom, the plagued farm and the val'kyr practice
-- ground after
-- Sourced: Phase.dbc 262 "Felstone Field - Alliance-Held" and 263 "Felstone Field - Val'kyr Practice". The quest texts: Foes
-- Before Hoes 26933 (kill 9 Alliance Laborers 44433, "the men who till Felstone Field") and Who Needs Cauldrons? 26978
-- (the four drum credits 44622-44625) are set on the working farm; Lower the Boom 26936's reward text is Derrington's
-- ignition ("Felstone is cleared and the bombs are set ... IN 3... 2.... 1..."); Strange New Faces 26979 "Now that
-- Felstone is ours"; When Death is Not Enough 26937 "it was I who ordered the plaguing of Felstone Field. We need this area
-- for practice" (kill 8 of the plagued Alliance Laborers 45047, template auras Green Radiation 68571 and Cower 78087, for
-- the val'kyr to raise); Combat Training 26938 (the Forsaken Troopers 45085 and Opengut Behemoth 45098 in the barn). The
-- Forsaken Troopers 45048-45050 stand on plagued laborers' spots ("Summon FT Felstone 01-03" 84129-84131, the raised
-- laborers). Neither TDB has any of the post-bomb entries or phases the farm.
-- Ours: the keys, granted zone-wide in Western Plaguelands (28) as Andorhal's:
--   262 (laborers 44433 and the drum credits): until Lower the Boom is rewarded and none of 26979/26937/26938 is taken, or
--       while Foes Before Hoes or Who Needs Cauldrons? is in the log; the Alliance always sees the working farm
--   263 (plagued laborers, troopers, the Behemoth): from Lower the Boom rewarded, or 26979/26937/26938 taken
-- The 25 Troopers 45085 also need their quest invisibility detection (84190 See Quest Invis 4) for Combat Training; that is not
-- in this file.
UPDATE `creature` SET `phaseId`=262, `phaseGroup`=0 WHERE `map`=0 AND `zoneId`=28 AND `areaId`=199 AND `id` IN (44433,44622,44623,44624,44625);
UPDATE `creature` SET `phaseId`=263, `phaseGroup`=0 WHERE `map`=0 AND `zoneId`=28 AND `areaId`=199 AND `id` IN (45047,45048,45049,45050,45085,45098);

DELETE FROM `phase_area` WHERE `AreaId`=28 AND `PhaseId` IN (262,263);
INSERT INTO `phase_area` (`AreaId`,`PhaseId`,`Comment`) VALUES
(28,262,'Western Plaguelands - Felstone Field: Alliance-Held, until Lower the Boom'),
(28,263,'Western Plaguelands - Felstone Field: Val''kyr Practice, from Lower the Boom');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceEntry`=28 AND `SourceGroup` IN (262,263);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(26,262,28,0,0,47,0,26936,64,0,1,0,0,'','Felstone Field - Alliance-Held: Lower the Boom not rewarded'),
(26,262,28,0,0,47,0,26979,74,0,1,0,0,'','Felstone Field - Alliance-Held: Strange New Faces not taken'),
(26,262,28,0,0,47,0,26937,74,0,1,0,0,'','Felstone Field - Alliance-Held: When Death is Not Enough not taken'),
(26,262,28,0,0,47,0,26938,74,0,1,0,0,'','Felstone Field - Alliance-Held: Combat Training (H) not taken'),
(26,262,28,0,1,47,0,26933,10,0,0,0,0,'','Felstone Field - Alliance-Held: Foes Before Hoes in the log'),
(26,262,28,0,2,47,0,26978,10,0,0,0,0,'','Felstone Field - Alliance-Held: Who Needs Cauldrons? in the log'),
(26,263,28,0,0,47,0,26936,64,0,0,0,0,'','Felstone Field - Val''kyr Practice: Lower the Boom rewarded'),
(26,263,28,0,1,47,0,26979,74,0,0,0,0,'','Felstone Field - Val''kyr Practice: Strange New Faces taken'),
(26,263,28,0,2,47,0,26937,74,0,0,0,0,'','Felstone Field - Val''kyr Practice: When Death is Not Enough taken'),
(26,263,28,0,3,47,0,26938,74,0,0,0,0,'','Felstone Field - Val''kyr Practice: Combat Training (H) taken');
