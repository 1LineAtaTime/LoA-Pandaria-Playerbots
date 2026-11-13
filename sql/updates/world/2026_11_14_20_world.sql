-- Eastern Plaguelands, Just Encased (27368) at cocooned Gidwin Goldbraids (Terrorweb Tunnel, east mouth).
--
-- 1. The four nerubians Gidwin killed ("you can count the bodies ... I killed four of them", quest text) lie 5-11 yd from him
--    with the permanent feign-death aura 29266, which only sets the dead look and REACT_PASSIVE. Their spawns had unit_flags 0,
--    so they were passive, selectable, attackable bodies with the full loot table (Crypt Bile included), respawning every
--    300 s: free kills beside the quest giver.
--    Sourced: both TDBs carry IMMUNE_TO_PC | IMMUNE_TO_NPC | NOT_SELECTABLE (0x2000300) on these entries, sniffed from these
--    bodies; on the template it would make every living crypt fiend unattackable, so the flags go on the four spawns only
--    (ObjectMgr::ChooseCreatureFlags takes a non-zero spawn value over the template's, called from Creature::InitEntry).
-- 2. The reward spell Knockback 50 No Log 70966 is cast by the player on himself (Player::RewardQuest) and has a 35 yd
--    area-entry effect with no condition, so it threw every unit within 35 yd: cocooned Gidwin, the four bodies and any other
--    player. Sourced: the wiki ("he smacks you knocking you back") and the completion text name only the player. Ours: the
--    condition that limits it to the caster. 70966 is used by nothing else in mop_world (no SmartAI, quest or creature spell).

UPDATE `creature` SET `unit_flags` = 33555200 WHERE `guid` IN (212067, 212068, 212069, 212070) AND `id` IN (8555, 8556, 8557, 8558);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 70966;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 70966, 0, 0, 33, 0, 1, 0, 0, 0, 0, 0, '', 'Knockback 50 No Log (Just Encased reward) - only the player who casts it');
