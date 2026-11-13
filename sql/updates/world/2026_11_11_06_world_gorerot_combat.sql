-- Pair with the native npc_gorerot_gilneas implementation.
-- VehicleId 629 previously caused passive VehicleAI to be selected.
-- Preserve the vehicle kit and all spawn/quest data; use explicit combat AI.
UPDATE creature_template SET AIName='', ScriptName='npc_gorerot_gilneas'
WHERE entry=38331;
