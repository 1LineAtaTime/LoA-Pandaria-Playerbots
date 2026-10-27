-- Break in Communications: Dreadwatch Outpost (27349): "Investigate Dreadwatch Outpost" is an area-trigger objective, but no
-- area trigger was linked to it, so the objective was never met (the quest could only be handed in early through its
-- auto-complete flag). The Cataclysm Classic world database gives it as AreaTrigger 6246 (Dreadwatch Outpost, Northern
-- Headlands). This core keys area-trigger objectives by the objective's own id, so the link goes through
-- areatrigger_involvedrelation, which also wants the quest's exploration flag.
DELETE FROM `areatrigger_involvedrelation` WHERE `id` = 6246;
INSERT INTO `areatrigger_involvedrelation` (`id`, `quest`) VALUES
(6246, 27349);
UPDATE `quest_objective` SET `objectId` = 6246 WHERE `questId` = 27349 AND `id` = 266335;
UPDATE `quest_template_addon` SET `SpecialFlags` = `SpecialFlags` | 2 WHERE `ID` = 27349;
