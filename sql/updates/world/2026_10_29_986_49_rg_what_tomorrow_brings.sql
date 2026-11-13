-- ported from PR 15 2026_10_27_65_world.sql (#986)
-- What Tomorrow Brings (27401): the 7th Legion Telescope seats the player in Gilneas Camera 01 (45427, Summon Gilneas Camera), but
-- the camera had no script, so it never moved and nothing gave the credit (Quest Credit: What Tomorrow Brings, kill credit 45430,
-- the camera target over the harbour). The camera now flies out over the harbour, holds there, credits the player and brings them
-- back to the telescope (see npc_silverpine_gilneas_camera).
-- Both world databases auto-complete the quest (flags 589824): it is handed in over the F.C.D., which then gives Fall Back!
-- ("<The F.C.D. buzzes.> <Howls can be heard in the background.>") in the Cathedral Quarter, as Excising the Taint is handed in
-- to Sylvanas' voice. (This core also lets an auto-complete quest be handed in at its ender before its objectives are done.)
UPDATE `quest_template` SET `Flags` = 589824 WHERE `ID` = 27401;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_silverpine_gilneas_camera' WHERE `entry` = 45427;
