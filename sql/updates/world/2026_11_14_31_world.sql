-- Eastern Plaguelands, Frederick's Fish Fancy (27450) at Light's Shield Tower: Puffer Breath, and the Argent Parachutes.
--
-- 1. Taking a Plague Puffer 45650 from the lake bottom gave no breath. What the player should see: "Puffer Breath", 20 s of
--    water breathing, each time he picks up a puffer, so he can stay down by taking puffers between the others. Sourced:
--    comments 2010-11-25 [+14] and later, wiki notes; spell 85354 "Puffer Breath" (DBC: aura WATER_BREATHING, 20 s, self),
--    in this zone's id block (91722 has the same effect elsewhere). The pickup itself is LoA's own spell-click mechanism
--    (npc_spellclick_spells -> 86328, SmartAI event 73 adds the item and despawns the critter) and stays; ours: the breath
--    is cast by the clicking player, triggered, between the item and the despawn.
-- 2. The box of Argent Parachutes 205873 beside Frederick (and at the other three towers: Crown Guard, Eastwall, Northpass)
--    casts 85882 "Parachute": "A portable parachute, automatically activates when the caster begins falling". 85882 gives
--    5 s of feather fall and a 200 ms periodic dummy naming 85883 "Parachute" (10 s feather fall, ends on landing), but
--    nothing scripted the periodic part: a player who jumped within those 5 s had feather fall only until they ran out, so a
--    long fall ended in free fall. Now the jump turns 85882 into 85883, which lasts until he lands. (Jumping more than 5 s
--    after using the box still gets nothing, as the spell's duration has it.) Sourced: the spells (DBC) and comments
--    2011-10-30 [+6] ("the parachute often shows only when you jump off"), 2014-07-24 [+3]. 45472 (the Wintergrasp/gnome
--    parachute) has the same shape and the script spell_gen_parachute; it now reads the buff from effect 0's base points
--    (same behaviour for 45472, C++ in this commit) and serves 85882 too.
--    Port: part 2 is left out here: this tree's spell_gen_parachute still handles 45472 only, so 85882 is not bound to it.

DELETE FROM `smart_scripts` WHERE `entryorguid` = 45650 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(45650, 0, 0, 1, 73, 0, 100, 0, 0, 0, 0, 0, 0, 56, 61292, 1, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Plague Puffer - On Spellclick - Add Item Plague Puffer'),
(45650, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 0, 85, 85354, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Plague Puffer - Linked - Invoker Casts Puffer Breath (triggered)'),
(45650, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Plague Puffer - Linked - Despawn');
