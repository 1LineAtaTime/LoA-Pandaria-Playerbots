-- #963 (2026-10-29, v14 boot: "Entry 34872 ... Event 3 Action 1 using non-existent Text id 2"): Foreman Dampwick row 3 is a
-- TEXT_OVER (52) with creatureEntry 0 (any creature). The load check (AC/TC IsTextValid) validates a TEXT_OVER talk against
-- event_param2, so 0 rejects the row. Dampwick's own text group 1 ending is the intended trigger: set param2 = 34872.
-- Idempotent. Undo: event_param2 = 0.
UPDATE `smart_scripts` SET `event_param2` = 34872 WHERE `entryorguid` = 34872 AND `source_type` = 0 AND `id` = 3 AND `event_type` = 52 AND `event_param2` = 0;
