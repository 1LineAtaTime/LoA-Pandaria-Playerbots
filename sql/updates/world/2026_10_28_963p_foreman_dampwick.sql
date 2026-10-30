-- #963 (2026-10-28, run v12): Foreman Dampwick 34872 (Kezan, goblin start zone).
-- * creature_text: none in our DB (all 6 lines silently failed; "Event 3 ... non-existent Text id 2" at load) - ported from
--   the reference 5.4.8 world DB (reference DB creature_text entry 34872, 6 groups).
-- * smart_scripts: rows 0/1/2 linked to rows 1/2/3, but rows 1 and 3 are TEXT_OVER (52) events and row 2 was a TEXT_OVER
--   too - a link target must be SMART_EVENT_LINK (61): "Link Event 1/2/3 not found or invalid" (same rows in the reference DB).
--   Chain as intended: 0 on quest 14138 reward -> say 0; 1 on text 0 over -> cast 87226, linked 2 -> say 1; 3 on text 1 over
--   -> say 2. So: row 0 link 0, row 2 event 61, row 2 link 0.
-- Idempotent. Undo: DELETE creature_text entry 34872; link 1 on row 0, event_type 52 + link 3 on row 2.
DELETE FROM `creature_text` WHERE `CreatureID` = 34872;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(34872,0,0,'Always nice to get a gift hand delivered by the second-in-command. I\'ll unwrap this right now!',12,0,100,1,0,0,0,0,0,'Foreman Dampwick'),
(34872,1,0,'OUCH!',14,0,100,34,0,0,0,0,0,'Foreman Dampwick'),
(34872,2,0,'Okay, okay! I got the message!',12,0,100,1,0,0,0,0,0,'Foreman Dampwick'),
(34872,3,0,'Boss, tell Sassy that I\'ll get her more Kaja\'Cola just as soon as production resumes.',12,0,100,1,0,0,0,0,0,'Foreman Dampwick'),
(34872,4,0,'I can\'t believe the nerve of those trolls. Rebelling after all that I\'ve done for them!',12,0,100,1,0,0,0,0,0,'Foreman Dampwick'),
(34872,5,0,'You! Yes, you! Get back to work!',14,0,100,0,0,0,0,0,0,'Foreman Dampwick');
UPDATE `smart_scripts` SET `link` = 0 WHERE `entryorguid` = 34872 AND `source_type` = 0 AND `id` = 0;
UPDATE `smart_scripts` SET `event_type` = 61, `event_param1` = 0, `event_param2` = 0, `link` = 0 WHERE `entryorguid` = 34872 AND `source_type` = 0 AND `id` = 2;
