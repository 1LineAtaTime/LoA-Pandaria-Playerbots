-- Hillsbrad Foothills, the Sludge Fields: Magistrate Burnside's Arcane Storm, his hover and his death.
--
-- 1. Aim Arcane Storm at the camera targets that float around him (TCPP). Burnside 47790 has the template auras Aegis of
-- Energy 89159 (5.4.8 DBC: a dummy aura on himself, visual 18932, the bubble) and Arcane Storm 89161 (a periodic trigger
-- aura, 1 s, of 89160). Arcane Storm 89160 is a dummy effect (visual 19037) with TARGET_DEST_CASTER +
-- TARGET_UNIT_SRC_AREA_ENTRY and radius 49 = 90 yd. An area-entry target with no condition takes every unit in range
-- (WorldObjectSpellTargetCheck has no condition list to test), so every second the storm's arcs went to the player, the
-- companions and the 28 creatures of the town within 90 yd, besides the 23 Generic Camera Targets 47943 spawned 13-40 yd
-- around him (the 4.3.4 sniff). The hits are dummy and visual only: no damage, no threat. TCPP has the condition
-- (13/1/89160 -> 47943, "Arcane Storm - Target Generic Camera Target"); neither TDB has one.
--
-- 2. His spawn goes back to the sniff (TCPP 110_hillsbrad_foothills.sql, TDB 442 guid 323287): respawn 120 s, no wandering
-- (mop: 60 s and 3 yd random). The sniffed z 62.85 is 8.1 yd above the terrain (54.77; no vmap surface there): he floats
-- in his bubble (warcraft.wiki.gg: "floating inside a bubble"). Before, the random movement put him on the ground.
--
-- 3. Ours (no source shows how retail handled his height in the fight or after his death): he hovers at the spawn point
-- out of combat, comes down to the ground when the fight starts so that melee players and the melee-only Lydon and Johnny
-- can reach him, and loses the bubble and the storm when he dies. Both auras are DEATH_PERSISTENT, so the corpse kept them.
-- - flags_extra 0x200 (CREATURE_FLAG_EXTRA_NO_MOVE_FLAGS_UPDATE, as on the Skitterweb Matriarch in Silverpine): otherwise
--   Creature::Update calls UpdateMovementFlags every tick he has moved (except under random or effect movement), and for a
--   creature without a Flight row that is SetFlying(false), i.e. gravity back on and anim tier ground. That flag is read
--   nowhere else. No creature_template_movement Flight row either: CanFly() would then be true for good.
-- - Reset (InitializeAI at spawn, JustAppeared at respawn, EnterEvadeMode, JustReachedHome): SET_FLY 1 = gravity off
--   (SmartAI::SetFly -> SetDisableGravity). Needed at the first spawn (the create state has gravity on); later it is a no-op,
--   because nothing turns gravity back on.
-- - Gravity stays off for the whole fight. IsFlying() counts DISABLE_GRAVITY, so CanFly() is true while it is off: the chase
--   still follows the navmesh on the ground (ChaseMovementGenerator only forces the destination), the trip home after an
--   evade takes PathGenerator's flying shortcut back up to the spawn z 62.85, and setDeathState calls MoveFall, so the corpse
--   always lands, also after a death in mid-jump.
-- - Aggro: combat movement off, anim tier ground, jump straight down to the terrain below (an effect movement in the
--   controlled slot; combat movement goes off first because the Fireball row's SMARTCAST_COMBAT_MOVE would otherwise call
--   SetCombatMove(false), whose MotionMaster::Clear would stop the jump in mid-air). On landing (MovementInform
--   EFFECT_MOTION_TYPE) combat movement is allowed again and EffectMovementGenerator::Finalize starts the chase; the Fireball
--   row then holds him in place while his target is in range, as before. The evade reloads the template addon (anim tier 3,
--   the two auras; Creature::LoadCreaturesAddon skips auras he still has), and so does arriving home.
-- - Death: remove 89161 and 89159.
-- His existing rows 0-2 (Fire Nova, Fireball, Inferno Shell) are kept as they are. Safe to re-run.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=89160;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,1,89160,0,0,31,0,3,47943,0,0,0,0,'','Arcane Storm - Target Generic Camera Target');

UPDATE `creature` SET `spawntimesecs`=120, `wander_distance`=0, `MovementType`=0 WHERE `guid`=176605 AND `id`=47790;

DELETE FROM `creature_template_movement` WHERE `CreatureId`=47790;
UPDATE `creature_template` SET `flags_extra`=`flags_extra`|0x200 WHERE `entry`=47790;

DELETE FROM `smart_scripts` WHERE `entryorguid`=47790 AND `source_type`=0 AND `id` BETWEEN 3 AND 11;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(47790,0,3,0,25,0,100,0,0,0,0,0,0,60,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Magistrate Burnside - On Reset - Disable Gravity (hover in the bubble)'),
(47790,0,4,0,4,0,100,0,0,0,0,0,0,21,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Magistrate Burnside - On Aggro - Disallow Combat Movement (keep the jump)'),
(47790,0,5,0,4,0,100,0,0,0,0,0,0,91,3,3,0,0,0,0,1,0,0,0,0,0,0,0,0,'Magistrate Burnside - On Aggro - Set Anim Tier Ground'),
(47790,0,6,0,4,0,100,0,0,0,0,0,0,97,10,1,0,0,0,0,1,0,0,0,0,-400.214,73.4757,54.7716,0,'Magistrate Burnside - On Aggro - Jump Down To The Ground'),
(47790,0,7,0,34,0,100,0,16,0,0,0,0,21,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Magistrate Burnside - On Landed - Allow Combat Movement'),
(47790,0,8,0,6,0,100,0,0,0,0,0,0,28,89161,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Magistrate Burnside - On Death - Remove Arcane Storm'),
(47790,0,9,0,6,0,100,0,0,0,0,0,0,28,89159,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Magistrate Burnside - On Death - Remove Aegis of Energy');
