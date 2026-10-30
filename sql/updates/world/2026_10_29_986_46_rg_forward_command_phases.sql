-- ported from PR 15 2026_10_27_62_world.sql (#986)
-- Ruins of Gilneas (zone 4706), Horde: phase the story around Forsaken Forward Command
-- Retail has every spawn of the zone in phase 274 "Gilneas Act I" (both world databases) and its area triggers (Dreadwatch
-- Outpost, the edge of Emberstone Village) also carry phase 274, but here only the ride aura of the flight in gave it. Give it
-- zone-wide to everyone on the Horde storyline (To Forsaken Forward Command taken or done), so those area triggers work.
-- Forward Command was spawned twice on the same spots: holding (Onslaught, Belmont, defenders, catapults, Blightspreaders,
-- Sharpshooters and the attacking worgen) and overrun after Fall Back! (feign-death Vanguards and catapults with their
-- Blightspreaders, Onslaught dead, worgen, the 7th Legion marching through in columns). Retail sniffed only the first.
--   199  Forward Command holding     Fall Back! not taken (an unused generic phase, "Quest Zone-Specific 24")
--   275  Forward Command overrun     Fall Back! taken, completed or rewarded ("Gilneas Act II", also given by the escape flight)
--   276  Godfrey's corpse and Arthura at Tempest's Reach   The Great Escape not taken ("Gilneas Act III"; Arthura then carries
--        the corpse away)
-- The ids and the quest that switches them are ours; no sniff has them.
UPDATE `creature` SET `phaseId` = 199, `phaseGroup` = 0 WHERE `map` = 0 AND `id` IN (45280, 45282, 45292, 45312, 45313, 45314, 45315, 45374, 45375, 45376)
    AND `position_x` BETWEEN -1110 AND -800 AND `position_y` BETWEEN 1440 AND 1700;
UPDATE `creature` SET `phaseId` = 275, `phaseGroup` = 0 WHERE `map` = 0 AND `id` IN (45532, 45533, 45534, 45535, 45539, 45541, 51192);
UPDATE `creature` SET `phaseId` = 276, `phaseGroup` = 0 WHERE (`guid`, `id`) IN ((306589, 44369), (306590, 45314));

DELETE FROM `phase_area` WHERE `AreaId` = 4706 AND `PhaseId` IN (199, 274, 275, 276);
INSERT INTO `phase_area` (`AreaId`, `PhaseId`, `Comment`) VALUES
(4706, 274, 'Ruins of Gilneas - Gilneas Act I for the Horde storyline (area triggers)'),
(4706, 199, 'Ruins of Gilneas - Forsaken Forward Command holding'),
(4706, 275, 'Ruins of Gilneas - Forsaken Forward Command overrun after Fall Back!'),
(4706, 276, 'Ruins of Gilneas - Godfrey''s corpse at Tempest''s Reach before The Great Escape');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 26 AND `SourceEntry` = 4706 AND `SourceGroup` IN (199, 274, 275, 276);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(26, 274, 4706, 0, 0, 47, 0, 27290, 74, 0, 0, 0, 0, '', 'Ruins of Gilneas - phase 274 once To Forsaken Forward Command is taken'),
(26, 199, 4706, 0, 0, 47, 0, 27405, 33, 0, 0, 0, 0, '', 'Ruins of Gilneas - phase 199 while Fall Back! is not taken'),
(26, 275, 4706, 0, 0, 47, 0, 27405, 74, 0, 0, 0, 0, '', 'Ruins of Gilneas - phase 275 once Fall Back! is taken'),
(26, 276, 4706, 0, 0, 47, 0, 27438, 33, 0, 0, 0, 0, '', 'Ruins of Gilneas - phase 276 while The Great Escape is not taken');
