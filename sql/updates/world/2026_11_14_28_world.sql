-- Eastern Plaguelands, Postponing the Inevitable (27420): the Plague Disseminators could be shot down for credit.
--
-- Plague Disseminator 45475 (12 spawns hovering over the Fungal Vale, faction 974, level 41) had unit_flags 0x200
-- (IMMUNE_TO_NPC) only, so a player could attack it: a ranged class killed three for the objective without a single Plague
-- Disseminator Control Rune, and a Disseminator that saw a player came down to melee him.
-- What the player should see: the floating constructs cannot be targeted for attack and ignore him; he stands under one and
-- uses the Overcharged Mote (with a rune in the bags), the beam runs, he is thrown back, the construct dies, one rune is
-- spent, 1/3 (the quest text: "aim them ... then overcharge"; comments 2010-11-26 [+28], 2013-01-22).
--  * Sourced: both TDBs have unit_flags 768 (IMMUNE_TO_PC | IMMUNE_TO_NPC) on 45475.
--  * The quest path still works with the flag: 84973 Overcharge picks the Disseminator as nearest-entry target and 84972 has
--    TARGET_UNIT_TARGET_ANY, which SpellInfo::CheckExplicitTarget does not test for attackability, and Spell::DoSpellHitOnUnit
--    rejects neither branch for an IMMUNE_TO_PC target. 45475's SmartAI (spell hit 84973 -> invoker casts 84972; spell hit
--    84972 -> die) is unchanged.

UPDATE `creature_template` SET `unit_flags` = 768 WHERE `entry` = 45475;
