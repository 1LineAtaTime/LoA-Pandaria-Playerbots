-- Restore registered Gate of the Setting Sun progression triggers from matching LoA reference.
-- Existing nonempty custom trigger bindings win; no character quest/boss progress is forced.
INSERT INTO areatrigger_scripts(entry,ScriptName) VALUES
(7104,'at_before_gadok'),
(7332,'at_destroy_corner_a'),
(7665,'at_destroy_corner_b'),
(8359,'at_first_door'),
(8395,'at_intro_gadok')
ON DUPLICATE KEY UPDATE ScriptName=IF(ScriptName='',VALUES(ScriptName),ScriptName);
