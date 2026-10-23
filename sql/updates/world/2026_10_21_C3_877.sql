-- Refs #877 (lane C3): MoP 1000-bot world diff degrades after ~1 h - Badlands area 5322 grid[19,26] cells 152-154 x 209-211.
--
-- Root cause 1 (the cost, unbounded): Cave In Trigger 47128 carries creature_template_addon aura 87842 "Cave In"
--   (5.4.8 SpellEffect.dbc: APPLY_AURA 23 PERIODIC_TRIGGER_SPELL, amplitude 1000 ms, trigger 87843). 87843 is
--   SPELL_EFFECT_PERSISTENT_AREA_AURA (27, radius idx 29 = 6 yd, TARGET_SRC_CASTER / SRC_AREA_ENEMY) with
--   SpellMisc DurationIndex 21 = -1 (infinite). Every trigger therefore creates one never-expiring DynamicObject per
--   second for as long as its grid is active (Spell::EffectPersistentAA SpellEffects.cpp:1992-2023; DynamicObject::Update
--   DynamicObject.cpp:176-193 only expires a dynobj when its aura expires). 15 of the 23 triggers sit in the hot cells:
--   ~54,000 dynobjs per hour. AC carries the same code (AC SpellEffects.cpp:1946-1982, DynamicObject.cpp:134-160), so
--   this is Cata data, not a core divergence. A scan of every creature_template_addon/creature_addon aura against the
--   5.4.8 DBC finds this the ONLY periodic-trigger -> infinite persistent-area-aura pair (the other, 100223 -> 100222,
--   lasts 20 s, so it is bounded). The trigger belongs to the unscripted "The Day that Deathwing Came" scenes
--   (quests 27713-27715, no ScriptName/SmartAI on world or in the MoP 5.4.8 data).
-- Root cause 2 (the keeper): Deathwing 46471 (spawns 161582/161640, Health_mod 10000 = 858,920,000 hp, faction 14,
--   creature_classlevelstats L87 class 1 damage_exp3 = 0 -> no melee damage) is a quest-scene NPC (Wowpedia
--   "The Day that Deathwing Came", quest 27713: punched only while possessing Theldurin; 858 million health) that is
--   attackable by everyone here. Low-level bots engage it, cannot kill it and are never hurt, so they stay in combat
--   for hours, which keeps the grid (and the leaking triggers) active. Same rows in the MoP 5.4.8 data.
--
-- Pre-state (test world DB 2026-10-21): creature_template_addon 47128 auras = '87842'; creature_template 46471
-- unit_flags = 32768 (UNIT_FLAG_CAN_SWIM); no creature_addon rows for either entry; spawn unit_flags 0 (template wins).
--
-- Undo:
--   UPDATE `creature_template_addon` SET `auras` = '87842' WHERE `entry` = 47128;
--   UPDATE `creature_template` SET `unit_flags` = `unit_flags` & ~0x302 WHERE `entry` = 46471;

-- 1) Cave In Trigger: no self-repeating infinite persistent area aura.
UPDATE `creature_template_addon` SET `auras` = NULL WHERE `entry` = 47128;

-- 2) Deathwing (Badlands scene): NON_ATTACKABLE 0x2 | IMMUNE_TO_PC 0x100 | IMMUNE_TO_NPC 0x200.
--    Players/bots cannot attack it (Unit::IsValidAttackTarget Unit.cpp:11995; bot AttackersValue.cpp:171-174 = AC
--    AttackersValue.cpp:153-157) and it never aggroes (Creature::CanStartAttack Creature.cpp:1633).
UPDATE `creature_template` SET `unit_flags` = `unit_flags` | 0x302 WHERE `entry` = 46471;
