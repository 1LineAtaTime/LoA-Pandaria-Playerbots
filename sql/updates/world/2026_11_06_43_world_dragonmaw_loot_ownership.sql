-- Preserve populated local Dragonmaw loot owners after the SF27 shared-owner update.
-- Original owners verified from the pre-SQL42 backup. No loot rows are deleted.
UPDATE `creature_template` SET `lootid`=41072 WHERE `entry`=41072 AND `lootid`=2102;
UPDATE `creature_template` SET `lootid`=41080 WHERE `entry`=41080 AND `lootid`=2103;
UPDATE `creature_template` SET `lootid`=42107 WHERE `entry`=42107 AND `lootid`=2102;
