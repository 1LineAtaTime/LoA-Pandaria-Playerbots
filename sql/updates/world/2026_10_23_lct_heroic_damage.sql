-- #680 Lost City of the Tol'vir (map 755): heroic DamageModifier rows.
-- DRAFT - not applied. The PM applies it to the test `world` DB with a backup. creature_template_difficulty is read at startup
-- (ObjectMgr::LoadCreatureDifficultyModifiers), so it needs a worldserver restart. No build needed.
--
-- The issue suspected the NORMAL rows (102-125). The data says the slip is in the HEROIC rows (test world DB, 2026-10-23):
--  - Creature::UpdateDamagePhysical (StatSystem.cpp:1251-1253) multiplies melee damage by the difficulty row's DamageModifier.
--  - Every other Cata 5-man in this DB (maps 643 644 645 657 670 725) has heroic bosses at 125, at or above normal (95-121).
--    Their trash keeps heroic = normal (median ratio 1.0).
--  - LCT heroic is 0.07-0.13x its own normal for the bosses (Husam 125 -> 13, Barim 102.1 -> 7.5) and 0.14-0.37x for trash/adds.
--    Siamat heroic (13) is 1/10 of normal.
--  - The heroic values (7.5 / 13 / 1) are the MoP 5.4.8 creature_difficulty.damage_mod. There they scale explicit
--    per-difficulty mindmg/maxdmg columns (Husam heroic 1111-1431). This table has no such columns, so they became a 10x
--    weaker multiplier on the class-level base damage.
--  - w104 heroic killed all four bosses with 0 bot dead ticks, which fits.
-- Change: heroic bosses -> 125 (the value of every other Cata heroic boss here). Heroic trash/adds below their normal value
-- -> the normal value. Rows where heroic already equals normal are left alone (43658, 44261, 44898, 45063, 45096).
-- Health is untouched. LCT heroic health_mod is 1.1-1.8x normal, inside the 1.1-2.9x range of the other Cata 5-mans, except
-- Siamat (heroic 23.15 < normal 25) - flagged in #680, not changed here.
-- Undo (the before values):
--   UPDATE creature_template_difficulty SET DamageModifier=7.5  WHERE difficulty='DUNGEON_HEROIC' AND id IN (43612,43927,45259,45268,45269);
--   UPDATE creature_template_difficulty SET DamageModifier=13   WHERE difficulty='DUNGEON_HEROIC' AND id IN (43614,44577,44819,44896,44922,44924,44926,44932,44976,44977,44980,44981,44982,45062,45097,45122);
--   UPDATE creature_template_difficulty SET DamageModifier=1    WHERE difficulty='DUNGEON_HEROIC' AND id IN (43934,49219);

-- bosses: General Husam, Lockmaw, High Prophet Barim, Siamat (Augh 49045 has no heroic row; heroic falls back to his normal 125)
UPDATE `creature_template_difficulty` SET `DamageModifier`=125  WHERE `difficulty`='DUNGEON_HEROIC' AND `id` IN (44577, 43614, 43612, 44819);
-- encounter adds: Harbinger of Darkness, Soul Fragment, Burning Soul, Servants of Siamat
UPDATE `creature_template_difficulty` SET `DamageModifier`=36.1 WHERE `difficulty`='DUNGEON_HEROIC' AND `id`=43927;
UPDATE `creature_template_difficulty` SET `DamageModifier`=7    WHERE `difficulty`='DUNGEON_HEROIC' AND `id` IN (43934, 49219);
UPDATE `creature_template_difficulty` SET `DamageModifier`=35   WHERE `difficulty`='DUNGEON_HEROIC' AND `id` IN (45259, 45268, 45269);
-- trash: Pygmy Brute; Oathsworn Axemaster/Myrmidon/Wanderer/Pathfinder/Skinner/Tamer/Captain; Neferset Plaguebringer/Torturer/
-- Theurgist/Darkcaster, Oathsworn Scorpid Keeper
UPDATE `creature_template_difficulty` SET `DamageModifier`=48   WHERE `difficulty`='DUNGEON_HEROIC' AND `id`=44896;
UPDATE `creature_template_difficulty` SET `DamageModifier`=35   WHERE `difficulty`='DUNGEON_HEROIC' AND `id` IN (44922, 44924, 44926, 44932, 44981, 45097, 45122);
UPDATE `creature_template_difficulty` SET `DamageModifier`=37.7 WHERE `difficulty`='DUNGEON_HEROIC' AND `id` IN (44976, 44977, 44980, 44982, 45062);
