-- SmartAI: combat and line-of-sight says are spoken by the creature itself
-- SMART_ACTION_TALK makes a creature target say the line (only with action_param3 = 1 does the script owner speak to
-- it). Rows that "say" to the invoker (7) or the victim (2) on combat, health, kill, death, spell hit and line-of-sight
-- events therefore hand the line to whatever creature, guardian or totem triggered them, which has no such text
-- ("CreatureTextMgr: Could not find Text..."): an Illidari Taskmaster's aggro line went to the Fleeing Dreghood
-- Warriors that attack it, about 900 times an hour. A player invoker is unaffected either way: with target self (1)
-- the owner speaks and SmartScript still passes the invoker as the text target, so $n and whispers keep working.
-- Quest, gossip, emote and data-set events are left alone: their invoker is a player, or another creature on purpose.

SET @EVENTS := '0,1,2,4,5,6,8,9,10,23,32';

-- Links two levels below such an event, then one level, then the rows themselves
UPDATE `smart_scripts` AS `c`
JOIN `smart_scripts` AS `p` ON `p`.`entryorguid` = `c`.`entryorguid` AND `p`.`source_type` = `c`.`source_type` AND `p`.`link` = `c`.`id`
JOIN `smart_scripts` AS `g` ON `g`.`entryorguid` = `p`.`entryorguid` AND `g`.`source_type` = `p`.`source_type` AND `g`.`link` = `p`.`id`
SET `c`.`target_type` = 1
WHERE `c`.`source_type` = 0 AND `c`.`event_type` = 61 AND `c`.`action_type` = 1 AND `c`.`action_param3` = 0 AND `c`.`target_type` IN (2, 7)
  AND `p`.`event_type` = 61 AND FIND_IN_SET(`g`.`event_type`, @EVENTS);

UPDATE `smart_scripts` AS `c`
JOIN `smart_scripts` AS `p` ON `p`.`entryorguid` = `c`.`entryorguid` AND `p`.`source_type` = `c`.`source_type` AND `p`.`link` = `c`.`id`
SET `c`.`target_type` = 1
WHERE `c`.`source_type` = 0 AND `c`.`event_type` = 61 AND `c`.`action_type` = 1 AND `c`.`action_param3` = 0 AND `c`.`target_type` IN (2, 7)
  AND FIND_IN_SET(`p`.`event_type`, @EVENTS);

UPDATE `smart_scripts`
SET `target_type` = 1
WHERE `source_type` = 0 AND `action_type` = 1 AND `action_param3` = 0 AND `target_type` IN (2, 7) AND FIND_IN_SET(`event_type`, @EVENTS);
