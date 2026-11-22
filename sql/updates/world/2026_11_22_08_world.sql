-- Gameobjects: 15 goobers (type 10) had their auto-close time (Data3) in the pre-3.0.3 format (seconds x 0x10000)
--   while the core reads milliseconds (GameObjectTemplate::GetAutoCloseTime). Example: the Theramore Ammunition
--   Stockpile (195203, quest 14042 Ammo Kerblammo) stayed activated and sparkling for 655 s instead of 10 s.
-- Change: Data3 converted to milliseconds; each UPDATE is guarded by entry, type and the old value.
-- Source: TrinityCore GameObjectData.h ("prior to 3.0.3, conversion was / 0x10000").
-- Rollback:
--   UPDATE `gameobject_template` SET `Data3`=327680 WHERE `entry`=19027 AND `type`=10 AND `Data3`=5000;
--   UPDATE `gameobject_template` SET `Data3`=196608 WHERE `entry` IN (101748,177884,186826,187526,194150,194633,195048,195052,205484,205882) AND `type`=10 AND `Data3`=3000;
--   UPDATE `gameobject_template` SET `Data3`=5898240 WHERE `entry`=185565 AND `type`=10 AND `Data3`=90000;
--   UPDATE `gameobject_template` SET `Data3`=1966080 WHERE `entry`=194482 AND `type`=10 AND `Data3`=30000;
--   UPDATE `gameobject_template` SET `Data3`=1638400 WHERE `entry`=195056 AND `type`=10 AND `Data3`=25000;
--   UPDATE `gameobject_template` SET `Data3`=655360 WHERE `entry`=195203 AND `type`=10 AND `Data3`=10000;
UPDATE `gameobject_template` SET `Data3`=5000 WHERE `entry`=19027 AND `type`=10 AND `Data3`=327680;
UPDATE `gameobject_template` SET `Data3`=3000 WHERE `entry`=101748 AND `type`=10 AND `Data3`=196608;
UPDATE `gameobject_template` SET `Data3`=3000 WHERE `entry`=177884 AND `type`=10 AND `Data3`=196608;
UPDATE `gameobject_template` SET `Data3`=90000 WHERE `entry`=185565 AND `type`=10 AND `Data3`=5898240;
UPDATE `gameobject_template` SET `Data3`=3000 WHERE `entry`=186826 AND `type`=10 AND `Data3`=196608;
UPDATE `gameobject_template` SET `Data3`=3000 WHERE `entry`=187526 AND `type`=10 AND `Data3`=196608;
UPDATE `gameobject_template` SET `Data3`=3000 WHERE `entry`=194150 AND `type`=10 AND `Data3`=196608;
UPDATE `gameobject_template` SET `Data3`=30000 WHERE `entry`=194482 AND `type`=10 AND `Data3`=1966080;
UPDATE `gameobject_template` SET `Data3`=3000 WHERE `entry`=194633 AND `type`=10 AND `Data3`=196608;
UPDATE `gameobject_template` SET `Data3`=3000 WHERE `entry`=195048 AND `type`=10 AND `Data3`=196608;
UPDATE `gameobject_template` SET `Data3`=3000 WHERE `entry`=195052 AND `type`=10 AND `Data3`=196608;
UPDATE `gameobject_template` SET `Data3`=25000 WHERE `entry`=195056 AND `type`=10 AND `Data3`=1638400;
UPDATE `gameobject_template` SET `Data3`=10000 WHERE `entry`=195203 AND `type`=10 AND `Data3`=655360;
UPDATE `gameobject_template` SET `Data3`=3000 WHERE `entry`=205484 AND `type`=10 AND `Data3`=196608;
UPDATE `gameobject_template` SET `Data3`=3000 WHERE `entry`=205882 AND `type`=10 AND `Data3`=196608;
