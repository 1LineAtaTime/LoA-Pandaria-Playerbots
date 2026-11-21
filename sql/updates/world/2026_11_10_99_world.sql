-- Scholomancer (26924 Horde, 27162 Alliance): Koltira and Thassarian come to Darkmaster Gandling only when his scene summons them
-- Sourced: Wowhead comments (27162): "When you get Gandling to about 50% hp, he will lift you in the air, and Koltira and Thassarian
-- will come and finish him off, after he teleports away, Koltira and Thassarian will have the following conversation", the
-- conversation itself (Thassarian: "Koltira! I had heard rumours..."; Koltira: "You had heard correctly..."; ... Thassarian: "Very
-- well. I look forward to it."; Koltira whispers the agreement to a Horde player), "After Gandling disappears, Thassarian whispered
-- me" (Alliance); 26924: "You do get help from Koltira Deathweaver". broadcast_text 45042-45056 hold the battle cries and the
-- conversation in that order (45047 and 45054 "%s laughs."). Neither TDB has creature_text for 44452/44453 or a spawn of the Shadow
-- Prison 46558 (Vehicle 1204, an invisible vehicle summoned by 86797); their Koltira 328338 and Thassarian 328339 beside Gandling are
-- the sniffed scene actors.
-- Ours: the static Koltira (3126400, added by 2026_11_10_73) and Thassarian (213211) at Gandling and the stray Shadow Prison 312027
-- (random movement, in no TDB) go, since the scene summons all three (the camp copies, the quest givers and enders, are untouched);
-- who speaks which line (the laughs given to Koltira, who has just spoken each time; the whisper to each side by its own commander);
-- Thassarian's battle cry moved from the generic 7327 to the Andorhal 45042 (same text); the prison's ScriptName.

-- The scene actors and the stray prison
DELETE FROM `creature_addon` WHERE `guid` IN (213211,3126400,312027);
DELETE FROM `creature` WHERE (`guid`,`id`) IN ((213211,44453),(3126400,44452),(312027,46558));

UPDATE `creature_template` SET `ScriptName`='npc_wpl_gandling_shadow_prison' WHERE `entry`=46558;

-- Koltira Deathweaver 44452 (group 0, the quest accept line, stays)
DELETE FROM `creature_text` WHERE `CreatureID`=44452 AND `GroupID` BETWEEN 1 AND 7;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(44452,1,0,'For the Horde!',14,0,100,0,0,0,0,45043,0,'Koltira Deathweaver - Scholomancer, arriving at Gandling'),
(44452,2,0,'You heard correctly, Thassarian.  I wanted to give you a chance to kill me again.',12,0,100,0,0,0,0,45046,0,'Koltira Deathweaver - Scholomancer conversation'),
(44452,3,0,'%s laughs.',16,0,100,0,0,0,0,45047,0,'Koltira Deathweaver - Scholomancer conversation'),
(44452,4,0,'Yes, it may... but not today.  With the Scourge out of the way, I plan to pull my troops back and regroup.',12,0,100,0,0,0,0,45049,0,'Koltira Deathweaver - Scholomancer conversation'),
(44452,5,0,'Yes, I know.  And don''t think I''ll go easy on you.',12,0,100,0,0,0,0,45053,0,'Koltira Deathweaver - Scholomancer conversation'),
(44452,6,0,'%s laughs.',16,0,100,0,0,0,0,45054,0,'Koltira Deathweaver - Scholomancer conversation'),
(44452,7,0,'You are not to speak to ANYONE about the agreement that was made today, $n.  Consider this an order from your direct superior.',15,0,100,0,0,0,0,45056,0,'Koltira Deathweaver - Scholomancer, to the Horde players');

-- Thassarian 44453 (group 0, the quest accept line, stays)
DELETE FROM `creature_text` WHERE `CreatureID`=44453 AND `GroupID` BETWEEN 1 AND 8;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(44453,1,0,'For the Alliance!',14,0,100,0,0,0,0,45042,0,'Thassarian - Scholomancer, arriving at Gandling'),
(44453,2,0,'Koltira!  I had heard rumors that you were here in Lordaeron.',12,0,100,0,0,0,0,45045,0,'Thassarian - Scholomancer conversation'),
(44453,3,0,'It looks like it may have to come to that.',12,0,100,0,0,0,0,45048,0,'Thassarian - Scholomancer conversation'),
(44453,4,0,'I see.  Then I will do the same.',12,0,100,0,0,0,0,45050,0,'Thassarian - Scholomancer conversation'),
(44453,5,0,'But you must be aware, Koltira: we are brothers no longer.  You are a member of the Horde now, and I of the Alliance.',12,0,100,0,0,0,0,45051,0,'Thassarian - Scholomancer conversation'),
(44453,6,0,'Eventually, we will fight this battle for Andorhal, and only one of us can walk off of this battlefield victorious.',12,0,100,0,0,0,0,45052,0,'Thassarian - Scholomancer conversation'),
(44453,7,0,'Very well.  I look forward to it.',12,0,100,0,0,0,0,45055,0,'Thassarian - Scholomancer conversation'),
(44453,8,0,'You are not to speak to ANYONE about the agreement that was made today, $n.  Consider this an order from your direct superior.',15,0,100,0,0,0,0,45056,0,'Thassarian - Scholomancer, to the Alliance players');
