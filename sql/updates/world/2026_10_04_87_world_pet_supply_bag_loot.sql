-- Epic #441 (alexkulya audit rows SQ027); staged 2026-09-23,
-- re-checked 2026-10-04 against the LoA base world dump + sql/updates and LoA SmartAI enums.
-- MoP battle-pet reward bags open empty (audit row SQ027).
--
-- Sack of Pet Supplies (89125), Pandaren Spirit Pet Supplies (93146-93149),
-- Fabled Pandaren Pet Supplies (94207) and Brawler's Pet Supplies (98095) have
-- no item_loot_template rows in LoA (verified: 0 rows for all 7 entries; no
-- spell_loot_template rows for their spells 143506-143512 either). Their on-use
-- spells are SPELL_EFFECT_BONUS_ROLL (189, verified in SpellEffect.dbc), which
-- LoA resolves through Spell::EffectBonusRoll -> BonusLoot on the item's own
-- item_loot_template; with no rows the bag is consumed for nothing.
--
-- Source: local patch
--   0030_battlepet_reward_bag_loot.sql
-- MUST be applied together with the Spell::EffectBonusRoll change of the same port unit (epic #441) (the
-- core change, scoped to spells 143506-143512), otherwise LoA's
-- BonusLoot hands out only ONE uniformly-random entry (or gold) per bag.
-- Adaptations for LoA:
--   * lootmode 1 -> 0: LoA `lootmode` is a difficulty SET and FillLoot uses
--     lootmode = 1 << map difficulty (LootMgr.cpp:684); 0 = "any difficulty",
--     the value used by 5175 of LoA's 5214 item_loot_template rows. With 1 the
--     bag would drop nothing when opened inside a dungeon/raid.
--   * All 36 referenced item ids verified present in LoA item_template.
-- Re-runnable: DELETE-then-INSERT.

DELETE FROM `item_loot_template` WHERE `entry` IN (89125,93146,93147,93148,93149,94207,98095);

-- Sack of Pet Supplies: one normal supply (group 1), independent rare rewards.
INSERT INTO `item_loot_template`
(`entry`,`item`,`ChanceOrQuestChance`,`lootmode`,`groupid`,`mincountOrRef`,`maxcount`) VALUES
(89125, 86143, 0,    0, 1, 2, 4), -- Battle Pet Bandage
(89125, 71153, 0,    0, 1, 2, 4), -- Magical Pet Biscuit
(89125, 43352, 0,    0, 1, 2, 4), -- Pet Grooming Kit
(89125, 43626, 0,    0, 1, 2, 4), -- Happy Pet Snack
(89125, 89906, 0,    0, 1, 2, 4), -- Magical Mini-Treat
(89125, 37431, 0,    0, 1, 1, 1), -- Fetch Ball
(89125, 89139, 0,    0, 1, 1, 1), -- Chain Pet Leash
(89125, 44820, 0,    0, 1, 1, 1), -- Red Ribbon Pet Leash
(89125, 37460, 0,    0, 1, 1, 1), -- Rope Pet Leash
(89125, 90048, 0,    0, 1, 1, 1), -- Exquisite Murloc Leash
(89125, 67356, 0,    0, 1, 2, 4), -- Hair Ball
(89125, 90043, 0,    0, 1, 1, 1), -- Rusty Pet Cage
(89125, 90044, 0,    0, 1, 1, 1), -- Tattered Collar
(89125, 90047, 0,    0, 1, 1, 1), -- Sack of Expired Pet Food
(89125, 90058, 0,    0, 1, 1, 1), -- Well-Loved Toy
(89125, 92665, 0.75, 0, 0, 1, 1),
(89125, 92675, 0.75, 0, 0, 1, 1),
(89125, 92676, 0.75, 0, 0, 1, 1),
(89125, 92677, 0.75, 0, 0, 1, 1),
(89125, 92678, 0.75, 0, 0, 1, 1),
(89125, 92679, 0.75, 0, 0, 1, 1),
(89125, 92680, 0.75, 0, 0, 1, 1),
(89125, 92681, 0.75, 0, 0, 1, 1),
(89125, 92682, 0.75, 0, 0, 1, 1),
(89125, 92683, 0.75, 0, 0, 1, 1),
(89125, 92741, 1.00, 0, 0, 1, 1),
(89125, 89587, 1.00, 0, 0, 1, 1);

-- Four Pandaren Spirit bags: one normal supply, family stones, matching spirit pet.
INSERT INTO `item_loot_template`
(`entry`,`item`,`ChanceOrQuestChance`,`lootmode`,`groupid`,`mincountOrRef`,`maxcount`)
SELECT b.entry, c.item, 0, 0, 1, c.mincount, c.maxcount
FROM
(
    SELECT 93146 AS entry UNION ALL SELECT 93147 UNION ALL
    SELECT 93148 UNION ALL SELECT 93149
) b
CROSS JOIN
(
    SELECT 86143 AS item, 2 AS mincount, 4 AS maxcount UNION ALL
    SELECT 71153, 2, 4 UNION ALL SELECT 43352, 2, 4 UNION ALL
    SELECT 43626, 2, 4 UNION ALL SELECT 89906, 2, 4 UNION ALL
    SELECT 89139, 1, 1 UNION ALL SELECT 90043, 1, 1 UNION ALL
    SELECT 90044, 1, 1 UNION ALL SELECT 90047, 1, 1 UNION ALL
    SELECT 90058, 1, 1
) c;

INSERT INTO `item_loot_template`
(`entry`,`item`,`ChanceOrQuestChance`,`lootmode`,`groupid`,`mincountOrRef`,`maxcount`)
SELECT b.entry, s.item, 0.50, 0, 0, 1, 1
FROM
(
    SELECT 93146 AS entry UNION ALL SELECT 93147 UNION ALL
    SELECT 93148 UNION ALL SELECT 93149
) b
CROSS JOIN
(
    SELECT 92665 AS item UNION ALL SELECT 92675 UNION ALL SELECT 92676 UNION ALL
    SELECT 92677 UNION ALL SELECT 92678 UNION ALL SELECT 92679 UNION ALL
    SELECT 92680 UNION ALL SELECT 92681 UNION ALL SELECT 92682 UNION ALL
    SELECT 92683
) s;

INSERT INTO `item_loot_template`
(`entry`,`item`,`ChanceOrQuestChance`,`lootmode`,`groupid`,`mincountOrRef`,`maxcount`) VALUES
(93146, 92798, 5.00, 0, 0, 1, 1), -- Pandaren Fire Spirit
(93147, 90173, 5.00, 0, 0, 1, 1), -- Pandaren Water Spirit
(93148, 92799, 5.00, 0, 0, 1, 1), -- Pandaren Air Spirit
(93149, 92800, 5.00, 0, 0, 1, 1); -- Pandaren Earth Spirit

-- Fabled Pandaren Pet Supplies: guaranteed Lesser Pet Treat, one normal supply,
-- panda pets, rare battle-stones.
INSERT INTO `item_loot_template`
(`entry`,`item`,`ChanceOrQuestChance`,`lootmode`,`groupid`,`mincountOrRef`,`maxcount`) VALUES
(94207, 98112, 100,  0, 0, 1, 1), -- Lesser Pet Treat
(94207, 86143, 0,    0, 1, 2, 4),
(94207, 71153, 0,    0, 1, 2, 4),
(94207, 43352, 0,    0, 1, 2, 4),
(94207, 43626, 0,    0, 1, 2, 4),
(94207, 89906, 0,    0, 1, 2, 4),
(94207, 89139, 0,    0, 1, 1, 1),
(94207, 90043, 0,    0, 1, 1, 1),
(94207, 90044, 0,    0, 1, 1, 1),
(94207, 90047, 0,    0, 1, 1, 1),
(94207, 90058, 0,    0, 1, 1, 1),
(94207, 94208, 7.50, 0, 0, 1, 1), -- Sunfur Panda
(94207, 94209, 7.50, 0, 0, 1, 1), -- Snowy Panda
(94207, 94210, 7.50, 0, 0, 1, 1), -- Mountain Panda
(94207, 92665, 1.00, 0, 0, 1, 1),
(94207, 92675, 1.00, 0, 0, 1, 1),
(94207, 92676, 1.00, 0, 0, 1, 1),
(94207, 92677, 1.00, 0, 0, 1, 1),
(94207, 92678, 1.00, 0, 0, 1, 1),
(94207, 92679, 1.00, 0, 0, 1, 1),
(94207, 92680, 1.00, 0, 0, 1, 1),
(94207, 92681, 1.00, 0, 0, 1, 1),
(94207, 92682, 1.00, 0, 0, 1, 1),
(94207, 92683, 1.00, 0, 0, 1, 1),
(94207, 92741, 1.00, 0, 0, 1, 1);

-- Brawler's Pet Supplies: guaranteed Pet Treat, one normal supply, better stone chance.
INSERT INTO `item_loot_template`
(`entry`,`item`,`ChanceOrQuestChance`,`lootmode`,`groupid`,`mincountOrRef`,`maxcount`) VALUES
(98095, 98114, 100,  0, 0, 1, 1), -- Pet Treat
(98095, 86143, 0,    0, 1, 2, 4),
(98095, 71153, 0,    0, 1, 2, 4),
(98095, 43352, 0,    0, 1, 2, 4),
(98095, 43626, 0,    0, 1, 2, 4),
(98095, 89906, 0,    0, 1, 2, 4),
(98095, 37431, 0,    0, 1, 1, 1),
(98095, 89139, 0,    0, 1, 1, 1),
(98095, 44820, 0,    0, 1, 1, 1),
(98095, 37460, 0,    0, 1, 1, 1),
(98095, 90048, 0,    0, 1, 1, 1),
(98095, 67356, 0,    0, 1, 2, 4),
(98095, 90043, 0,    0, 1, 1, 1),
(98095, 90044, 0,    0, 1, 1, 1),
(98095, 90047, 0,    0, 1, 1, 1),
(98095, 90058, 0,    0, 1, 1, 1),
(98095, 92665, 2.00, 0, 0, 1, 1),
(98095, 92675, 2.00, 0, 0, 1, 1),
(98095, 92676, 2.00, 0, 0, 1, 1),
(98095, 92677, 2.00, 0, 0, 1, 1),
(98095, 92678, 2.00, 0, 0, 1, 1),
(98095, 92679, 2.00, 0, 0, 1, 1),
(98095, 92680, 2.00, 0, 0, 1, 1),
(98095, 92681, 2.00, 0, 0, 1, 1),
(98095, 92682, 2.00, 0, 0, 1, 1),
(98095, 92683, 2.00, 0, 0, 1, 1),
(98095, 92741, 1.00, 0, 0, 1, 1);
