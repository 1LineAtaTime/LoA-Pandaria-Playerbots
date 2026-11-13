-- Requires npc_admiral_nightwind_departure in the matching native build.
-- TrinityCore issue22248 supplies the sniffed Rut'theran destination;
-- this build18414 DBC has both bind(effect0) and teleport(effect1).
-- https://github.com/TrinityCore/TrinityCore/issues/22248
INSERT INTO spell_target_position
 (id,effIndex,target_map,target_position_x,target_position_y,target_position_z,target_orientation)
VALUES (78107,0,1,8343.86,1165.28,4.40044,0),
       (78107,1,1,8343.86,1165.28,4.40044,0)
ON DUPLICATE KEY UPDATE target_map=VALUES(target_map),
 target_position_x=VALUES(target_position_x),target_position_y=VALUES(target_position_y),
 target_position_z=VALUES(target_position_z),target_orientation=VALUES(target_orientation);
UPDATE creature_template SET ScriptName='npc_admiral_nightwind_departure'
 WHERE entry=36616 AND name='Admiral Nightwind';
UPDATE quest_template SET PrevQuestId=26706 WHERE ID=14434;

-- A persistent addon cannot hold a oneshot animation. Preserve supported ready
-- and casting poses using their actual persistent counterparts in Emotes.dbc.
UPDATE creature_template_addon SET emote=333 WHERE entry IN(16803,29941) AND emote=36;
UPDATE creature_template_addon SET emote=333 WHERE entry=25764 AND emote=45;
UPDATE creature_template_addon SET emote=193 WHERE entry IN(33382,33383,33384,33780) AND emote=50;
-- Beg and rude have no matching persistent state: leave their scripted/combat
-- animations in control instead of writing invalid oneshot IDs as a state.
UPDATE creature_template_addon SET emote=0 WHERE entry IN(11388,11391) AND emote=20;
UPDATE creature_template_addon SET emote=0 WHERE entry=19719 AND emote=14;

-- Linked smart actions must use LINK(61), not a second RANGE(9) event.
-- This restores the missing melee-weapon step rather than hiding its diagnostic.
UPDATE smart_scripts SET event_type=61 WHERE entryorguid=6004 AND source_type=0
 AND id=9 AND event_type=9 AND action_type=40 AND action_param1=1;
-- Despawn a summoned skeletal servant regardless of dungeon difficulty.
UPDATE smart_scripts SET event_flags=1 WHERE entryorguid=8477 AND source_type=0
 AND id=0 AND event_flags=3 AND action_type=41;
