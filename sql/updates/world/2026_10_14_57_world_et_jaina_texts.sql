-- #694 (End Time script gaps, found in the #671 w61 lane): Echo of Jaina's creature_text groups do not match her script.
-- boss_echo_of_jaina.cpp:21-28 Talks SAY_AGGRO=0 (JustEngagedWith :191), SAY_DEATH=1 (JustDied :153), SAY_INTRO=2 (first staff
-- fragment, DoAction :129-133), SAY_KILL=3 (KilledUnit :166), SAY_SPELL=4 (Frost Blades, :265). The world DB groups are
-- 0 = intro + aggro, 1 = aggro, 2 = Frost Blades, 3 = death + kills, 4 = kills, so the first fragment yelled a Frost Blades line
-- (w61 master log 19:52:18 "A little ice ought to quench the fire in your hearts..."), her death said "You asked for it." and
-- Frost Blades said kill lines. Regrouped to the script with the same rows (Sound / BroadcastTextId unchanged).
-- Intended lines: Wowpedia "Echo of Jaina" (MediaWiki API, fetched 2026-10-14) Quotes: Intro (say) "I don't know who you are...";
-- Aggro (yell) "You asked for it." / "I hate resorting to violence."; Frost Blades (yell) "Why won't you give up?!" /
-- "Perhaps this will cool your heads..." / "A little ice ought to quench the fire in your hearts..."; Killing a player (yell)
-- "You forced my hand." / "I didn't want to do that." / "I wish you'd surrendered!"; Death (say) "I understand, now...".
-- NOT APPLIED - staged for review. Replaces the 12 rows of the MoP test world DB (2026-10-14) with 10: "I didn't want to do
-- that." / "I wish you'd surrendered." were in groups 3 and 4 both, they are kept once in SAY_KILL.

DELETE FROM `creature_text` WHERE `CreatureID` = 54445;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(54445, 0, 0, 'You asked for it.', 14, 0, 100, 0, 0, 25917, 0, 53040, 0, 'Echo of Jaina - Aggro 1'),
(54445, 0, 1, 'I hate resorting to violence.', 14, 0, 100, 0, 0, 25918, 0, 56572, 0, 'Echo of Jaina - Aggro 2'),
(54445, 1, 0, 'I understand, now. Farewell, and good luck.', 12, 0, 100, 0, 0, 25919, 0, 56574, 0, 'Echo of Jaina - Death'),
(54445, 2, 0, 'I don''t know who you are, but I''ll defend this shrine with my life. Leave, now, before we come to blows.', 12, 0, 100, 0, 0, 25920, 0, 56573, 0, 'Echo of Jaina - Intro (first staff fragment)'),
(54445, 3, 0, 'You forced my hand.', 14, 0, 100, 0, 0, 25921, 0, 56575, 0, 'Echo of Jaina - Slay 1'),
(54445, 3, 1, 'I didn''t want to do that.', 14, 0, 100, 0, 0, 25922, 0, 56576, 0, 'Echo of Jaina - Slay 2'),
(54445, 3, 2, 'I wish you''d surrendered.', 14, 0, 100, 0, 0, 25923, 0, 56577, 0, 'Echo of Jaina - Slay 3'),
(54445, 4, 0, 'Why won''t you give up?!', 14, 0, 100, 0, 0, 25926, 0, 56580, 0, 'Echo of Jaina - Frost Blades 1'),
(54445, 4, 1, 'Perhaps this will cool your heads...', 14, 0, 100, 0, 0, 25924, 0, 56578, 0, 'Echo of Jaina - Frost Blades 2'),
(54445, 4, 2, 'A little ice ought to quench the fire in your hearts...', 14, 0, 100, 0, 0, 25925, 0, 56579, 0, 'Echo of Jaina - Frost Blades 3');
