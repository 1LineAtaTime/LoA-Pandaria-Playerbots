-- Eastern Plaguelands, Tyr's Hand: the five named Scarlets of The Commander, The Wrathcaster, The Huntsman, The Assassin and
-- Like Rats (46092-46096).
-- Sourced:
--  * faction 67 (Scarlet Crusade) in both TDBs; mop_world had 14.
--  * each speaks on aggro: broadcast_text 50339 (Marjhan, say), 50345 (Mataus, say), 50344 (Leopold, yell), 50341 (Rohan, yell),
--    50348 (Valdelmar, yell); say / yell as the quest pages on warcraft.wiki.gg give them.
--  * Rohan the Assassin crouches stealthed behind the platform of the bell tower and "won't attack till you do, even if you run
--    straight through him" (Wowhead comments 2010-12-30, 2011-01-09, 2010-12-20). Leopold stands on the barracks roof.
-- Ours: how Rohan waits. He is passive at rest (SmartAI::MoveInLineOfSight starts nothing for a passive creature); the first
-- hostile act against him engages him (Unit::SetInCombatState calls JustEngagedWith whatever the react state), and on that he
-- yells, turns aggressive and attacks whoever engaged him. Evade and respawn both run SmartScript::OnReset, which makes him
-- passive again; his own rows 0 and 1 put the Stealth back. Nothing else (faction, flags) is changed at run time.
-- Ours: Rohan and Leopold no longer wander 5 yd (a tower platform and a roof ledge).
-- What the player sees: Rohan invisible until attacked, then "Die!" and a normal fight; the other four speak when they engage.

UPDATE `creature_template` SET `faction` = 67 WHERE `entry` IN (46092, 46093, 46094, 46095, 46096);

DELETE FROM `creature_text` WHERE `CreatureID` IN (46092, 46093, 46094, 46095, 46096) AND `GroupID` = 0;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(46092, 0, 0, 'I will kill you, or I will die trying.', 12, 0, 100, 0, 0, 0, 0, 50339, 0, 'Scarlet Commander Marjhan - aggro'),
(46093, 0, 0, 'Your pain is my release.', 12, 0, 100, 0, 0, 0, 0, 50345, 0, 'Mataus the Wrathcaster - aggro'),
(46094, 0, 0, 'I''ll fill your corpses with arrows!', 14, 0, 100, 0, 0, 0, 0, 50344, 0, 'Huntsman Leopold - aggro'),
(46095, 0, 0, 'Die!', 14, 0, 100, 0, 0, 0, 0, 50341, 0, 'Rohan the Assassin - aggro'),
(46096, 0, 0, 'The Scarlet Crusade is not over! Undeath is merely a setback!', 14, 0, 100, 0, 0, 0, 0, 50348, 0, 'Crusader Lord Valdelmar - aggro');

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (46092, 46093, 46094, 46095, 46096);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND ((`entryorguid` = 46092 AND `id` = 3) OR (`entryorguid` IN (46093, 46094, 46096) AND `id` = 4) OR (`entryorguid` = 46095 AND `id` IN (5, 6, 7, 8)));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(46092, 0, 3, 0, 4, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Scarlet Commander Marjhan - On Aggro - Say Line 0'),
(46093, 0, 4, 0, 4, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Mataus the Wrathcaster - On Aggro - Say Line 0'),
(46094, 0, 4, 0, 4, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Huntsman Leopold - On Aggro - Say Line 0'),
(46096, 0, 4, 0, 4, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Crusader Lord Valdelmar - On Aggro - Say Line 0'),
(46095, 0, 5, 6, 4, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rohan the Assassin - On Aggro - Say Line 0'),
(46095, 0, 6, 7, 61, 0, 100, 0, 0, 0, 0, 0, 0, 8, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rohan the Assassin - On Aggro (Link) - Set React Aggressive'),
(46095, 0, 7, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 49, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Rohan the Assassin - On Aggro (Link) - Attack Start Invoker'),
(46095, 0, 8, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rohan the Assassin - On Reset - Set React Passive');

UPDATE `creature` SET `MovementType` = 0, `wander_distance` = 0 WHERE `guid` IN (372313, 372315) AND `id` IN (46095, 46094);
