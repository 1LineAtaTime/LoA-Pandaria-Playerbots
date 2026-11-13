-- Eastern Plaguelands, Righteous Indignation (27479): Mossflayer Eyes drop in pairs, and no Infected Mossflayer stands on
-- the Shallow Graves before anyone steps on them.
-- Sourced: the pair from every living troll ("2 eyes from each mob", "these always drop in pairs": Wowhead comments
--   2010-12-09 [+29], 2012-01-23, item 61313 2010-12-08 [+7]); one or two, sometimes none, from the risen undead (reply [+9],
--   2011-11-13 [+4]); no 12261 spawned in either TDB.
-- Ours: 100 % for both (the less grindy value: no source gives a rate), 1-2 for the undead ("one or two").
-- The graves raise their own: Shallow Grave 177677 (trap, radius 8) casts 19368 at a player within 8 yd, which summons
-- 12261 for 300 s (GameObject::Update -> CastSpell -> Spell::EffectSummonType, wild branch, TEMPSUMMON_TIMED_DESPAWN),
-- so the 19 static copies (each within 0-9 yd of a grave) only doubled them.
UPDATE `creature_loot_template` SET `ChanceOrQuestChance`=-100,`mincountOrRef`=2,`maxcount`=2 WHERE `item`=61313 AND `entry` IN (8560,8561,8562,10822);
UPDATE `creature_loot_template` SET `ChanceOrQuestChance`=-100,`mincountOrRef`=1,`maxcount`=2 WHERE `item`=61313 AND `entry`=12261;
DELETE FROM `creature_addon` WHERE `guid` IN (175251,175252,175255,175256,175257,175270,175276,175277,175278,175279,175280,175281,175282,175283,175284,175285,175289,175297,175298);
DELETE FROM `creature` WHERE `id`=12261 AND `map`=0 AND `guid` IN (175251,175252,175255,175256,175257,175270,175276,175277,175278,175279,175280,175281,175282,175283,175284,175285,175289,175297,175298);
