-- #963 (lane f): remaining "CreatureTextMgr: Could not find Text for Creature(X) Entry N" lines for
--   7th Legion Infantryman 27160, Cavalier Durkon 27318, Drolfy 27038 (Wintergarde Keep, Dragonblight),
--   Wild Imp 55659 (warlock guardian) and Treant 54984 (druid guardian).
-- None of the five has creature_text or a talking script of its own (world DB 2026-10-28: smart_scripts has no
-- action 1 / 84 row owned by them; no C++ script or playerbots code references these entries or calls
-- SendChat/Talk on them).
-- Mechanism (SmartScript.cpp:121-145): SMART_ACTION_TALK whose target resolves to a non-Pet creature makes THAT
-- creature the speaker unless action_param3 (useTalkTarget) = 1. The IsPet() filter does not cover guardians, so a
-- Wild Imp / Treant that is the victim (target 2) or invoker (target 7) of an aggro / low-HP / death / spell-hit /
-- OOC-LoS talk row of another mob becomes the talker and SendChat fails for its entry. The same happens when
-- ANY friendly creature (Wintergarde NPCs included) is the invoker of such a row.
-- Caller search result: the exact Wintergarde caller could not be pinned to one row. All 223 remaining rows below
-- are the generic "target 2 / 7, useTalkTarget 0" pattern; no mob that has such a row is spawned within 400 yards of
-- the Wintergarde NPCs on map 571 (summoned / patrolling mobs are not in `creature`), so the Wintergarde hits come
-- through the same pattern via a summoned or wandering mob. Fixing the whole pattern closes it for all five entries.
-- Fix = the TrinityCore / AzerothCore pattern for the same rows (see 2026_10_28_963b header): the owner speaks, to
-- the invoker / victim (action_param3 = 1). TC TDB 1210.26091 smart_scripts 17058/2, 29880/4, 30243/2, 31258/4;
-- AC acore_world 28989/0, 28041/3 style rows. A player target is unaffected (talker was already `me`), so only
-- creature invokers / victims change behaviour: they stop speaking in place of the row's owner.
-- Left alone on purpose: event 38 (data set), 40 (waypoint), 52 (text over), 54 (just summoned), and quest /
-- gossip events 19, 20, 62 (invoker is always the player); source_type 9 lists.
-- Idempotent (re-running sets the same value). Undo: set action_param3 back to 0 on the same rows.
UPDATE `smart_scripts` SET `action_param3`=1
WHERE `source_type`=0 AND `action_type`=1 AND `target_type` IN (2,7) AND `action_param3`=0
  AND `event_type` NOT IN (19,20,62,38,40,52,54);

