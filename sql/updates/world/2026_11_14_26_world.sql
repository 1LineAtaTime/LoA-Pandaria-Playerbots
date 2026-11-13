-- Eastern Plaguelands, What I Do Best (27371): the Death's Step Miscreations' Blight Bomb was never cast.
--
-- Death's Step Miscreation 45444, SmartAI row 0, cast Blight Bomb 83021 on death with target_type 0 (none), so the action had
-- no target and did nothing (ledger CAST-NO-TARGET). The row came in with the fork's Eastern Plaguelands abilities import
-- (MoP_Playerbots_Beta 2026_10_29_987, entry-wide DELETE + INSERT of source_type 0; no fork rows with ids 200+ on this entry).
--
-- What the player should see (ruling of the research gate, on the user's question list as item 12): a Miscreation brought
-- below 30 % health starts a 5 s "Blight Bomb" cast bar; interrupted, it fights on; if the cast completes it blows itself up and
-- hurts the enemies within 20 yd (the player, his pet, Tarenar's follower).
--  * Sourced: spell 83021 (DBC): cast time 5000 ms (SpellCastTimes 6), effect 0 SCHOOL_DAMAGE to the caster's enemies in
--    20 yd (targets 22 / 15), effect 1 INSTAKILL on the caster, interrupt flags 9, no CASTABLE_WHILE_DEAD attribute: shapes
--    that fit only a living caster. A 2014-04-12 [+3] comment describes a mind-controlled Miscreation blowing itself up on the
--    Putrifiers with it. No 2010-2014 comment mentions Miscreations exploding as they die.
--  * Ours: the 30 % threshold, once per life (event flag NOT_REPEATABLE, reset on evade and respawn by SmartScript::OnReset),
--    not triggered (cast bar, interruptible), and SMARTCAST_INTERRUPT_PREVIOUS: a Miscreation still casting Toxic Waste when it
--    reaches 30 % would otherwise fail the cast and use up the once-per-life event.
--  * Documented fallback: both TDBs cast it on death, triggered, on self (event 6, action_param2 2, target_type 1). If the
--    user rules for that, the row becomes
--      UPDATE `smart_scripts` SET `event_type`=6,`event_flags`=1,`event_param1`=0,`event_param2`=0,`action_param2`=2,`target_type`=1,`target_param1`=0
--      WHERE `entryorguid`=45444 AND `source_type`=0 AND `id`=0 AND `action_param1`=83021;
--  * Credit stays safe: a self-kill credits the tapper when players (and the follower, an owned guardian) did at least half
--    the damage (Unit::Kill), which they have at 30 %.

UPDATE `smart_scripts` SET `event_type` = 2, `event_flags` = 1, `event_param1` = 0, `event_param2` = 30, `event_param3` = 0, `event_param4` = 0, `event_param5` = 0,
  `action_param2` = 1, `action_param3` = 0, `target_type` = 1, `target_param1` = 0, `target_param2` = 0, `target_param3` = 0,
  `comment` = 'Death''s Step Miscreation - Below 30% Health - Cast Blight Bomb (once)'
  WHERE `entryorguid` = 45444 AND `source_type` = 0 AND `id` = 0 AND `action_param1` = 83021;
