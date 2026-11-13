-- ported from PR 15 2026_10_28_47_world.sql (#986)
-- Tirisfal Glades, Silverpine Forest and Hillsbrad Foothills: quests that are handed in from the quest log in 5.4.8.
-- Both TDBs (Cataclysm Classic 4.4.2 and retail 1210) give these quests QUEST_FLAGS_AUTO_COMPLETE (0x10000); mop lacked it,
-- so e.g. Transdimensional Warfare: Chapter II had to be walked back to Sylvanas and Chapter III was not offered on the spot.
-- Only that bit is added; mop's other flags are kept, and the quests' enders stay. (Terrible Little Creatures 28355 gets its
-- TDB flags in 2026_10_28_42.)
UPDATE `quest_template` SET `Flags`=`Flags`|0x10000 WHERE `ID` IN (
27095, -- Skitterweb Menace
27097, -- Rise, Forsaken
27513, -- Transdimensional Warfare: Chapter II
27574, -- I Never Forget a Face
28329, -- Angry Scrubbing Bubbles
28538, -- Cry of the Banshee
28566, -- Decimation
28603, -- Infiltration
28643, -- The Durnholde Challenge: Zephyrus
28644, -- The Durnholde Challenge: Teracula
28645, -- The Durnholde Challenge: Bloodvenom
28646); -- The Durnholde Challenge: Infernus
