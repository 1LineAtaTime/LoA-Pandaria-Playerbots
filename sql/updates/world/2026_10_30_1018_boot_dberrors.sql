-- #1018: eight DBErrors.log lines that are new since v1.0.4. Idempotent.
-- Sources: AC = AzerothCore world DB; reference = the reference 5.4.8 world DB; client SpellEffect.dbc read directly.

-- ---------------------------------------------------------------------------------------------------------------
-- pool 7512 (child of 1075745): "has no equal-chance entries and its chance sum is not 100".
-- The pool held 4 Copper Vein / Tin Vein nodes (guid 10983 at 70, guids 82312-82314 at 10 each = 100, same in the reference).
-- The gathering-node ground-fix update deleted the three 10-chance nodes and left the 70 one. One member left: make it
-- the equal-chance entry (chance 0), as a one-member pool needs.
UPDATE `pool_gameobject` SET `chance`=0 WHERE `pool_entry`=7512 AND `guid`=10983 AND `chance`=70;

-- ---------------------------------------------------------------------------------------------------------------
-- SmartAI 4484 (Feero Ironhand) row 24 "non-existent text id 5":
--   TEXT_OVER of creature 3899 group 0 (event_param2=3899) is validated against creature 3899's creature_text, which has
--   only group 0, so "Say line 5" (a line of 4484) can never validate. AC has the same trigger as "Play Emote 1"
--   (id 18). Use AC's action; row 25 (set data, linked) is left as is.
UPDATE `smart_scripts` SET `action_type`=5, `action_param1`=1, `action_param2`=0, `action_param3`=0, `action_param4`=0,
  `action_param5`=0, `action_param6`=0,
  `comment`='Feero Ironhand - On Text Over line 0 (Balizar the Umbrage) - Play Emote 1'
WHERE `entryorguid`=4484 AND `source_type`=0 AND `id`=24 AND `event_type`=52 AND `event_param2`=3899 AND `action_type`=1;

-- SmartAI 30945 (Vardmadra) row 9 "non-existent text id 3":
--   same shape: TEXT_OVER of creature 30955 group 0 with "Say line 3" validated against creature 30955's texts (group 0 only).
--   AC has no such row (its ids go 8 -> 10). Remove it as AC does.
DELETE FROM `smart_scripts` WHERE `entryorguid`=30945 AND `source_type`=0 AND `id`=9 AND `event_type`=52 AND `event_param2`=30955;

-- ---------------------------------------------------------------------------------------------------------------
-- creatures 45405 (Master Sergeant Pietro Zaren) and 48912 (Stormpike Trainee): smart_scripts rows added by the quest
-- ports, AIName left empty. Both are plain creatures in the reference (generic script) and absent in AC, so only the
-- AIName is missing.
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (45405,48912) AND `AIName`='';

-- ---------------------------------------------------------------------------------------------------------------
-- spells 49986 and 65042 in spell_target_position: "does not have target TARGET_DEST_DB".
-- The client SpellEffect data has no effect with target DEST_DB for either spell (49986: one effect, unit target;
-- 65042: effect 0 target 46, effect 1 teleport with targets 25 and 87), so neither effect index can ever load a
-- position. Moving the rows between effect indexes (earlier update) cannot work. The rows were never used; remove them.
DELETE FROM `spell_target_position` WHERE `id` IN (49986,65042);

-- ---------------------------------------------------------------------------------------------------------------
-- creature 1365 (Goli Krumn) starts quest 7062 but has no questgiver flag. Starter row is intentional (AC and the newer
-- 4.4.2 data list it); AC has npcflag 2 on this creature.
UPDATE `creature_template` SET `npcflag`=`npcflag`|2 WHERE `entry`=1365;
