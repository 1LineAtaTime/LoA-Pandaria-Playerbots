-- The Hinterlands: creatures that stood twice on one spot.
-- Sourced: the Cataclysm Classic TDB 4.4.2 spawns of each entry, taken by box (map 0, x -950..1050, y -4910..-1900; the retail TDB 1210
-- has the same rows there). A spawn goes when it and another spawn of the same entry map onto one TDB spawn, stand closer to each other
-- than twice their wander distance (5 yd when standing, at most 8 yd), both lie within 20 yd of that TDB spawn, and mop has more of the
-- entry than the TDB within 20 yd of it. The copy with a path stays, otherwise the one nearer the TDB spawn.
-- Ours: a pair is kept when the entry's count in the box would drop below the TDB's (Wildhammer Warrior, Wildhammer Shaman, Spawn of
-- Shadra, Spider): there the layout is shifted, not doubled. Vilebranch Shadow Hunter 192205 stands 1.5 yd from 192146 with no TDB
-- Shadow Hunter within 144 yd (19 in the box against the TDB's 18). The Midsummer spawns were imported twice on the exact same spots;
-- the 37xxxx copy goes and the 352xxx copy stays, as in Arathi.

-- Vilebranch Witch Doctor
DELETE FROM `creature` WHERE `id`=2640 AND `guid` IN (192108,192342,192343,192347,192372);
-- Vilebranch Headhunter
DELETE FROM `creature` WHERE `id`=2641 AND `guid` IN (192091,192099,192211,192238,192353);
-- Vilebranch Shadowcaster
DELETE FROM `creature` WHERE `id`=2642 AND `guid` IN (191943);
-- Vilebranch Berserker
DELETE FROM `creature` WHERE `id`=2643 AND `guid` IN (192368,192373);
-- Vilebranch Shadow Hunter
DELETE FROM `creature` WHERE `id`=2645 AND `guid` IN (192205);
-- Vilebranch Soul Eater
DELETE FROM `creature` WHERE `id`=2647 AND `guid` IN (192334);
-- Green Sludge
DELETE FROM `creature` WHERE `id`=2655 AND `guid` IN (192359,192363);
-- Jade Ooze
DELETE FROM `creature` WHERE `id`=2656 AND `guid` IN (192245);
-- Trained Razorbeak
DELETE FROM `creature` WHERE `id`=2657 AND `guid` IN (191224,191225,192063,192092);
-- Mangy Silvermane
DELETE FROM `creature` WHERE `id`=2923 AND `guid` IN (192083,192085);
-- Vilebranch Warrior
DELETE FROM `creature` WHERE `id`=4465 AND `guid` IN (191876,192095,192096,192240,192250);
-- Maggot
DELETE FROM `creature` WHERE `id`=16030 AND `guid` IN (191804);
-- Skittering Spiderling
DELETE FROM `creature` WHERE `id`=42689 AND `guid` IN (191875,191882);
-- Cockroach
DELETE FROM `creature` WHERE `id`=45439 AND `guid` IN (191878);
-- Midsummer Bonfire
DELETE FROM `creature` WHERE `id`=16592 AND `guid` IN (370007,370280);
-- Midsummer Celebrant
DELETE FROM `creature` WHERE `id`=16781 AND `guid` IN (369949,370009,370010,370011,370371,370372,370407,370408);
-- Fire Eater
DELETE FROM `creature` WHERE `id`=25962 AND `guid` IN (369948);
-- Flame Eater
DELETE FROM `creature` WHERE `id`=25994 AND `guid` IN (370204);
-- [DND] Midsummer Bonfire Faction Bunny - H
DELETE FROM `creature` WHERE `id`=26355 AND `guid` IN (370281);

-- Their addon and game event rows (only once the creature is gone).
DELETE a FROM `creature_addon` a LEFT JOIN `creature` c ON c.`guid`=a.`guid` WHERE a.`guid` IN (192334,369948,369949,370007,370009,370010,370011,370204,370280,370281,370371,370372,370407,370408) AND c.`guid` IS NULL;
DELETE g FROM `game_event_creature` g LEFT JOIN `creature` c ON c.`guid`=g.`guid` WHERE g.`guid` IN (369948,369949,370007,370009,370010,370011,370204,370280,370281,370371,370372,370407,370408) AND c.`guid` IS NULL;
