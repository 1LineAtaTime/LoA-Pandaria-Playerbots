-- =====================================================================================
-- Issue #175: Alterac Valley NPC stats and per-bracket scaling as 5.4.8.
-- Target: LoA world DB (`world`). Test-only AC parity for the level-80 1:1 runs.
--
-- Mechanism (existing LoA core, no schema change): creature_template_difficulty rows
-- 'BG_DIFFICULTY_<n>', n = PvpDifficulty.dbc RangeIndex of the battleground's bracket
-- (ObjectMgr::SelectDifficultyInfo, ObjectMgr.cpp:10945-10952; enum value = n + MAX_DIFFICULTY(15)).
-- AV (map 30) 5.4.8 brackets: 0=45-49 1=50-54 2=55-59 3=60-64 4=65-69 5=70-74 6=75-79 7=80-84 8=85-89 9=90.
-- A row sets level_min/level_max, health_mod and DamageModifier for that bracket (the per-bracket
-- equivalent of AC's difficulty_entry_1..3 templates). Rows survive Creature::Respawn (SelectLevel re-reads them).
--
-- REQUIRES core change C2 (Creature.cpp:1229-1238 and :1273, see the .md): the damage column must follow the
-- same bracket expansion as health, and level<=71 must use the classic columns. Without C2 every level-86+ AV
-- NPC deals ~0 melee damage (damage_base/damage_exp1 are 0 at levels 86-93) and the DamageModifier values below
-- are wrong for all brackets.
--
-- Levels: bracketMax + (vanilla level - 60): boss 61, captain/marshal/warmaster 60, Duros/Drakan 60,
--   defender 58, seasoned 59, veteran 60, champion 61, bowman 59-60, guardsman/legionnaire 57, battleguard 61.
--   Sourced end points: Wowhead 2013 (5.2/5.3) marshal/warmaster 49-90, Stormpike Defender/Frostwolf Guardian 47-88.
-- health_mod: brackets 0-8 = template Health_mod (= retail creature-cache modifier; Wowhead 2014: captains
--   19,697,050 = 50 x L90 base, Drek'Thar 28,527,240 = 70 x L91 base). Bracket 9 = patch 5.4.7 buff
--   (Drek'Thar 150: in-game 5.4.7 61,119k = 150 x L91 base; others from the MoP 5.4.8 data BG_DIFFICULTY_9).
-- DamageModifier: brackets 0-7 = one value per NPC giving AC's 80-bracket per-hit damage at the bracket-7 level
--   (inferred; AC is the spec where 5.4.8 has no source). Bracket 8 = pre-5.4.7 boss anchor (Vanndar 5.1: 40-60k
--   main-hand) x AC relative strengths (inferred). Bracket 9 = 5.4.7 anchor (Drek'Thar 80-120k main-hand; Duros/
--   Drakan 7-9k) x MoP BG_DIFFICULTY_9 relative strengths.
--   Faction pairs are symmetric (AC itself is not: its warmasters have DamageModifier 51.4 vs marshals 13.8).
-- =====================================================================================

-- 1. Replace every AV BG_DIFFICULTY_* row of these NPCs (the old BG_DIFFICULTY_1..3 rows are 3.3.5/4.x
--    difficulty-1..3 data - levels 70-85 - that the 5.4.8 bracket indices apply to the 50-64 brackets).
DELETE FROM `creature_template_difficulty` WHERE `id` IN (11946,11947,11948,11949,12050,12051,12053,12121,12122,12127,13326,13328,13331,13332,13358,13359,13421,13422,14284,14285,14762,14763,14764,14765,14772,14773,14776,14777)
  AND `difficulty` IN ('BG_DIFFICULTY_0','BG_DIFFICULTY_1','BG_DIFFICULTY_2','BG_DIFFICULTY_3','BG_DIFFICULTY_4',
                       'BG_DIFFICULTY_5','BG_DIFFICULTY_6','BG_DIFFICULTY_7','BG_DIFFICULTY_8','BG_DIFFICULTY_9');

INSERT INTO `creature_template_difficulty` (`id`, `difficulty`, `health_mod`, `DamageModifier`, `level_min`, `level_max`) VALUES
(11946, 'BG_DIFFICULTY_0', 70, 97.54, 50, 50), -- Drek'Thar
(11946, 'BG_DIFFICULTY_1', 70, 97.54, 55, 55),
(11946, 'BG_DIFFICULTY_2', 70, 97.54, 60, 60),
(11946, 'BG_DIFFICULTY_3', 70, 97.54, 65, 65),
(11946, 'BG_DIFFICULTY_4', 70, 97.54, 70, 70),
(11946, 'BG_DIFFICULTY_5', 70, 97.54, 75, 75),
(11946, 'BG_DIFFICULTY_6', 70, 97.54, 80, 80),
(11946, 'BG_DIFFICULTY_7', 70, 97.54, 85, 85),
(11946, 'BG_DIFFICULTY_8', 70, 2.73, 90, 90),
(11946, 'BG_DIFFICULTY_9', 150, 4.27, 91, 91),
(11947, 'BG_DIFFICULTY_0', 50, 53.98, 49, 49), -- Captain Galvangar
(11947, 'BG_DIFFICULTY_1', 50, 53.98, 54, 54),
(11947, 'BG_DIFFICULTY_2', 50, 53.98, 59, 59),
(11947, 'BG_DIFFICULTY_3', 50, 53.98, 64, 64),
(11947, 'BG_DIFFICULTY_4', 50, 53.98, 69, 69),
(11947, 'BG_DIFFICULTY_5', 50, 53.98, 74, 74),
(11947, 'BG_DIFFICULTY_6', 50, 53.98, 79, 79),
(11947, 'BG_DIFFICULTY_7', 50, 53.98, 84, 84),
(11947, 'BG_DIFFICULTY_8', 50, 2.09, 89, 89),
(11947, 'BG_DIFFICULTY_9', 125, 3.44, 90, 90),
(11948, 'BG_DIFFICULTY_0', 70, 97.54, 50, 50), -- Vanndar Stormpike
(11948, 'BG_DIFFICULTY_1', 70, 97.54, 55, 55),
(11948, 'BG_DIFFICULTY_2', 70, 97.54, 60, 60),
(11948, 'BG_DIFFICULTY_3', 70, 97.54, 65, 65),
(11948, 'BG_DIFFICULTY_4', 70, 97.54, 70, 70),
(11948, 'BG_DIFFICULTY_5', 70, 97.54, 75, 75),
(11948, 'BG_DIFFICULTY_6', 70, 97.54, 80, 80),
(11948, 'BG_DIFFICULTY_7', 70, 97.54, 85, 85),
(11948, 'BG_DIFFICULTY_8', 70, 2.73, 90, 90),
(11948, 'BG_DIFFICULTY_9', 150, 4.27, 91, 91),
(11949, 'BG_DIFFICULTY_0', 50, 56.27, 49, 49), -- Captain Balinda Stonehearth
(11949, 'BG_DIFFICULTY_1', 50, 56.27, 54, 54),
(11949, 'BG_DIFFICULTY_2', 50, 56.27, 59, 59),
(11949, 'BG_DIFFICULTY_3', 50, 56.27, 64, 64),
(11949, 'BG_DIFFICULTY_4', 50, 56.27, 69, 69),
(11949, 'BG_DIFFICULTY_5', 50, 56.27, 74, 74),
(11949, 'BG_DIFFICULTY_6', 50, 56.27, 79, 79),
(11949, 'BG_DIFFICULTY_7', 50, 56.27, 84, 84),
(11949, 'BG_DIFFICULTY_8', 50, 2.23, 89, 89),
(11949, 'BG_DIFFICULTY_9', 125, 3.44, 90, 90),
(12050, 'BG_DIFFICULTY_0', 0.4, 1.5, 47, 47), -- Stormpike Defender
(12050, 'BG_DIFFICULTY_1', 0.4, 1.5, 52, 52),
(12050, 'BG_DIFFICULTY_2', 0.4, 1.5, 57, 57),
(12050, 'BG_DIFFICULTY_3', 0.4, 1.5, 62, 62),
(12050, 'BG_DIFFICULTY_4', 0.4, 1.5, 67, 67),
(12050, 'BG_DIFFICULTY_5', 0.4, 1.5, 72, 72),
(12050, 'BG_DIFFICULTY_6', 0.4, 1.5, 77, 77),
(12050, 'BG_DIFFICULTY_7', 0.4, 1.5, 82, 82),
(12050, 'BG_DIFFICULTY_8', 0.4, 0.09, 87, 87),
(12050, 'BG_DIFFICULTY_9', 2, 4.21, 88, 88),
(12051, 'BG_DIFFICULTY_0', 0.6, 1.56, 46, 46), -- Frostwolf Legionnaire
(12051, 'BG_DIFFICULTY_1', 0.6, 1.56, 51, 51),
(12051, 'BG_DIFFICULTY_2', 0.6, 1.56, 56, 56),
(12051, 'BG_DIFFICULTY_3', 0.6, 1.56, 61, 61),
(12051, 'BG_DIFFICULTY_4', 0.6, 1.56, 66, 66),
(12051, 'BG_DIFFICULTY_5', 0.6, 1.56, 71, 71),
(12051, 'BG_DIFFICULTY_6', 0.6, 1.56, 76, 76),
(12051, 'BG_DIFFICULTY_7', 0.6, 1.56, 81, 81),
(12051, 'BG_DIFFICULTY_8', 0.6, 0.1, 86, 86),
(12051, 'BG_DIFFICULTY_9', 0.6, 3.58, 87, 87),
(12053, 'BG_DIFFICULTY_0', 0.4, 1.5, 47, 47), -- Frostwolf Guardian
(12053, 'BG_DIFFICULTY_1', 0.4, 1.5, 52, 52),
(12053, 'BG_DIFFICULTY_2', 0.4, 1.5, 57, 57),
(12053, 'BG_DIFFICULTY_3', 0.4, 1.5, 62, 62),
(12053, 'BG_DIFFICULTY_4', 0.4, 1.5, 67, 67),
(12053, 'BG_DIFFICULTY_5', 0.4, 1.5, 72, 72),
(12053, 'BG_DIFFICULTY_6', 0.4, 1.5, 77, 77),
(12053, 'BG_DIFFICULTY_7', 0.4, 1.5, 82, 82),
(12053, 'BG_DIFFICULTY_8', 0.4, 0.09, 87, 87),
(12053, 'BG_DIFFICULTY_9', 2, 4.21, 88, 88),
(12121, 'BG_DIFFICULTY_0', 3, 5.15, 49, 49), -- Drakan
(12121, 'BG_DIFFICULTY_1', 3, 5.15, 54, 54),
(12121, 'BG_DIFFICULTY_2', 3, 5.15, 59, 59),
(12121, 'BG_DIFFICULTY_3', 3, 5.15, 64, 64),
(12121, 'BG_DIFFICULTY_4', 3, 5.15, 69, 69),
(12121, 'BG_DIFFICULTY_5', 3, 5.15, 74, 74),
(12121, 'BG_DIFFICULTY_6', 3, 5.15, 79, 79),
(12121, 'BG_DIFFICULTY_7', 3, 5.15, 84, 84),
(12121, 'BG_DIFFICULTY_8', 3, 0.19, 89, 89),
(12121, 'BG_DIFFICULTY_9', 3, 0.48, 90, 90),
(12122, 'BG_DIFFICULTY_0', 3, 4.99, 49, 49), -- Duros
(12122, 'BG_DIFFICULTY_1', 3, 4.99, 54, 54),
(12122, 'BG_DIFFICULTY_2', 3, 4.99, 59, 59),
(12122, 'BG_DIFFICULTY_3', 3, 4.99, 64, 64),
(12122, 'BG_DIFFICULTY_4', 3, 4.99, 69, 69),
(12122, 'BG_DIFFICULTY_5', 3, 4.99, 74, 74),
(12122, 'BG_DIFFICULTY_6', 3, 4.99, 79, 79),
(12122, 'BG_DIFFICULTY_7', 3, 4.99, 84, 84),
(12122, 'BG_DIFFICULTY_8', 3, 0.19, 89, 89),
(12122, 'BG_DIFFICULTY_9', 3, 0.48, 90, 90),
(12127, 'BG_DIFFICULTY_0', 0.6, 1.56, 46, 46), -- Stormpike Guardsman
(12127, 'BG_DIFFICULTY_1', 0.6, 1.56, 51, 51),
(12127, 'BG_DIFFICULTY_2', 0.6, 1.56, 56, 56),
(12127, 'BG_DIFFICULTY_3', 0.6, 1.56, 61, 61),
(12127, 'BG_DIFFICULTY_4', 0.6, 1.56, 66, 66),
(12127, 'BG_DIFFICULTY_5', 0.6, 1.56, 71, 71),
(12127, 'BG_DIFFICULTY_6', 0.6, 1.56, 76, 76),
(12127, 'BG_DIFFICULTY_7', 0.6, 1.56, 81, 81),
(12127, 'BG_DIFFICULTY_8', 0.6, 0.1, 86, 86),
(12127, 'BG_DIFFICULTY_9', 0.6, 3.58, 87, 87),
(13326, 'BG_DIFFICULTY_0', 0.5, 1.75, 48, 48), -- Seasoned Defender
(13326, 'BG_DIFFICULTY_1', 0.5, 1.75, 53, 53),
(13326, 'BG_DIFFICULTY_2', 0.5, 1.75, 58, 58),
(13326, 'BG_DIFFICULTY_3', 0.5, 1.75, 63, 63),
(13326, 'BG_DIFFICULTY_4', 0.5, 1.75, 68, 68),
(13326, 'BG_DIFFICULTY_5', 0.5, 1.75, 73, 73),
(13326, 'BG_DIFFICULTY_6', 0.5, 1.75, 78, 78),
(13326, 'BG_DIFFICULTY_7', 0.5, 1.75, 83, 83),
(13326, 'BG_DIFFICULTY_8', 0.5, 0.08, 88, 88),
(13326, 'BG_DIFFICULTY_9', 2.5, 3.89, 89, 89),
(13328, 'BG_DIFFICULTY_0', 0.5, 1.75, 48, 48), -- Seasoned Guardian
(13328, 'BG_DIFFICULTY_1', 0.5, 1.75, 53, 53),
(13328, 'BG_DIFFICULTY_2', 0.5, 1.75, 58, 58),
(13328, 'BG_DIFFICULTY_3', 0.5, 1.75, 63, 63),
(13328, 'BG_DIFFICULTY_4', 0.5, 1.75, 68, 68),
(13328, 'BG_DIFFICULTY_5', 0.5, 1.75, 73, 73),
(13328, 'BG_DIFFICULTY_6', 0.5, 1.75, 78, 78),
(13328, 'BG_DIFFICULTY_7', 0.5, 1.75, 83, 83),
(13328, 'BG_DIFFICULTY_8', 0.5, 0.08, 88, 88),
(13328, 'BG_DIFFICULTY_9', 2.5, 3.89, 89, 89),
(13331, 'BG_DIFFICULTY_0', 0.6, 1.99, 49, 49), -- Veteran Defender
(13331, 'BG_DIFFICULTY_1', 0.6, 1.99, 54, 54),
(13331, 'BG_DIFFICULTY_2', 0.6, 1.99, 59, 59),
(13331, 'BG_DIFFICULTY_3', 0.6, 1.99, 64, 64),
(13331, 'BG_DIFFICULTY_4', 0.6, 1.99, 69, 69),
(13331, 'BG_DIFFICULTY_5', 0.6, 1.99, 74, 74),
(13331, 'BG_DIFFICULTY_6', 0.6, 1.99, 79, 79),
(13331, 'BG_DIFFICULTY_7', 0.6, 1.99, 84, 84),
(13331, 'BG_DIFFICULTY_8', 0.6, 0.07, 89, 89),
(13331, 'BG_DIFFICULTY_9', 3, 3.29, 90, 90),
(13332, 'BG_DIFFICULTY_0', 0.6, 1.99, 49, 49), -- Veteran Guardian
(13332, 'BG_DIFFICULTY_1', 0.6, 1.99, 54, 54),
(13332, 'BG_DIFFICULTY_2', 0.6, 1.99, 59, 59),
(13332, 'BG_DIFFICULTY_3', 0.6, 1.99, 64, 64),
(13332, 'BG_DIFFICULTY_4', 0.6, 1.99, 69, 69),
(13332, 'BG_DIFFICULTY_5', 0.6, 1.99, 74, 74),
(13332, 'BG_DIFFICULTY_6', 0.6, 1.99, 79, 79),
(13332, 'BG_DIFFICULTY_7', 0.6, 1.99, 84, 84),
(13332, 'BG_DIFFICULTY_8', 0.6, 0.07, 89, 89),
(13332, 'BG_DIFFICULTY_9', 3, 3.29, 90, 90),
(13358, 'BG_DIFFICULTY_0', 1.5, 1.35, 48, 49), -- Stormpike Bowman
(13358, 'BG_DIFFICULTY_1', 1.5, 1.35, 53, 54),
(13358, 'BG_DIFFICULTY_2', 1.5, 1.35, 58, 59),
(13358, 'BG_DIFFICULTY_3', 1.5, 1.35, 63, 64),
(13358, 'BG_DIFFICULTY_4', 1.5, 1.35, 68, 69),
(13358, 'BG_DIFFICULTY_5', 1.5, 1.35, 73, 74),
(13358, 'BG_DIFFICULTY_6', 1.5, 1.35, 78, 79),
(13358, 'BG_DIFFICULTY_7', 1.5, 1.35, 83, 84),
(13358, 'BG_DIFFICULTY_8', 1.5, 0.06, 88, 89),
(13358, 'BG_DIFFICULTY_9', 4, 3.29, 89, 90),
(13359, 'BG_DIFFICULTY_0', 1.5, 1.28, 48, 49), -- Frostwolf Bowman
(13359, 'BG_DIFFICULTY_1', 1.5, 1.28, 53, 54),
(13359, 'BG_DIFFICULTY_2', 1.5, 1.28, 58, 59),
(13359, 'BG_DIFFICULTY_3', 1.5, 1.28, 63, 64),
(13359, 'BG_DIFFICULTY_4', 1.5, 1.28, 68, 69),
(13359, 'BG_DIFFICULTY_5', 1.5, 1.28, 73, 74),
(13359, 'BG_DIFFICULTY_6', 1.5, 1.28, 78, 79),
(13359, 'BG_DIFFICULTY_7', 1.5, 1.28, 83, 84),
(13359, 'BG_DIFFICULTY_8', 1.5, 0.06, 88, 89),
(13359, 'BG_DIFFICULTY_9', 4, 3.29, 89, 90),
(13421, 'BG_DIFFICULTY_0', 0.7, 1.94, 50, 50), -- Champion Guardian
(13421, 'BG_DIFFICULTY_1', 0.7, 1.94, 55, 55),
(13421, 'BG_DIFFICULTY_2', 0.7, 1.94, 60, 60),
(13421, 'BG_DIFFICULTY_3', 0.7, 1.94, 65, 65),
(13421, 'BG_DIFFICULTY_4', 0.7, 1.94, 70, 70),
(13421, 'BG_DIFFICULTY_5', 0.7, 1.94, 75, 75),
(13421, 'BG_DIFFICULTY_6', 0.7, 1.94, 80, 80),
(13421, 'BG_DIFFICULTY_7', 0.7, 1.94, 85, 85),
(13421, 'BG_DIFFICULTY_8', 0.7, 0.05, 90, 90),
(13421, 'BG_DIFFICULTY_9', 3.5, 2.57, 91, 91),
(13422, 'BG_DIFFICULTY_0', 0.7, 1.94, 50, 50), -- Champion Defender
(13422, 'BG_DIFFICULTY_1', 0.7, 1.94, 55, 55),
(13422, 'BG_DIFFICULTY_2', 0.7, 1.94, 60, 60),
(13422, 'BG_DIFFICULTY_3', 0.7, 1.94, 65, 65),
(13422, 'BG_DIFFICULTY_4', 0.7, 1.94, 70, 70),
(13422, 'BG_DIFFICULTY_5', 0.7, 1.94, 75, 75),
(13422, 'BG_DIFFICULTY_6', 0.7, 1.94, 80, 80),
(13422, 'BG_DIFFICULTY_7', 0.7, 1.94, 85, 85),
(13422, 'BG_DIFFICULTY_8', 0.7, 0.05, 90, 90),
(13422, 'BG_DIFFICULTY_9', 3.5, 2.57, 91, 91),
(14284, 'BG_DIFFICULTY_0', 6, 9.28, 50, 50), -- Stormpike Battleguard
(14284, 'BG_DIFFICULTY_1', 6, 9.28, 55, 55),
(14284, 'BG_DIFFICULTY_2', 6, 9.28, 60, 60),
(14284, 'BG_DIFFICULTY_3', 6, 9.28, 65, 65),
(14284, 'BG_DIFFICULTY_4', 6, 9.28, 70, 70),
(14284, 'BG_DIFFICULTY_5', 6, 9.28, 75, 75),
(14284, 'BG_DIFFICULTY_6', 6, 9.28, 80, 80),
(14284, 'BG_DIFFICULTY_7', 6, 9.28, 85, 85),
(14284, 'BG_DIFFICULTY_8', 6, 0.29, 90, 90),
(14284, 'BG_DIFFICULTY_9', 6, 0.23, 91, 91),
(14285, 'BG_DIFFICULTY_0', 6, 9.28, 50, 50), -- Frostwolf Battleguard
(14285, 'BG_DIFFICULTY_1', 6, 9.28, 55, 55),
(14285, 'BG_DIFFICULTY_2', 6, 9.28, 60, 60),
(14285, 'BG_DIFFICULTY_3', 6, 9.28, 65, 65),
(14285, 'BG_DIFFICULTY_4', 6, 9.28, 70, 70),
(14285, 'BG_DIFFICULTY_5', 6, 9.28, 75, 75),
(14285, 'BG_DIFFICULTY_6', 6, 9.28, 80, 80),
(14285, 'BG_DIFFICULTY_7', 6, 9.28, 85, 85),
(14285, 'BG_DIFFICULTY_8', 6, 0.29, 90, 90),
(14285, 'BG_DIFFICULTY_9', 6, 0.23, 91, 91),
(14762, 'BG_DIFFICULTY_0', 25, 20.4, 49, 49), -- Dun Baldar North Marshal
(14762, 'BG_DIFFICULTY_1', 25, 20.4, 54, 54),
(14762, 'BG_DIFFICULTY_2', 25, 20.4, 59, 59),
(14762, 'BG_DIFFICULTY_3', 25, 20.4, 64, 64),
(14762, 'BG_DIFFICULTY_4', 25, 20.4, 69, 69),
(14762, 'BG_DIFFICULTY_5', 25, 20.4, 74, 74),
(14762, 'BG_DIFFICULTY_6', 25, 20.4, 79, 79),
(14762, 'BG_DIFFICULTY_7', 25, 20.4, 84, 84),
(14762, 'BG_DIFFICULTY_8', 25, 0.71, 89, 89),
(14762, 'BG_DIFFICULTY_9', 50, 4.42, 90, 90),
(14763, 'BG_DIFFICULTY_0', 25, 20.4, 49, 49), -- Dun Baldar South Marshal
(14763, 'BG_DIFFICULTY_1', 25, 20.4, 54, 54),
(14763, 'BG_DIFFICULTY_2', 25, 20.4, 59, 59),
(14763, 'BG_DIFFICULTY_3', 25, 20.4, 64, 64),
(14763, 'BG_DIFFICULTY_4', 25, 20.4, 69, 69),
(14763, 'BG_DIFFICULTY_5', 25, 20.4, 74, 74),
(14763, 'BG_DIFFICULTY_6', 25, 20.4, 79, 79),
(14763, 'BG_DIFFICULTY_7', 25, 20.4, 84, 84),
(14763, 'BG_DIFFICULTY_8', 25, 0.71, 89, 89),
(14763, 'BG_DIFFICULTY_9', 50, 4.42, 90, 90),
(14764, 'BG_DIFFICULTY_0', 25, 20.4, 49, 49), -- Icewing Marshal
(14764, 'BG_DIFFICULTY_1', 25, 20.4, 54, 54),
(14764, 'BG_DIFFICULTY_2', 25, 20.4, 59, 59),
(14764, 'BG_DIFFICULTY_3', 25, 20.4, 64, 64),
(14764, 'BG_DIFFICULTY_4', 25, 20.4, 69, 69),
(14764, 'BG_DIFFICULTY_5', 25, 20.4, 74, 74),
(14764, 'BG_DIFFICULTY_6', 25, 20.4, 79, 79),
(14764, 'BG_DIFFICULTY_7', 25, 20.4, 84, 84),
(14764, 'BG_DIFFICULTY_8', 25, 0.71, 89, 89),
(14764, 'BG_DIFFICULTY_9', 50, 4.42, 90, 90),
(14765, 'BG_DIFFICULTY_0', 25, 20.4, 49, 49), -- Stonehearth Marshal
(14765, 'BG_DIFFICULTY_1', 25, 20.4, 54, 54),
(14765, 'BG_DIFFICULTY_2', 25, 20.4, 59, 59),
(14765, 'BG_DIFFICULTY_3', 25, 20.4, 64, 64),
(14765, 'BG_DIFFICULTY_4', 25, 20.4, 69, 69),
(14765, 'BG_DIFFICULTY_5', 25, 20.4, 74, 74),
(14765, 'BG_DIFFICULTY_6', 25, 20.4, 79, 79),
(14765, 'BG_DIFFICULTY_7', 25, 20.4, 84, 84),
(14765, 'BG_DIFFICULTY_8', 25, 0.71, 89, 89),
(14765, 'BG_DIFFICULTY_9', 50, 4.42, 90, 90),
(14772, 'BG_DIFFICULTY_0', 25, 20.4, 49, 49), -- East Frostwolf Warmaster
(14772, 'BG_DIFFICULTY_1', 25, 20.4, 54, 54),
(14772, 'BG_DIFFICULTY_2', 25, 20.4, 59, 59),
(14772, 'BG_DIFFICULTY_3', 25, 20.4, 64, 64),
(14772, 'BG_DIFFICULTY_4', 25, 20.4, 69, 69),
(14772, 'BG_DIFFICULTY_5', 25, 20.4, 74, 74),
(14772, 'BG_DIFFICULTY_6', 25, 20.4, 79, 79),
(14772, 'BG_DIFFICULTY_7', 25, 20.4, 84, 84),
(14772, 'BG_DIFFICULTY_8', 25, 0.71, 89, 89),
(14772, 'BG_DIFFICULTY_9', 50, 4.42, 90, 90),
(14773, 'BG_DIFFICULTY_0', 25, 20.4, 49, 49), -- Iceblood Warmaster
(14773, 'BG_DIFFICULTY_1', 25, 20.4, 54, 54),
(14773, 'BG_DIFFICULTY_2', 25, 20.4, 59, 59),
(14773, 'BG_DIFFICULTY_3', 25, 20.4, 64, 64),
(14773, 'BG_DIFFICULTY_4', 25, 20.4, 69, 69),
(14773, 'BG_DIFFICULTY_5', 25, 20.4, 74, 74),
(14773, 'BG_DIFFICULTY_6', 25, 20.4, 79, 79),
(14773, 'BG_DIFFICULTY_7', 25, 20.4, 84, 84),
(14773, 'BG_DIFFICULTY_8', 25, 0.71, 89, 89),
(14773, 'BG_DIFFICULTY_9', 50, 4.42, 90, 90),
(14776, 'BG_DIFFICULTY_0', 25, 20.4, 49, 49), -- Tower Point Warmaster
(14776, 'BG_DIFFICULTY_1', 25, 20.4, 54, 54),
(14776, 'BG_DIFFICULTY_2', 25, 20.4, 59, 59),
(14776, 'BG_DIFFICULTY_3', 25, 20.4, 64, 64),
(14776, 'BG_DIFFICULTY_4', 25, 20.4, 69, 69),
(14776, 'BG_DIFFICULTY_5', 25, 20.4, 74, 74),
(14776, 'BG_DIFFICULTY_6', 25, 20.4, 79, 79),
(14776, 'BG_DIFFICULTY_7', 25, 20.4, 84, 84),
(14776, 'BG_DIFFICULTY_8', 25, 0.71, 89, 89),
(14776, 'BG_DIFFICULTY_9', 50, 4.42, 90, 90),
(14777, 'BG_DIFFICULTY_0', 25, 20.4, 49, 49), -- West Frostwolf Warmaster
(14777, 'BG_DIFFICULTY_1', 25, 20.4, 54, 54),
(14777, 'BG_DIFFICULTY_2', 25, 20.4, 59, 59),
(14777, 'BG_DIFFICULTY_3', 25, 20.4, 64, 64),
(14777, 'BG_DIFFICULTY_4', 25, 20.4, 69, 69),
(14777, 'BG_DIFFICULTY_5', 25, 20.4, 74, 74),
(14777, 'BG_DIFFICULTY_6', 25, 20.4, 79, 79),
(14777, 'BG_DIFFICULTY_7', 25, 20.4, 84, 84),
(14777, 'BG_DIFFICULTY_8', 25, 0.71, 89, 89),
(14777, 'BG_DIFFICULTY_9', 50, 4.42, 90, 90);

-- 2. Patch 5.4.7: Vanndar, Balinda, Galvangar, Drek'Thar immune to disarm, daze and Death Grip.
--    Daze (1<<26) is already set (mask 617299803); add DISARM (mechanic 3 -> 0x4) and GRIP (mechanic 6 -> 0x20).
UPDATE `creature_template` SET `mechanic_immune_mask` = `mechanic_immune_mask` | 0x4 | 0x20 WHERE `entry` IN (11946,11947,11948,11949);
