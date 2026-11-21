-- Eastern Plaguelands, Honor and Strength (27449): Vex'tul's test at Light's Shield Tower was never built.
--
-- Before: three static Mossflayer Rogues 45579 (guids 173775-173777, 60 s respawn, the sniffed run-in paths of the summons)
-- stood 30 yd from Vex'tul 45574 and were the only way to the objective; nothing happened on accept; Vex'tul had no gossip.
--
-- What the player should see, with the source of each part:
--  * Talking to Vex'tul: "You do not need to fight me, <race>. I mean you no ill will." (sniffed menu 12041, npc_text 16871 =
--    broadcast 45709, already in mop but on no NPC; comment 2011-11-27 quotes it) with the quest.
--  * On accept he yells "Now, brothers! Attack the <race>!" (broadcast 45708; wiki note) and three Mossflayer Rogues appear
--    behind the player and attack him (comment 2010-12-03 [+13]: also when the quest is shared to someone far away; wiki).
--    Spell 85093 "Honor and Strength: Summon Rogues" (DBC): 45579 at the caster's back 5 yd, and 85094 / 85095 at back-left and
--    back-right 10 yd; SummonProperties 2912, 120 s. The quest's SourceSpellID makes the player cast it on accept and on a shared
--    accept (QuestHandler.cpp, triggered); the sniffed first waypoints of the static rogues fit exactly that geometry.
--  * If the rogues died without credit or are gone: "Can I take your test again, Vex'tul?" (broadcast 45710, a player line)
--    and three more come with the yell (comments 2011-02-03 [+2], 2011-05-16 [+2]).
--  * Hand-in: progress text in 2026_11_14_21. After hand-in he is gone from the tower (caravan staging: 80852 on guid 373695,
--    spell_area 85088 in 5448 from 27432 rewarded to 27449 rewarded; not in this file).
-- Ours:
--  * the rogues attack their summoner on summon (event 54 -> attack invoker): SummonProperties 2912 flag 0x1 ("attack
--    summoner" in later TrinityCore) is not implemented in this core, and by proximity alone a high-level player would not
--    be attacked;
--  * the retake option shows only while 27449 is incomplete (condition 9) and no living Mossflayer Rogue is within 50 yd of
--    the player (condition 29, negated), so waves cannot be stacked and a player who died always gets a way back;
--  * the yell on retake;
--  * no new wave while living Mossflayer Rogues are within 50 yd of the player (condition on 85093 itself, the same test as the
--    retake option): abandoning and accepting again, or a second accept nearby, no longer stacks waves. Rogues cannot be told
--    apart by summoner in a condition, so another player's live wave nearby also holds his back; once those are dead or gone
--    (120 s at most) the retake option brings his three.
--  * the yell comes only with a wave, once per wave: on accept it carries the same "no living rogue within 50 yd of the player"
--    condition as the summon (source 22 on the accept row; the AI's OnQuestAccept runs before QuestHandler casts the
--    SourceSpellID, so both see the same state), and on retake it is said before the cast (the option itself needs that
--    state). Several players accepting at once hear one yell, from the accept that brings the wave.
-- The template is shared with Vex'tul's caravan copies (staging 3128015 Eastwall, 3128020 Northpass; they show from 27449
-- rewarded): the accept row fires only for 27449 and the retake option needs 27449 incomplete, so neither does anything there;
-- they show the same greeting page. Vex'tul (faction 35) and the rogues (37) are neutral to each other: he neither helps nor
-- is attacked. Text group 0 is new (45574 had no creature_text).

-- The rogues come by the quest's source spell
DELETE FROM `quest_template_addon` WHERE `ID` = 27449;
INSERT INTO `quest_template_addon` (`ID`, `MaxLevel`, `AllowableClasses`, `SourceSpellID`, `PrevQuestID`, `NextQuestID`, `ExclusiveGroup`, `RequiredMinRepValue`, `RequiredMaxRepValue`, `ProvidedItemCount`, `SpecialFlags`, `ScriptName`) VALUES
(27449, 0, 0, 85093, 0, 0, 0, 0, 0, 0, 0, '');

-- The static copies of the summons and their run-in paths go
DELETE FROM `creature` WHERE `guid` IN (173775, 173776, 173777) AND `id` = 45579;
DELETE FROM `creature_addon` WHERE `guid` IN (173775, 173776, 173777);
DELETE FROM `waypoint_data` WHERE `id` IN (173775, 173776, 173777);

-- No new wave while living rogues are within 50 yd of the caster (accept, shared accept and retake all cast 85093)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 17 AND `SourceEntry` = 85093;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(17, 0, 85093, 0, 0, 29, 0, 45579, 50, 0, 1, 0, 0, '', 'Honor and Strength: Summon Rogues - not while living Mossflayer Rogues are within 50 yd');

-- Summoned rogues attack their summoner (row 0, Sinister Strike, stays as both TDBs have it)
DELETE FROM `smart_scripts` WHERE `entryorguid` = 45579 AND `source_type` = 0 AND `id` = 1;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(45579, 0, 1, 0, 54, 0, 100, 0, 0, 0, 0, 0, 0, 49, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Mossflayer Rogue - Just Summoned - Attack Summoner');

-- Vex'tul: greeting menu, retake option, yell
UPDATE `creature_template` SET `gossip_menu_id` = 12041, `npcflag` = 3, `AIName` = 'SmartAI' WHERE `entry` = 45574;

DELETE FROM `gossip_menu_option` WHERE `MenuID` = 12041;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcflag`, `ActionMenuID`) VALUES
(12041, 0, 0, 'Can I take your test again, Vex''tul?', 45710, 1, 1, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 12041;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 12041, 0, 0, 0, 9, 0, 27449, 0, 0, 0, 0, 0, '', 'Vex''tul - retake the test only while Honor and Strength is incomplete'),
(15, 12041, 0, 0, 0, 29, 0, 45579, 50, 0, 1, 0, 0, '', 'Vex''tul - retake the test only when no living Mossflayer Rogue is within 50 yd');

DELETE FROM `creature_text` WHERE `CreatureID` = 45574;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(45574, 0, 0, 'Now, brothers!  Attack the $r!', 14, 0, 100, 0, 0, 0, 0, 45708, 0, 'Vex''tul - Honor and Strength accepted or retaken');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceEntry` = 45574 AND `SourceId` = 0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(22, 1, 45574, 0, 0, 29, 0, 45579, 50, 0, 1, 0, 0, '', 'Vex''tul - yell on accept only when the wave comes (no living Mossflayer Rogue within 50 yd of the player)');

DELETE FROM `smart_scripts` WHERE `entryorguid` = 45574 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(45574, 0, 0, 0, 19, 0, 100, 0, 27449, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Vex''tul - On Quest Honor and Strength Accepted (wave comes) - Yell'),
(45574, 0, 1, 2, 62, 0, 100, 0, 12041, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Vex''tul - On Gossip Option 0 Selected - Close Gossip'),
(45574, 0, 2, 3, 61, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Vex''tul - Linked - Yell'),
(45574, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 85, 85093, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Vex''tul - Linked - Invoker Casts Honor and Strength: Summon Rogues (triggered)');
