-- #1020: SMART_EVENT_RESPAWN now fires once on a summon's first appearance, in InitializeAI, like AC (AC SmartAI.cpp:1080;
-- AC Creature.cpp:718 skips JustRespawned for temp summons). InitializeAI runs inside Map::SummonCreature (AddToMap), before
-- the summon spell seats the player. For a SUMMON_CATEGORY_VEHICLE summon (SummonProperties 161) the seating comes after
-- that, and the bat parked when its path started in InitializeAI (build v24).
-- These two summon-only vehicles start their flight path when the player boards, AC's pattern for ridden flight paths
-- (Heb'Jin's Bat 28639, Brann's Flying Machine 30134, Skeletal Assault Gryphon 31157: "On Passenger Boarded - Start
-- Waypoint"). The player is the only passenger. Both are summoned by a VEHICLE spell that seats the caster in the same call.
--   Captured Riding Bat 38540 (spell 72472, Gilneas 24920)
--   Eagle Spirit 36790 (spell 69340)
-- event_type 27 = SMART_EVENT_PASSENGER_BOARDED (cooldown min/max 0).
UPDATE `smart_scripts` SET `event_type`=27, `comment`='Captured Riding Bat - On Passenger Boarded - Waypoint Start'
WHERE `entryorguid`=38540 AND `source_type`=0 AND `id`=0 AND `event_type`=11;
UPDATE `smart_scripts` SET `event_type`=27, `comment`='Eagle Spirit - On Passenger Boarded - Waypoint Start'
WHERE `entryorguid`=36790 AND `source_type`=0 AND `id`=0 AND `event_type`=11;
