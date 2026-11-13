-- #963 (2026-10-29, run v13: "Entry 60729 SourceType 0, Event 18, Link Event 19 not found or invalid"): Aysa Cloudsinger (Wandering
-- Isle) row 18 (Jump To Pos) links to row 19, but row 19 is event 58, not SMART_EVENT_LINK (61); same rows in reference DB 
-- The link was always skipped at runtime, so dropping it changes no behaviour (row 19 keeps its own event). Idempotent.
-- Undo: SET link = 19 on row 18.
UPDATE `smart_scripts` SET `link` = 0 WHERE `entryorguid` = 60729 AND `source_type` = 0 AND `id` = 18 AND `link` = 19;
