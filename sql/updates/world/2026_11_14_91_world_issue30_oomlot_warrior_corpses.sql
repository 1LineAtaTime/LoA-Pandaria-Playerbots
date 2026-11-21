-- Lost Isles, after 24901 "Town-In-A-Box: Under Attack": the dead Oomlot Warriors around the Town-In-A-Box could be
-- attacked, fought and killed for experience.
--
-- Why: the 31 Oomlot Warrior (38531) spawns of the aftermath phase (8192) are decoration. Their creature_addon gives them
-- Permanent Feign Death (29266) and Blacksmith Smoke (71333), so they lie on the ground like corpses, but they are live
-- creatures with the warriors' template flags (0x8000 only), the hostile Oomlot faction and the warriors' combat AI.
-- A player could target them, they fought back, and a kill gave experience and Town-In-A-Box kill credit (KillCredit1
-- 38536).
--
-- Fix: give these spawns, and only these, the flags of a lying-dead decoration. They keep 0x8000 and add
-- UNIT_FLAG_IMMUNE_TO_PC 0x100 + UNIT_FLAG_IMMUNE_TO_NPC 0x200 + UNIT_FLAG_NOT_SELECTABLE 0x02000000 = 33588992, set in the
-- per-spawn creature.unit_flags (ObjectMgr::ChooseCreatureFlags uses it in place of the template value when it is
-- non-zero). The 302 living warriors of the attack phase (4096, no addon auras) are not touched.
UPDATE `creature` c
JOIN `creature_addon` a ON a.`guid` = c.`guid`
SET c.`unit_flags` = 33588992
WHERE c.`id` = 38531 AND c.`map` = 648 AND c.`phaseMask` = 8192 AND a.`auras` = '71333 29266' AND c.`unit_flags` = 0;

-- Undo:
-- UPDATE `creature` c JOIN `creature_addon` a ON a.`guid` = c.`guid` SET c.`unit_flags` = 0
--   WHERE c.`id` = 38531 AND c.`map` = 648 AND c.`phaseMask` = 8192 AND a.`auras` = '71333 29266' AND c.`unit_flags` = 33588992;
