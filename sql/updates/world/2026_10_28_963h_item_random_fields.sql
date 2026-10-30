-- #963 (2026-10-28): two MoP greens had both RandomProperty and RandomSuffix set, so Item::GenerateItemRandomPropertyId
-- refused both ("must have one from field =0") and the item dropped with no random enchantment. Their item-level-429
-- siblings use only RandomSuffix (Battered Saurscale *: RandomProperty 0, RandomSuffix 405; Corroded *: RandomSuffix 390),
-- so the stray RandomProperty is cleared. Same values in the reference 5.4.8 world DBs (inherited).
-- Idempotent. Undo: RandomProperty 827 on 94057, 4990 on 94099.
UPDATE `item_template` SET `RandomProperty` = 0 WHERE `entry` = 94057 AND `RandomSuffix` = 405;
UPDATE `item_template` SET `RandomProperty` = 0 WHERE `entry` = 94099 AND `RandomSuffix` = 390;
