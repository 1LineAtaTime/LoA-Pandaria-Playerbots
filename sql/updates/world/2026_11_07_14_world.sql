-- Hill Fawn 48187, A Fighting Chance 28495: blessed fawns fight, explode, and give one credit.
-- Sourced:
-- - id 13, Duskingdawn's Blessing 90685 at 40% HP: TDB 4.4.2 and 1210 (mop dropped it; Gore and Fawnplode on death were already here).
-- - That fawns fight and explode: Wowhead comments on quest 28495 (posted after Cataclysm launched): "killing machines", "put a fight against
--   anything near them", "or yourself", "If there's no one to fight, they'll blow up", "they can explode when they are killed". The wiki has
--   saving enrage them and a second use explode them.
-- - 20 s: the duration of 90685 in the 5.4.8 DBC, and mop's old despawn timer.
-- Ours (no source has this script):
-- - Aggressive instead of Defensive, and the fawn attacks whoever blessed it: at level 10-11 its aggro radius against level 21+ is too small
--   for it to come after the caster by itself.
-- - The explosion is the fawn's death: at 20 s, when the fight ends, or on a second wand use it goes to phase 3 and kills itself, id 12 casts
--   Fawnplode as on any death, and id 22 removes the corpse after 0.5 s. A forced despawn would keep it alive and fighting, invisible, for 5 s.
--   The fawn has no loot and gives no kill credit, so nothing is lost with the corpse.
-- - Evade off while blessed. EnterEvadeMode strips the auras and calls Reset, which puts the fawn back in phase 1 with faction 14 still on,
--   and Creature::Update does not run the AI during evade, so it walked home and could be blessed for another credit. With evade off the
--   call only fires the event. Reset turns evade back on (mEvadeDisabled survives a respawn).
-- - No blessing while a fawn evades (condition on id 5): its AI is not running, and JustReachedHome would undo the blessed state and allow a
--   second credit. Reset also restores faction 1999, so no path leaves an unblessed fawn hostile.
-- - Condition on Fawnplode: players only. It is TARGET_UNIT_SRC_AREA_ENTRY, 12 yd, with no condition in mop or either TDB, so it took every
--   unit in range: the fawn field by chain (a kill explodes too, and the 40% Blessing fires on the damage), town NPCs if a fawn was chased
--   there, and the fawn itself. No source says whom retail's hit except a player ("It killed my horde ironman toon"). Pets are not hit either.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=90686;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceEntry`=48187 AND `SourceId`=0 AND `SourceGroup`=6;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,3,90686,0,0,31,0,4,0,0,0,0,0,'','Fawnplode - Only hit players'),
(22,6,48187,0,0,21,1,4194304,0,0,1,0,0,'','Hill Fawn - Blessing only takes while the fawn is not evading (UNIT_STATE_EVADE)');

DELETE FROM `smart_scripts` WHERE `entryorguid`=48187 AND `source_type`=0 AND `id` IN (2,3,4,9,10,13,14,15,16,17,18,19,20,21,22,23,24);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(48187,0,2,19,25,0,100,0,0,0,0,0,0,22,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hill Fawn - On Reset - Set Phase 1 (unblessed)'),
(48187,0,19,23,61,0,100,0,0,0,0,0,0,117,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hill Fawn - Linked - Enable Evade'),
(48187,0,23,0,61,0,100,0,0,0,0,0,0,2,1999,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hill Fawn - Linked - Set Faction 1999'),
(48187,0,3,4,8,2,100,0,90679,0,0,0,0,22,3,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hill Fawn - On Spellhit Duskingdawn''s Blessing (Phase 2, blessed) - Set Phase 3 (exploding)'),
(48187,0,4,0,61,0,100,0,0,0,0,0,0,37,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hill Fawn - Linked - Die (Fawnplode)'),
(48187,0,9,10,61,0,100,0,0,0,0,0,0,8,2,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hill Fawn - Linked - Set React State Aggressive'),
(48187,0,10,24,61,0,100,0,0,0,0,0,0,117,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hill Fawn - Linked - Disable Evade'),
(48187,0,24,0,61,0,100,0,0,0,0,0,0,49,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Hill Fawn - Linked - Attack the Blesser'),
(48187,0,13,0,2,1,100,1,0,40,0,0,0,11,90685,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hill Fawn - At 40% HP (Phase 1) - Cast Duskingdawn''s Blessing'),
(48187,0,14,15,60,2,100,1,20000,20000,0,0,0,22,3,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hill Fawn - 20 sec after the blessing (Phase 2) - Set Phase 3 (exploding)'),
(48187,0,15,0,61,0,100,0,0,0,0,0,0,37,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hill Fawn - Linked - Die (Fawnplode)'),
(48187,0,17,18,7,2,100,1,0,0,0,0,0,22,3,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hill Fawn - On Evade (Phase 2, nobody left to fight) - Set Phase 3 (exploding)'),
(48187,0,18,0,61,0,100,0,0,0,0,0,0,37,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hill Fawn - Linked - Die (Fawnplode)'),
(48187,0,22,0,6,4,100,0,0,0,0,0,0,41,500,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hill Fawn - On Death (Phase 3, exploded) - Remove Corpse after 0.5 sec');
