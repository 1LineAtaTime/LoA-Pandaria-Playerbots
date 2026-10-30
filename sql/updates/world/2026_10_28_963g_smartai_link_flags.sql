-- #963 (2026-10-28, run v8 DBErrors.log):
-- * 20763 Captured Protectorate Vanguard: row 0 (On Respawn - Set NpcFlag) links to row 1, which is an On Quest Accepted
--   event, not SMART_EVENT_LINK (61): "Link Event 1 not found or invalid" at every respawn. Same rows in reference DB 
--   The link is dropped; the respawn action itself still runs.
-- * 5649 Sandfury Blood Drinker: its only SmartAI row carried event_flags 2 (normal dungeon only), so a spawn outside the
--   instance got "no events added to list because of instance flags". AC acore_world uses event_flags 0 for it.
-- Idempotent. Undo: link 1 on 20763/0; event_flags 2 on 5649/0.
UPDATE `smart_scripts` SET `link` = 0 WHERE `entryorguid` = 20763 AND `source_type` = 0 AND `id` = 0 AND `link` = 1;
UPDATE `smart_scripts` SET `event_flags` = 0 WHERE `entryorguid` = 5649 AND `source_type` = 0 AND `id` = 0 AND `event_flags` = 2;
