-- Seven equipment sets were imported under ID 0, reserved for unarmed.
-- Item.db2 build 18273 confirms every authored item exists and is a hand item.
-- Renumber five otherwise absent sets; discard two byte-identical duplicates
-- only while their valid ID-1 definitions remain. Preserve custom overrides.
UPDATE creature_equip_template old LEFT JOIN creature_equip_template valid ON valid.entry=old.entry AND valid.id=1 SET old.id=1 WHERE old.`entry`=60524 AND old.`id`=0 AND old.`itemEntry1`=94849 AND old.`itemEntry2`=0 AND old.`itemEntry3`=0 AND old.`CreatureID`=60524 AND old.`ItemID1`=94849 AND old.`ItemID2`=0 AND old.`ItemID3`=0 AND valid.entry IS NULL;
UPDATE creature_equip_template old LEFT JOIN creature_equip_template valid ON valid.entry=old.entry AND valid.id=1 SET old.id=1 WHERE old.`entry`=60572 AND old.`id`=0 AND old.`itemEntry1`=94119 AND old.`itemEntry2`=0 AND old.`itemEntry3`=0 AND old.`CreatureID`=60572 AND old.`ItemID1`=94119 AND old.`ItemID2`=0 AND old.`ItemID3`=0 AND valid.entry IS NULL;
UPDATE creature_equip_template old LEFT JOIN creature_equip_template valid ON valid.entry=old.entry AND valid.id=1 SET old.id=1 WHERE old.`entry`=60801 AND old.`id`=0 AND old.`itemEntry1`=76289 AND old.`itemEntry2`=76289 AND old.`itemEntry3`=0 AND old.`CreatureID`=60801 AND old.`ItemID1`=76289 AND old.`ItemID2`=76289 AND old.`ItemID3`=0 AND valid.entry IS NULL;
UPDATE creature_equip_template old LEFT JOIN creature_equip_template valid ON valid.entry=old.entry AND valid.id=1 SET old.id=1 WHERE old.`entry`=60826 AND old.`id`=0 AND old.`itemEntry1`=82348 AND old.`itemEntry2`=0 AND old.`itemEntry3`=0 AND old.`CreatureID`=60826 AND old.`ItemID1`=82348 AND old.`ItemID2`=0 AND old.`ItemID3`=0 AND valid.entry IS NULL;
UPDATE creature_equip_template old LEFT JOIN creature_equip_template valid ON valid.entry=old.entry AND valid.id=1 SET old.id=1 WHERE old.`entry`=60912 AND old.`id`=0 AND old.`itemEntry1`=82349 AND old.`itemEntry2`=0 AND old.`itemEntry3`=0 AND old.`CreatureID`=60912 AND old.`ItemID1`=82349 AND old.`ItemID2`=0 AND old.`ItemID3`=0 AND valid.entry IS NULL;
DELETE old FROM creature_equip_template old JOIN creature_equip_template valid
 ON valid.entry=old.entry AND valid.id=1
 AND valid.itemEntry1=old.itemEntry1 AND valid.itemEntry2=old.itemEntry2 AND valid.itemEntry3=old.itemEntry3
 AND valid.CreatureID=old.CreatureID AND valid.ItemID1=old.ItemID1 AND valid.ItemID2=old.ItemID2 AND valid.ItemID3=old.ItemID3
 WHERE old.entry IN (61404,66825) AND old.id=0;
-- SkyFire 26.002 Flamekin Rager: enrage at 0-30 percent HP, not 30-0.
UPDATE smart_scripts SET event_param1=0,event_param2=30
 WHERE entryorguid=9779 AND source_type=0 AND id=0 AND link=1 AND event_type=2
 AND event_phase_mask=0 AND event_chance=100 AND event_flags=0
 AND event_param1=30 AND event_param2=0 AND event_param3=120000 AND event_param4=120000
 AND action_type=11 AND action_param1=15716 AND action_param2=1 AND action_param3=0
 AND action_param4=0 AND action_param5=0 AND action_param6=0 AND target_type=1
 AND target_param1=0 AND target_param2=0 AND target_param3=0 AND target_param4=0
 AND target_x=0 AND target_y=0 AND target_z=0 AND target_o=0 AND event_param5=0;
