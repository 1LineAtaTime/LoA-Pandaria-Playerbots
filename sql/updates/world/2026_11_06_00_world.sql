-- Hillsbrad Foothills: Angry Blight Slime 48319 starts Angry Scrubbing Bubbles (28329) from its On Death smart_scripts row
-- (2026_11_04_30_world.sql), but that file does not set its AIName: it counted on an abilities update having done so. Without
-- SmartAI the row never runs and the quest cannot start.
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=48319 AND `ScriptName`='';
