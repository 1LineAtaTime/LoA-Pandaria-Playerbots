-- Eastern Plaguelands, Tyr's Hand: Lynnia Abbendis (10828, the rare of pool 149) is of the Scarlet Crusade.
-- Sourced: faction 67 in both TDBs; mop_world had 7 (a generic hostile faction), so her "call for help" on aggro reached nobody:
-- Creature::CanAssistTo wants a helper friendly to her, and the garrison around her is 67.
UPDATE `creature_template` SET `faction` = 67 WHERE `entry` = 10828;
