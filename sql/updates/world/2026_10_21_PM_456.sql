-- Refs #456: Volcanoth's On Death cast of 73193 (Volcanoth Dead Explosion) is not triggered (cast flags 0), so a dead caster can never
-- cast it - on this core and under AC's rule (AC Spell.cpp:5853 lets only triggered casts through for a dead caster). Mark it
-- SMARTCAST_TRIGGERED (0x02), the way death casts are written elsewhere.
-- Undo: UPDATE smart_scripts SET action_param2=0 WHERE entryorguid=38855 AND source_type=0 AND id=203;
UPDATE `smart_scripts` SET `action_param2`=2 WHERE `entryorguid`=38855 AND `source_type`=0 AND `id`=203 AND `action_type`=11 AND `action_param1`=73193;
