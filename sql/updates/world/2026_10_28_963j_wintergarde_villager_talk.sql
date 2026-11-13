-- #963 (lane j): the Wintergarde caller of "CreatureTextMgr: Could not find Text for Creature(X) Entry N" for
--   7th Legion Infantryman 27160, Cavalier Durkon 27318 and Drolfy 27038 (none has creature_text or a talk script).
-- Caller: Trapped Wintergarde Villager 27359, smart_scripts id 5 "Linked To Id 4 - Say Line 0", target 12 (stored
-- list 1), action_param3 (useTalkTarget) 0. List 1 is not the villager's own: Vengeful Geist 27370 stores its killer
-- on death (id 0: event 6 DEATH, STORE_TARGET_LIST 1, target 7 = the killer), sends it to the closest villager
-- (id 1: SEND_TARGET_TO_TARGET) and sets data 0 1 on it (id 2). The villager then talks to that stored target.
-- SmartScript.cpp:121-145: a talk target that is a non-pet creature becomes the SPEAKER unless useTalkTarget = 1.
-- A player killer is fine (talker = villager); a creature killer speaks the villager's group 0 and SendChat fails
-- for its entry. The 18 geists stand among the Wintergarde NPCs (world DB: geist 117513 6 yd from Durkon 120666,
-- 117504 / 117509 13 yd from Drolfy 121110, villager 117497 on Drolfy's spot, geists 2488 / 34602 / 117506 within
-- 25-70 yd of the infantrymen), the NPCs kill them, and each such kill made the killer "talk".
-- Only cross-creature chain of its kind in the DB: every other talk-to-stored-list row (target 12, useTalkTarget 0)
-- stores a quest / gossip / summon / GO-use invoker (always a player), not a death / aggro killer.
-- Sources: AC acore_world 27359 calls action list 2735900, whose talk (id 4) targets self (1); the reference 5.4.8 DBs
-- (reference 5.4.8 world DBs) carry the same id 5 row as ours. Fix = the 963b / 963f pattern: the villager speaks, to
-- the stored killer (useTalkTarget = 1), so a player killer is unchanged and a creature killer no longer speaks.
-- Idempotent. Undo: UPDATE `smart_scripts` SET `action_param3`=0 WHERE `entryorguid`=27359 AND `source_type`=0 AND `id`=5;
UPDATE `smart_scripts` SET `action_param3`=1
WHERE `entryorguid`=27359 AND `source_type`=0 AND `id`=5 AND `action_type`=1 AND `target_type`=12;
