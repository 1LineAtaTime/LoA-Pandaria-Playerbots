-- Western Plaguelands, Lord Maldazzar 1848 (Malicia's Outpost): the Skeleton 6412 he raises on aggro (Summon Skeleton 17204,
-- SmartAI row 2) is level 38 like him; it was level 1.
--
--  * Why: 17204's SummonProperties 1761 (ally category, no type) take the plain-summon branch of Spell::EffectSummonType, which
--    gives the summon its master's faction but its own template's level, and the template had 1-1.
--  * Sourced: that the Skeleton has no level of its own. Wowhead lists it at "Level 24 - 53" (Classic), "1 - 73" (Wrath) and
--    "1 - 71" (MoP Classic), in Western Plaguelands among other zones; AzerothCore 3.3.5 has 20-21 (Maldazzar was 56 there), and
--    TDB 4.4.2 has 1-1 in an unverified row (VerifiedBuild 0), like the 4.3.4 data here. Lord Maldazzar is 38 in mop_world,
--    TCPP 4.3.4 and TDB 4.4.2.
--  * Ours: 38-38, the level of its only summoner in mop_world (17204 is cast by nobody else and no script summons 6412). The
--    other columns are left alone: health and mana modifiers 0.5 match AzerothCore and both TDBs, and class 1 has base stats at 38.
--  * Side effect: the one static Skeleton (guid 308569, Howling Fjord) goes from level 1 to 38 as well; neither fits that zone.

UPDATE `creature_template` SET `minlevel` = 38, `maxlevel` = 38 WHERE `entry` = 6412;
