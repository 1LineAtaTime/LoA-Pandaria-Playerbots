-- #963 (lane b): SmartAI talk rows that make the WRONG creature talk.
-- Root cause of most runtime "CreatureTextMgr: Could not find Text(Group) for Creature(X)" lines: in this core
-- SMART_ACTION_TALK with a creature target makes that creature the talker (SmartScript.cpp:121-145) unless
-- action_param3 (useTalkTarget) is 1. Rows that target the action invoker (7) or the victim (2) therefore make
-- whatever creature triggered the event talk: Dalaran/Shrine vendors via neighbouring vendors' OOC-LoS rows,
-- Fleeing Dreghood Warriors via the Illidari Taskmaster aggro row (447 lines/boot-hour), Dai-Lo Farmers via
-- Plump Virmen, Ebon Blade Prisoners / Skybreaker Infantry via Jotunheim/Ymirheim aggro rows, Frostflower /
-- Brad Rhodes via the Sha-Touched Footman.
-- Fix = the pattern TrinityCore and AzerothCore use for the same rows. Idempotent (plain UPDATEs / DELETE+INSERT).
-- Undo: set the listed columns back to 0 (event_param5 / action_param3), and move -84711 rows back to -84709.

-- (1) OOC-LoS "Say Line" to the invoker: only players may trigger (event_param5 = playerOnly, supported by this
--     core SmartScript.cpp:3244), and talk TO the invoker (useTalkTarget).
--     Source: AzerothCore acore_world smart_scripts 28989/0 (Aemara): event_param5=1, action_param3=1 (local AC DB,
--     2026-10-28); TrinityCore TDB 1210.26091 smart_scripts 14822/0 (Sayge OOC-LoS say) uses event_param5=1.
UPDATE `smart_scripts` SET `event_param5`=1, `action_param3`=1
WHERE `source_type`=0 AND `event_type`=10 AND `action_type`=1 AND `target_type`=7 AND (`entryorguid`,`id`) IN (
 (28347,1),(28714,0),(28715,0),(28718,0),(28721,0),(28723,0),(28725,0),(28726,0),(28727,0),(28989,0),
 (28994,0),(28997,0),(29491,0),(29523,0),(33027,0),(63013,1),(64149,2),(64164,1));

-- (2) On-aggro "Say" to the invoker: talk as self, to the invoker (useTalkTarget = 1).
--     Source: TrinityCore TDB_full_world_335.26091 smart_scripts 17058/2, 29880/4, 30243/2, 31258/4 all have
--     action_param3=1 with target_type 7; AzerothCore uses target_type 1 for 29880/30243/31258.
--     30632 (Mjordin Water Magus) and 68006/5 (Sha-Touched Footman, linked from an HP event whose invoker is the
--     attacker) follow the same pattern.
UPDATE `smart_scripts` SET `action_param3`=1
WHERE `source_type`=0 AND `action_type`=1 AND `target_type`=7 AND (`entryorguid`,`id`) IN (
 (17058,2),(29880,1),(30243,1),(30632,2),(31258,4),(68006,5));

-- (3) Plump Virmen on-aggro say with target VICTIM: talk as self to the victim (Dai-Lo Farmers are its victims).
--     Source: same useTalkTarget pattern as AC acore_world (4 rows event 4 / target 2 / action_param3 1).
UPDATE `smart_scripts` SET `action_param3`=1
WHERE `source_type`=0 AND `action_type`=1 AND `target_type`=2 AND (`entryorguid`,`id`) IN (
 (55483,0),(-562577,2),(-562724,2),(-563630,2),(-563648,3),(-562655,3),(-562663,2),(-562695,3));

-- (4) Brewfest Gordok Brew Barker (Durotar) script is keyed to guid 84709, which in this DB is Driz Tumblequick
--     (24510); the Durotar Gordok Brew Barker is guid 84711 (23685). Same 7 rows as -84635 (Dun Morogh barker).
--     Source: bromacia/TrinityCore-SymboCore sql/custom/gary_brewfest.sql (Durotar version keyed to the barker guid).
DELETE FROM `smart_scripts` WHERE `entryorguid`=-84711 AND `source_type`=0;
UPDATE `smart_scripts` SET `entryorguid`=-84711 WHERE `entryorguid`=-84709 AND `source_type`=0;

-- (5) guid 121192 is a Shoveltusk (23690) in this DB; the row "ELM General Purpose Bunny - On Data Set 0 1 - Say Line 0"
--     comes from a DB where 121192 is an ELM bunny (CPP TDB 434.22011 / SkyFire SFDB 26.002 carry the same row).
--     Nothing sets data on that Shoveltusk; the row is dead here.
DELETE FROM `smart_scripts` WHERE `entryorguid`=-121192 AND `source_type`=0 AND `id`=0 AND `action_type`=1;

-- (6) Talk rows whose text exists in no source; TrinityCore turned the same rows into a plain emote.
--     Source: TrinityCore TDB_full_world_335.26091 smart_scripts 15526/2 (Meridith the Mermaiden, gossip),
--     18938/2 (Krexcil, aggro) and 23669/3 (Winterskorn Oracle, in combat): action_type 5 (PLAY_EMOTE) action_param1 1.
--     No creature_text for these entries in TC335/TC434/TC442/TC1210, AC, SF548, EMU, AK; Wowpedia has no quote.
UPDATE `smart_scripts` SET `action_type`=5, `action_param1`=1, `action_param2`=0, `action_param3`=0
WHERE `source_type`=0 AND `action_type`=1 AND `action_param1`=0 AND (`entryorguid`,`id`) IN ((15526,2),(18938,2),(23669,3));
