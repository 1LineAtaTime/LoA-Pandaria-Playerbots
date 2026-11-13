-- #963 (2026-10-28, run v10): SMART_ACTION_COMBAT_STOP (203) on this core also ends the linked chain (leaving combat
-- resets the SmartAI after UPDATE_TEMPLATE), the same as EVADE did (963i): "Entry 16937 ... Event 5, Link Event 6 not
-- found or invalid". The combat stop now runs LAST in the chain: say, random movement and the 10 s despawn timer (kept by
-- the creature itself) run first. Rows 5-8 of 16937 / 16938 rewritten in that order; text, faction, emote rows unchanged.
-- Idempotent (absolute values). Undo: 963i order (5 combat stop, 6 say, 7 random movement, 8 despawn).
UPDATE `smart_scripts` SET `action_type` = 1,     `action_param1` = 0,     `link` = 6, `comment` = 'Dreghood - Linked - Say Line 0 (as Fleeing Dreghood Warrior)' WHERE `entryorguid` IN (16937, 16938) AND `source_type` = 0 AND `id` = 5;
UPDATE `smart_scripts` SET `action_type` = 89,    `action_param1` = 10,    `link` = 7, `comment` = 'Dreghood - Linked - Start Random Movement 10' WHERE `entryorguid` IN (16937, 16938) AND `source_type` = 0 AND `id` = 6;
UPDATE `smart_scripts` SET `action_type` = 41,    `action_param1` = 10000, `link` = 8, `comment` = 'Dreghood - Linked - Despawn In 10000 ms' WHERE `entryorguid` IN (16937, 16938) AND `source_type` = 0 AND `id` = 7;
UPDATE `smart_scripts` SET `action_type` = 203,   `action_param1` = 0,     `link` = 0, `comment` = 'Dreghood - Linked - Combat Stop (last: it ends the chain)' WHERE `entryorguid` IN (16937, 16938) AND `source_type` = 0 AND `id` = 8;
