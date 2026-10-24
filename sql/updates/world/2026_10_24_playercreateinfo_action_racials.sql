-- Starting action bars: 11 racial buttons pointed at the variant of Blood Fury / Arcane Torrent / Gift of the Naaru that belongs to
-- another class (the SkillLineAbility ClassMask of each racial skill). A new character of these race/class pairs does not learn
-- that spell, so the button stayed empty and the server logged "Spell action N not added into button M ... player don't known this spell".
-- Orc shaman/monk: Blood Fury 33697. Blood elf warrior: Arcane Torrent 69179, monk: 129597. Draenei warrior: Gift of the
-- Naaru 28880, monk: 121093.
-- Worgen warrior: buttons 9 and 81 held Darkflight (68992). Worgen learn their racials from the Gilneas starting quests (every
-- class but the death knight, which knows them from the start), so a new worgen warrior does not know it either; no other
-- Gilneas-start worgen class has a racial on its starting bar. The rows are removed.
-- Undo: the same UPDATEs with the old values (orc 7/9 20572; orc 10/9 20572, 10/81 33702; blood elf 1 and 10 both 28730;
-- draenei 1/9 59543, 1/81 59542; draenei 10/9 59542, 10/81 59548), and
-- INSERT INTO `playercreateinfo_action` (`race`, `class`, `button`, `action`, `type`) VALUES (22, 1, 9, 68992, 0), (22, 1, 81, 68992, 0);
UPDATE `playercreateinfo_action` SET `action` = 33697 WHERE `race` = 2 AND `class` = 7 AND `button` = 9 AND `type` = 0 AND `action` = 20572;
UPDATE `playercreateinfo_action` SET `action` = 33697 WHERE `race` = 2 AND `class` = 10 AND `button` = 9 AND `type` = 0 AND `action` = 20572;
UPDATE `playercreateinfo_action` SET `action` = 33697 WHERE `race` = 2 AND `class` = 10 AND `button` = 81 AND `type` = 0 AND `action` = 33702;
UPDATE `playercreateinfo_action` SET `action` = 69179 WHERE `race` = 10 AND `class` = 1 AND `button` IN (9, 81) AND `type` = 0 AND `action` = 28730;
UPDATE `playercreateinfo_action` SET `action` = 129597 WHERE `race` = 10 AND `class` = 10 AND `button` IN (9, 81) AND `type` = 0 AND `action` = 28730;
UPDATE `playercreateinfo_action` SET `action` = 28880 WHERE `race` = 11 AND `class` = 1 AND `button` = 9 AND `type` = 0 AND `action` = 59543;
UPDATE `playercreateinfo_action` SET `action` = 28880 WHERE `race` = 11 AND `class` = 1 AND `button` = 81 AND `type` = 0 AND `action` = 59542;
UPDATE `playercreateinfo_action` SET `action` = 121093 WHERE `race` = 11 AND `class` = 10 AND `button` = 9 AND `type` = 0 AND `action` = 59542;
UPDATE `playercreateinfo_action` SET `action` = 121093 WHERE `race` = 11 AND `class` = 10 AND `button` = 81 AND `type` = 0 AND `action` = 59548;
DELETE FROM `playercreateinfo_action` WHERE `race` = 22 AND `class` = 1 AND `button` IN (9, 81) AND `type` = 0 AND `action` = 68992;
