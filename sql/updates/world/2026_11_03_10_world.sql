-- Hillsbrad Foothills, the Sludge Fields: The Battle for Hillsbrad (28196) starts only from Clerk Horrace Whitesteed's journal.
-- Sources: Cataclysm Classic TDB 442 and retail TDB 1210 (no creature starter for 28196; creature_loot_template 47789 ->
-- 63250, Chance 100, QuestRequired 1), Wowhead MoP Classic Quick Facts (no "Start", end Warden Stillwater),
-- warcraft.wiki.gg (starts from the item The Battle for Hillsbrad; Discretion is Key: Clerk Horrace Whitesteed drops the book).

-- Warden Stillwater offered the quest himself; he only takes the journal in.
DELETE FROM `creature_queststarter` WHERE `id`=47781 AND `quest`=28196;

-- The journal (63250, StartQuest 28196, party loot) dropped from Whitesteed 82% of the time; both TDBs drop it every time.
-- The TDBs' QuestRequired=1 is not copied as a quest drop (negative chance): no quest has 63250 as an objective or ItemDrop,
-- so LootItem::AllowedForPlayer would never let anyone loot it. As a plain drop the core still hides it once 28196 has been
-- taken or done (the StartQuest check in AllowedForPlayer).
UPDATE `creature_loot_template` SET `ChanceOrQuestChance`=100 WHERE `entry`=47789 AND `item`=63250;
