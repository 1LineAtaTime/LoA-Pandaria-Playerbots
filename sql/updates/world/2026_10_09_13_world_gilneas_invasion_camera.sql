-- Refs #505, #439. 14320 "In Need of Ingredients": the reward cutscene of the Forsaken ships (cinematic 168) never
-- plays. SQL only, no build needed.
--
-- Intended (Wowpedia "In Need of Ingredients", MediaWiki API read 2026-10-09): "Completing the quest shows a cutscene
-- pointing down to the ships assaulting the beach."
--
-- Cause: the reward spell 69296 "Invasion Camera" (cast by the player on himself, Player::RewardQuest) = effect 0
-- PLAY_MUSIC 23676 + effect 1 ACTIVATE_OBJECT on TARGET_GAMEOBJECT_NEARBY_ENTRY (40, range 90 yd). The camera is GO
-- 197337 "Gilneas Invasion Camera" (type 13 CAMERA, Data1 = cinematic 168), guid 166866, 3 yd from the quest-ender
-- crate 196394. Two gaps stop it:
--   * no conditions row for 69296 effect 1, so Spell::SelectImplicitNearbyTargets selects no GO
--     (TARGET_CHECK_ENTRY without conditions, Spell.cpp:1079);
--   * the camera spawn is phaseMask 1 while the player is in phase 4096 at this point (phase_definitions 4714/6:
--     14222 rewarded, 14386 not), so even a conditioned search could not see it. The crate itself is 4096.
-- With both fixed: ACTIVATE_OBJECT -> SCRIPT_COMMAND_ACTIVATE_OBJECT -> GameObject::Use(player)
-- (MapScripts.cpp:627-647) -> GAMEOBJECT_TYPE_CAMERA -> SendCinematicStart(168) (GameObject.cpp:1512-1524).
-- The MoP 5.4.8 data has the same two gaps (camera phaseMask 1, no conditions, crate unscripted), so this goes beyond it;
-- 4.3.4 references do the same thing in code (CPP gilneas_chapter_2.cpp go_gilneas_invasion_camera
-- -> SendCinematicStart(168); mopcore go_mandragore -> SendCinematicStart(168)).
-- 14386 (69027) and 14466 (68954): nothing to fix. The CPP spell_gilneas_forcecast_cataclysm_1 exists because 4.3.4
-- has the quest giver cast the reward spell; here the player casts it on himself, so the default FORCE_CAST chain
-- 69027 -> 68709 -> 68707 (stun 4 s + SEND_EVENT 22394 + periodic 68953) and 68954 -> 68953 already runs. Neither
-- spell has a cinematic; the quake is the spell visual.
--
-- Backup (state before this file): gameobject 166866 phaseMask = 1; conditions for 13/2/69296: 0 rows.
--   mysqldump -h<host> -u<user> -p<password> world gameobject --where="guid=166866"
-- Rollback:
--   UPDATE `gameobject` SET `phaseMask` = 1 WHERE `guid` = 166866 AND `id` = 197337;
--   DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceGroup` = 2 AND `SourceEntry` = 69296;
-- Re-runnable: UPDATE + DELETE-then-INSERT.

-- The camera must share the Duskhaven phase (4096); keep phase 1 as well.
UPDATE `gameobject` SET `phaseMask` = 1 | 4096 WHERE `guid` = 166866 AND `id` = 197337;

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceGroup` = 2 AND `SourceEntry` = 69296;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 2, 69296, 0, 0, 31, 0, 5, 197337, 0, 0, 0, 0, '', 'Invasion Camera effect 1 - targets Gilneas Invasion Camera 197337');
