-- Call of Duty (14482 / 25924): the kraken scene at the Vashj'ir sea stop of the mercenary Ships to Vashj'ir.
--
-- Retail (Warcraft Wiki "Call of Duty (Alliance)" / "Call of Duty (Horde)", MediaWiki API, revisions 2026-07-30): at
-- Seafarer's Tomb the crew sights a wreck ("Captain! Shipwreck off the starboard bow! ..."), a kraken attacks, the
-- player is thrown into the sea and starts to drown, Erunak Stonespeaker kills the attacking naga ("To the depths with
-- you, beast!", "Hold on, <race>!") and the player wakes beside him in the Briny Cutter (Alliance) or the Immortal Coil
-- (Horde), with the quest done ("Ride the mercenary ship to Vashj'ir"). Patch 7.0.3 replaced the ride with a teleport.
-- No core or world database here scripts it: the ships' stop events (22520 / 25428) have no `event_scripts`, and the
-- crew / Erunak SmartAI is the same in every reference copy.
--
-- The scene is the TransportScript transport_ship_to_vashjir (src/server/scripts/EasternKingdoms/zone_vashjir.cpp),
-- the pattern of the zeppelin arrival scripts (TransportScript::OnRelocate at a stop node). This file only binds it.
--
-- Backup (state before this file): `gameobject_template`.`ScriptName` = '' for 197195 and 203466.
-- Undo:
--   UPDATE `gameobject_template` SET `ScriptName` = '' WHERE `entry` IN (197195, 203466);
-- Takes effect at the next worldserver start. Re-runnable.

UPDATE `gameobject_template` SET `ScriptName` = 'transport_ship_to_vashjir' WHERE `entry` IN (197195, 203466);
