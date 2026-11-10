-- Cocooned Victim 42907 (Starvation Diet 26387/26643, the Creeping Ruin): they aggroed on players who came near and ran at them.
-- Sourced: both TDBs give the template Civilian (here flags_extra 0x2: no proximity aggro) and no AI script.
UPDATE `creature_template` SET `flags_extra`=`flags_extra`|2, `AIName`='' WHERE `entry`=42907;
DELETE FROM `smart_scripts` WHERE `entryorguid`=42907 AND `source_type`=0;
