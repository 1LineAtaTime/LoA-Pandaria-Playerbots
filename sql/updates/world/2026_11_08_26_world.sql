-- Boulderfist Hall (area 1858): the Witherbark Trolls and Shadowcasters were tied to game event 43 "Hammerfall Under Attack", which ended on 2020-12-30, so none of them ever spawned.
-- Sourced: TDB 442 and 1210 spawn six Witherbarks here outside any event (285497, 285508, 285522, 285606, 285610, 285614; event 43 holds only their 127324/127325).
-- The six mop spawns that match those spots, entry for entry, leave the event and are always up: 160628, 160635, 160651, 160661 (with its path), 160832, 161016.
-- The other three stand on a TDB spot that already has its copy and go, as in the doubled-spawn cleanup 2026_11_08_12: 217337 (0.9 yd from 160635, same entry),
-- 217933 (a Troll 0.5 yd from Shadowcaster 160651, where the TDB has a Shadowcaster), 218665 (a Shadowcaster 7.6 yd from Troll 161016, where the TDB has one Troll).
-- None of the nine has pool, formation, linked-respawn or per-guid SmartAI rows; the three that go have no creature_addon. The event's own trolls west of Hammerfall are left alone.
DELETE FROM `game_event_creature` WHERE `eventEntry` = 43 AND `guid` IN (160628,160635,160651,160661,160832,161016,217337,217933,218665);
DELETE FROM `creature_addon` WHERE `guid` IN (217337,217933,218665);
DELETE FROM `creature` WHERE `guid` IN (217337,217933,218665) AND `id` IN (2552,2553);
