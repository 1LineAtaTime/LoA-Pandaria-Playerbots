-- Eastern Plaguelands: Fiona's parked caravan is seen from afar.
--
-- Sourced: both TDBs give the caravan wagon 45423 visibilityDistanceType 5 (Infinite), and players found the caravan by
--   looking for it on the way to the next tower ("press M and her caravan will appear on the map", Wowhead 2010 [+18]).
-- Ours: in this database the parked caravan is the harness 45416 carrying the gossip wagon 45400 as an accessory (45423 is
--   the moving wagon of a ride), so the far visibility goes on those two templates. Whether the 5.4.8 client then draws a
--   map icon for it is not known. Each stop's caravan still shows only to players who hold that stop's detection.
--
-- Neither entry has a per-spawn creature_addon row, so the template rows apply to all six stops.

UPDATE `creature_template_addon` SET `visibilityDistanceType`=5 WHERE `entry` IN (45400,45416);
