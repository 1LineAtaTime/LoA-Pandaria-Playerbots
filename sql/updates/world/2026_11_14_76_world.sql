-- Eastern Plaguelands, The Corpsebeasts (27530): offered after all three of Kevin Frost's quests; three lightwells destroy a
-- hulk; lightwells 10 yd apart; the hulks are lifeless and unselectable; the reanimators channel into them; spent lightwells go.
--
--  * Order. Sourced: wiki storyline "All of the following, in any order ... then The Corpsebeasts", the quest page's previous
--    = all three. 27528 / 27529 / 27539 have NextQuestID 27530 and ExclusiveGroup 0, which in this core lets any one of them
--    unlock 27530. A negative group makes all three needed (Player::SatisfyQuestDependentPreviousQuests); both TDBs' positive
--    +27528 would make them mutually exclusive.
--  * Three lightwells (C++ npc_epl_corpsebeast). Sourced: "Place three or more Argent Lightwells around the corpsebeast"
--    (objectives); "you need to put 3 Lightwells around each body to count as one" (2010-11-25 [+153]); "once I did that,
--    they would immediately register" (peoriapeg's [+0] reply to 2012-07-20 [+8]). Before: each
--    barrage hit added to one SmartAI counter and ten hits from ONE lightwell (10 s) destroyed a hulk. Now the hulk counts the
--    distinct lightwells (anyone's) whose barrage hit it within 1.5 s and overloads at three (Holy Overload 85594: the credit
--    to players within 30 yd, the lightwells within 15 yd end), and goes 2 s later as before.
--  * Spacing (C++ spell_epl_summon_argent_lightwell on 85590). Sourced: "Must be 10 yards apart" (objectives); the refusal
--    "Must be 10 yards apart" (2011-05-13 [+9]) = SPELL_CUSTOM_ERROR_TOO_CLOSE_ARGENT_LIGHTWELL 124 "You are within 10 yards of
--    another Argent Lightwell."; wells of other players block too (comments of players locked out of spots). Ours: measured
--    flat from where the lightwell will stand (the caster's object size in front of the caster, the summon's own point).
--  * Unselectable hulks: TDB 4.4.2 unit_flags carry NOT_SELECTABLE ("lifeless hulks"); the barrage still lands (the
--    NOT_SELECTABLE check of SpellInfo::CheckTarget is commented out in this core).
--  * Reanimators. Sourced: "cultists ... channelling" into each hulk (description; "the five reanimators channelling the
--    beasts", 2011-01-10 [+25]); Reanimate Corpsebeast 85550 (channelled dummy on a nearby entry, 20 yd) is cast by nobody
--    and had no target condition. Ours: the timing (out of combat, once per spawn or evade, 1-3 s after it: a repeated cast
--    would restart the channel, since a running channel does not block a new cast).
--    All 38 reanimators stand within 29.2 yd of a hulk; the search adds both object sizes to its 20 yd. The channel (infinite)
--    breaks only on moving or on a spell hit (SpellInterrupts 18657): a reanimator pulled in melee would stand channelling, so it
--    stops on aggro ("they attack you while you place wells", 2011-01-17, 2012-07-20).
--  * Spent lightwells: the lightwell's "Force Despawn" row was action 37 (DIE) and left a dead lightwell lying there until its
--    90 s ran out; now it despawns, as its comment says.

-- Order
UPDATE `quest_template_addon` SET `ExclusiveGroup` = -27528 WHERE `ID` IN (27528,27529,27539);

-- Three lightwells: the hulks' C++ replaces their counter rows
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_epl_corpsebeast' WHERE `entry` IN (45886,45893,45895);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (45886,45893,45895) AND `source_type` = 0;

-- Spacing
DELETE FROM `spell_script_names` WHERE `spell_id` = 85590;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(85590,'spell_epl_summon_argent_lightwell');

-- Unselectable hulks
UPDATE `creature_template` SET `unit_flags` = `unit_flags` | 33554432 WHERE `entry` IN (45886,45893,45895);

-- Reanimators
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 85550;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,1,85550,0,0,31,0,3,45886,0,0,0,0,'','Reanimate Corpsebeast - target a Corpsebeast'),
(13,1,85550,0,1,31,0,3,45893,0,0,0,0,'','Reanimate Corpsebeast - target a Corpsebeast'),
(13,1,85550,0,2,31,0,3,45895,0,0,0,0,'','Reanimate Corpsebeast - target a Corpsebeast');
DELETE FROM `smart_scripts` WHERE `entryorguid` = 45897 AND `source_type` = 0 AND `id` IN (1,2);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`target_type`,`comment`) VALUES
(45897,0,1,0,1,0,100,1,1000,3000,0,0,11,85550,0,1,'Plaguewood Reanimator - Out of Combat - Channel Reanimate Corpsebeast (No Repeat)'),
(45897,0,2,0,4,0,100,0,0,0,0,0,92,0,85550,1,'Plaguewood Reanimator - On Aggro - Stop channelling Reanimate Corpsebeast');

-- Spent lightwells
UPDATE `smart_scripts` SET `action_type` = 41, `action_param1` = 0, `comment` = 'Argent Lightwell - On Spellhit Holy Overload - Despawn'
WHERE `entryorguid` = 45921 AND `source_type` = 0 AND `id` = 3;
