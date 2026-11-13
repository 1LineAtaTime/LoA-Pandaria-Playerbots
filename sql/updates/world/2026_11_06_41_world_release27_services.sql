-- SF27 services and quest gates, adapted to the active LoA schema.

-- SkyFire contributors, GPL-3.0; source revision and hashes in the paired ledger.

-- 2026_08_26_world_01.sql

UPDATE `gossip_menu_option` SET
    `OptionBroadcastTextID` = 20225,
    `OptionType` = 3,
    `OptionNpcflag` = 128
WHERE `MenuID` = 11778 AND `OptionID` = 0;

-- 2026_08_26_world_07.sql

UPDATE `gossip_menu_option`
SET `OptionType` = 3, `OptionNpcflag` = 128
WHERE `MenuID` = 12726 AND `OptionID` = 0;

-- 2026_09_01_world_00.sql

UPDATE `creature_template`
SET `npcflag` = `npcflag` | 4194304
WHERE `gossip_menu_id` IN (7451, 9212, 9437, 9576, 9821, 9864, 11971, 21194)
  AND (`npcflag` & 4194304) = 0;

-- 2026_09_21_world_01.sql

UPDATE `gossip_menu_option` SET `OptionType` = 8, `OptionNpcflag` = 65536
WHERE `OptionType` = 0 AND `OptionNpcflag` = 0 AND (`MenuID`, `OptionID`) IN (
    (1293,1),

    (6059,0),

    (7290,0),

    (7497,0),

    (7869,0),

    (8912,0),

    (11520,0),

    (11538,0),

    (11568,0),

    (12003,0),

    (12040,0),

    (12101,2),

    (12596,0),

    (12677,1)

);

UPDATE `gossip_menu_option` SET `OptionType` = 3, `OptionNpcflag` = 128
WHERE `OptionType` = 0 AND `OptionNpcflag` = 0 AND (`MenuID`, `OptionID`) IN (
    (1293,2), (6059,1), (7290,1), (7497,1), (7869,1),
    (8912,1), (11520,1), (12040,1), (12677,2)
);

UPDATE `gossip_menu_option` SET `OptionNpcflag` = 65536
 WHERE `MenuID` = 9868 AND `OptionID` = 0 AND `OptionType` = 8 AND `OptionNpcflag` = 128;

UPDATE `gossip_menu_option` SET `OptionNpcflag` = 128
 WHERE `MenuID` = 9868 AND `OptionID` = 1 AND `OptionType` = 3 AND `OptionNpcflag` = 65536;

UPDATE `creature_template` SET `npcflag` = `npcflag` | 1
 WHERE `entry` IN (
    1247,

    5111,

    6741,

    41618,

    55180,

    55809,

    59405,

    62322,

    62871,

    62872,

    62996,

    64149,

    65528

   )
   AND (`npcflag` & 1) = 0;

-- 2026_09_21_world_02.sql

UPDATE `conditions` SET `ConditionValue1` = 29605
WHERE `SourceTypeOrReferenceId` = 15
  AND `SourceGroup` = 7868
  AND `SourceEntry` = 0
  AND `ConditionTypeOrReference` = 9
  AND `ConditionValue1` = 10097;

-- 2026_09_21_world_06.sql

UPDATE `conditions`
   SET `ConditionValue1` = 2015
 WHERE `ConditionTypeOrReference` = 15
   AND `ConditionValue1` = 1503
   AND `SourceTypeOrReferenceId` IN (19, 20)
   AND `SourceEntry` IN (13104, 13408, 13409);

UPDATE `quest_template_addon`
   SET `AllowableClasses` = 2015
 WHERE `AllowableClasses` = 1503
   AND `ID` IN (13104, 13682, 13789, 13790, 13794, 13809, 13810, 13811, 13861, 13862);

-- 2026_09_21_world_07.sql

UPDATE `conditions` SET `NegativeCondition` = 1, `ConditionValue3` = 1
WHERE `SourceTypeOrReferenceId` = 15
  AND `SourceGroup` IN (9155, 9156)
  AND `SourceEntry` = 0
  AND `ConditionTypeOrReference` = 2
  AND `ConditionValue1` = 34842;
