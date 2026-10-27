-- Ruins of Gilneas: the Forsaken Vanguards (45280) were immune to NPCs (template unit_flags 0x8210: IMMUNE_TO_NPC and RENAME
-- besides CAN_SWIM), so they and the Worgen Rebels and Rushers attacking them across the Northern Headlands and Forward Command
-- never fought: neither side can start combat with, or attack, a unit immune to NPCs. Retail has no unit flags on them.
UPDATE `creature_template` SET `unit_flags` = `unit_flags` & ~(0x200 | 0x10) WHERE `entry` = 45280;
