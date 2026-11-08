-- Myzrael's Tale (Horde 26911, Alliance 26346): offered by the Stone of Inner Binding only after Breaking the Keystone (26042) is handed in.
-- Ours: no data source has a prerequisite (PrevQuestID 0 in mop, TDB 442, TDB 1210; Wowhead's series starts with Myzrael's Tale). The order is sourced from the quest text
-- ("tell Zaruk of what you've done so far"), warcraft.wiki.gg (Breaking the Keystone -> Myzrael's Tale) and Wowhead comments (the stone offers it after the Keystone).
-- 26042 is offered to both factions, so one link serves both. Characters who already hold Myzrael's Tale keep it.
UPDATE `quest_template_addon` SET `PrevQuestID` = 26042 WHERE `ID` IN (26911,26346);
