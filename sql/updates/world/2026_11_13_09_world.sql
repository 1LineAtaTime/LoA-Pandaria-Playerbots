-- Western Plaguelands, Andorhal: the living Benjamin Henderson (47717, guid 212073) stands at his stall instead of lying dead.
-- His spawn addon carried aura 31261 (a feign-death aura), so the Food & Drink vendor of the Alliance camp looked dead for the
-- whole chain. Sourced: neither TDB (4.4.2, 12.1.0) gives his spawn (327931) or his template an addon aura; Foreman Milos and
-- Alexis Torry beside him have no spawn addon at all, and his row held nothing else (all defaults), so it goes. His body after
-- the battle is a separate spawn (3127503, 2026_11_13_08_world.sql).
DELETE FROM `creature_addon` WHERE `guid`=212073;
