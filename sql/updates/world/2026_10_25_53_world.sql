-- Lordaeron (27098): accepting it summoned no one, so Lady Sylvanas Windrunner was never accompanied to the Sepulcher and the
-- quest could not be completed. Sylvanas at Forsaken High Command now casts Summon Sylvanas + Horse on accept (or when talked to
-- while the quest is in progress): the player rides the Forsaken Warhorse beside Sylvanas to the Sepulcher while she tells the
-- story of the Forsaken, and is credited on arrival.
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_sylvanas_windrunner_lordaeron' WHERE `entry` = 45051;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_forsaken_warhorse_lordaeron' WHERE `entry` = 45057;

-- Sylvanas rides her own horse, as at Forsaken High Command
DELETE FROM `creature_template_addon` WHERE `entry` = 45051;
INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `MountCreatureID`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(45051, 0, 29257, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 3, NULL);

DELETE FROM `creature_text` WHERE `CreatureID` = 45051;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(45051, 0, 0, 'I have not always been the Banshee Queen, $n.', 12, 0, 100, 0, 0, 20470, 0, 45201, 5, 'Lady Sylvanas Windrunner - Lordaeron'),
(45051, 1, 0, 'And my people have not always been the Forsaken.', 12, 0, 100, 0, 0, 20471, 0, 45202, 5, 'Lady Sylvanas Windrunner - Lordaeron'),
(45051, 2, 0, 'Long ago this land comprised the northern kingdoms of Lordaeron, ruled by King Terenas Menethil.', 12, 0, 100, 0, 0, 20472, 0, 45203, 5, 'Lady Sylvanas Windrunner - Lordaeron'),
(45051, 3, 0, 'Terenas had a son named Arthas.', 12, 0, 100, 0, 0, 20473, 0, 45204, 5, 'Lady Sylvanas Windrunner - Lordaeron'),
(45051, 4, 0, 'Arthas... even saying his name makes my body quiver in rage.', 12, 0, 100, 0, 0, 20474, 0, 45205, 5, 'Lady Sylvanas Windrunner - Lordaeron'),
(45051, 5, 0, 'This man-child, Arthas, took for himself a cursed blade known as Frostmourne.', 12, 0, 100, 0, 0, 20475, 0, 45206, 5, 'Lady Sylvanas Windrunner - Lordaeron'),
(45051, 6, 0, 'Through Frostmourne, Arthas killed his own father and razed this land along with every living creature in it!', 12, 0, 100, 0, 0, 20476, 0, 45207, 5, 'Lady Sylvanas Windrunner - Lordaeron'),
(45051, 7, 0, 'My own death came at the hands of Arthas when he and his armies sacked my homeland, Quel''Thalas, and murdered my people.', 12, 0, 100, 0, 0, 20477, 0, 45208, 5, 'Lady Sylvanas Windrunner - Lordaeron'),
(45051, 8, 0, 'In his vast cruelty, Arthas severed my spirit from my body and raised me as a banshee to serve in his Scourge army.', 12, 0, 100, 0, 0, 20478, 0, 45209, 5, 'Lady Sylvanas Windrunner - Lordaeron'),
(45051, 9, 0, 'A similar fate befell all that would die to the Scourge war machine.', 12, 0, 100, 0, 0, 20479, 0, 45210, 5, 'Lady Sylvanas Windrunner - Lordaeron'),
(45051, 10, 0, 'In death, they were reborn as mindless undead.', 12, 0, 100, 0, 0, 20480, 0, 45217, 5, 'Lady Sylvanas Windrunner - Lordaeron'),
(45051, 11, 0, 'But Arthas was not invincible! With each passing day his power waned - his grip over the will of the damned loosening.', 12, 0, 100, 0, 0, 20481, 0, 45218, 5, 'Lady Sylvanas Windrunner - Lordaeron'),
(45051, 12, 0, 'It was when Arthas was at his weakest that I struck!', 12, 0, 100, 0, 0, 20482, 0, 45219, 5, 'Lady Sylvanas Windrunner - Lordaeron'),
(45051, 13, 0, 'And though the hour of his atonement had come, the worm managed to escape his fate, returning to the frozen wastes of Northrend.', 12, 0, 100, 0, 0, 20483, 0, 45220, 5, 'Lady Sylvanas Windrunner - Lordaeron'),
(45051, 14, 0, 'With Arthas gone, so too was the control he held over the undead masses of Lordaeron. After recovering my body, I freed the remaining Scourge that were left behind.', 12, 0, 100, 0, 0, 20484, 0, 45221, 5, 'Lady Sylvanas Windrunner - Lordaeron'),
(45051, 15, 0, 'From the cruelty and mercilessness of Arthas, the man who would be the Lich King, the Forsaken were born.', 12, 0, 100, 0, 0, 20485, 0, 45222, 5, 'Lady Sylvanas Windrunner - Lordaeron'),
(45051, 16, 0, 'Our goal... Our sole purpose was to destroy the Lich King. We threw our lot in with the Horde and began our journey towards redemption.', 12, 0, 100, 0, 0, 20486, 0, 45223, 5, 'Lady Sylvanas Windrunner - Lordaeron'),
(45051, 17, 0, 'Now the Lich King is dead and we have returned.', 12, 0, 100, 0, 0, 20487, 0, 45224, 5, 'Lady Sylvanas Windrunner - Lordaeron'),
(45051, 18, 0, 'The people who called this land their home in life, do so in death as well.', 12, 0, 100, 0, 0, 20488, 0, 45225, 5, 'Lady Sylvanas Windrunner - Lordaeron'),
(45051, 19, 0, 'But the Alliance does not recognize our rights. They claim this land is their own while attempting to invalidate the claims of the founders of this kingdom.', 12, 0, 100, 0, 0, 20489, 0, 45226, 5, 'Lady Sylvanas Windrunner - Lordaeron'),
(45051, 20, 0, 'I will never allow it... Never!', 12, 0, 100, 0, 0, 20490, 0, 45227, 5, 'Lady Sylvanas Windrunner - Lordaeron'),
(45051, 21, 0, 'Lordaeron belongs to the Forsaken - always and forever.', 12, 0, 100, 0, 0, 20491, 0, 45228, 5, 'Lady Sylvanas Windrunner - Lordaeron');
