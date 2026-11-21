-- Western Plaguelands, Felstone Field: When Death is Not Enough is handed in from the quest log.
-- Sourced: both TDBs set QUEST_FLAGS_AUTO_COMPLETE (0x10000) on 26937 (4.4.2 Flags 67174408, 1210 65544); mop had 67108872. Its reward text
-- is spoken through the val'kyr ("I am able to communicate with you through the Val'kyr"), and RewardNextQuest 26938 (Combat Training) is
-- then offered on the spot. The rest of mop's flags stay.
-- Note: the objective (credit 45089 from the val'kyr's Raise Forsaken) still has no source in this core; this only restores the hand-in.
UPDATE `quest_template` SET `Flags`=67174408 WHERE `ID`=26937;
