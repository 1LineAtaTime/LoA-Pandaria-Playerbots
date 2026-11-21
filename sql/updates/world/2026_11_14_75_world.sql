-- Eastern Plaguelands, Add 'em to the Pile (27539): a dying Cursed Mage 8524 puts Tainted Mind 16567 (-20 % intellect, 10 min)
-- on those around it.
--
--  * Before: its death row cast 16567 at its victim (target 2), but a dying creature has stopped combat before JustDied runs
--    (Unit::setDeathState calls CombatStop), so there was no victim and the cast was skipped: the curse never landed.
--  * Sourced: "Cursed Mages put Tainted Mind on you as they die, every time" (2011-04-03 [+7], 2011-01-19, 2011-10-13, an old
--    2006 comment "Every time"). TDB 4.4.2 casts it in combat every 25-28 s instead; the comments are followed. A second mage's
--    curse replaces the first (same spell, not periodic: it does not stack), as the [-2] 2006 comment says.
--  * How: 16567 is an area aura around its caster (enemies within 10 yd), so the mage casts it on itself; the row already casts
--    triggered, which a dead caster may do in this core. A ranged killer farther than 10 yd plus both combat reaches is not
--    cursed (ours: the comments are melee and pet stories).

UPDATE `smart_scripts` SET `target_type` = 1 WHERE `entryorguid` = 8524 AND `source_type` = 0 AND `id` = 1;
