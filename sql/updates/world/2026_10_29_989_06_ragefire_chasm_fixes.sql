-- #989 (Refs #989): Ragefire Chasm: Lava/Flame Visual unit_flags, Ground Runes model
-- Source: DeltaTho playerbots-fork PR 15, file 2026_10_27_77 + 2026_10_27_78 (commit as in deltatho/playerbots-fork), ported as-is unless noted below.
-- Design check 2026-10-29: retail flags per TDB 1210.26091 as cited in the PR; Wowhead tooltip API lists 61560/61601 Lava and 61413 Flame Visual as non-NPC triggers (no map, category 10).
-- Idempotent: DELETE+INSERT / absolute UPDATEs.

-- Ragefire Chasm: the Lava triggers (61560, 61601) and the Flame Visual (61413) came over from the sniff with unit_flags 0, so the
-- rising lava on the path was a hostile, attackable level 93 elite that aggroed and killed parties walking past it.
-- Give them retail's flags (TDB 1210.26091): the Lava is not selectable and immune to players and NPCs, the Flame Visual is
-- not selectable; the Flame Visual also gets the immune flags so it cannot start a fight either.
UPDATE `creature_template` SET `unit_flags` = 33555200 WHERE `entry` IN (61560, 61601); -- UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_SELECTABLE
UPDATE `creature_template` SET `unit_flags` = 33555200 WHERE `entry` = 61413;

-- Ragefire Chasm: Ground Runes (61630) is the trigger under the rune on the floor (aura 119627). Retail gives its infernal
-- placeholder model 12817 probability 0 and always uses 31687 (TDB 1210.26091); ours weighted both 1, so it often spawned as a
-- visible infernal.
UPDATE `creature_template_model` SET `Probability` = 0 WHERE `CreatureID` = 61630 AND `CreatureDisplayID` = 12817;
UPDATE `creature_template_model` SET `Probability` = 1 WHERE `CreatureID` = 61630 AND `CreatureDisplayID` = 31687;
