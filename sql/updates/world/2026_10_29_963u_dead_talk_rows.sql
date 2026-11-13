-- #963 (2026-10-29, v15 boot: "EntryOrGuid N using event(N) has invalid action type (0), skipped" x5): 963t turned 5 talk
-- rows without any known text into action 0, but the loader rejects action 0, so a row that carried a link also dropped
-- its linked action. Delete the dead rows; where they linked on, give the linked row the dead row's event instead.
-- Idempotent. Undo: re-run 963t's row values (see that file) and restore event 61 + link on 30474/9 and 46276/2.
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `action_type` = 0 AND
  ((`entryorguid` = -84714 AND `id` = 0) OR (`entryorguid` = 46425 AND `id` = 11) OR (`entryorguid` = 54615 AND `id` = 0));
-- North Wind: Die on data set 0 2 (was: say text 2, linked Die)
UPDATE `smart_scripts` SET `event_type` = 38, `event_param1` = 0, `event_param2` = 2, `comment` = 'North Wind - On data set 0 2 - Die (#963u)'
  WHERE `entryorguid` = 30474 AND `source_type` = 0 AND `id` = 9 AND `event_type` = 61;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 30474 AND `source_type` = 0 AND `id` = 8 AND `action_type` = 0;
-- Caimas the Pit Master: Attack Start on aggro (was: say line 4, linked Attack Start)
UPDATE `smart_scripts` SET `event_type` = 4, `event_param1` = 0, `event_param2` = 0, `comment` = 'Caimas the Pit Master - On Aggro - Attack Start (#963u)'
  WHERE `entryorguid` = 46276 AND `source_type` = 0 AND `id` = 2 AND `event_type` = 61;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 46276 AND `source_type` = 0 AND `id` = 1 AND `action_type` = 0;
