-- Ragefire Chasm: Ground Runes (61630) is the trigger under the rune on the floor (aura 119627). Retail gives its infernal
-- placeholder model 12817 probability 0 and always uses 31687 (TDB 1210.26091); ours weighted both 1, so it often spawned as a
-- visible infernal.
UPDATE `creature_template_model` SET `Probability` = 0 WHERE `CreatureID` = 61630 AND `CreatureDisplayID` = 12817;
UPDATE `creature_template_model` SET `Probability` = 1 WHERE `CreatureID` = 61630 AND `CreatureDisplayID` = 31687;
