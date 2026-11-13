-- Quest 24681 "They Have Allies, But So Do We": Launch Glaive (70735) and Glaive Barrage (70732) hit their
-- targets through TARGET_UNIT_DEST_AREA_ENEMY (spells 84874 / 70738 and the follow-ups).  The summoned
-- Darnassian Glaive Thrower (41599) carries faction 35 (friendly to everybody), so the Orc Raiders, Wolfmaw
-- Outriders and Orcish War Machines are never "enemies" of the vehicle: every volley resolved with zero
-- targets, no damage and no kill credit.  Give it the Darnassus faction, the same one the other
-- Darnassian Glaive Thrower (35150) already uses.
UPDATE `creature_template` SET `faction` = 80 WHERE `entry` = 41599 AND `faction` = 35;
