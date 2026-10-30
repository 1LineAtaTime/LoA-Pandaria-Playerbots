-- ported from PR 15 2026_10_26_30_world.sql (#986)
-- Silverpine Forest: Excising the Taint (27181) is turned in to Sylvanas' voice, like Seek and Destroy (27193) after it
-- The 4.3.4 sniff has Flags 0x190000; our row lost QUEST_FLAGS_AUTO_COMPLETE (0x10000), so the client offered no remote turn-in
UPDATE `quest_template` SET `Flags` = 0x190000 WHERE `ID` = 27181;
