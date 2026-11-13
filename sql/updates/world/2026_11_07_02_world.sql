-- Exploration quests, other locales: 2026_11_07_01 moved the English AreaDescription of 90 exploration quests into their area trigger
-- objective (type 10) and emptied it, so the quest log shows the objective once. quest_template_locale still holds a localized
-- AreaDescription for these quests, so a non-English client got the localized line plus the English objective. Copy each localized
-- AreaDescription into quest_objectives_locale for that objective (only where no localized objective text exists yet), then empty the
-- localized AreaDescription where the objective now has localized text.
INSERT INTO `quest_objectives_locale` (`ID`, `locale`, `QuestId`, `StorageIndex`, `Description`, `VerifiedBuild`)
SELECT o.`id`, l.`locale`, o.`questId`, o.`index`, l.`AreaDescription`, 0
  FROM `quest_objective` o
  JOIN `quest_template_locale` l ON l.`ID` = o.`questId`
  LEFT JOIN `quest_objectives_locale` x ON x.`ID` = o.`id` AND x.`locale` = l.`locale`
  WHERE o.`type` = 10 AND l.`AreaDescription` <> '' AND x.`ID` IS NULL AND o.`questId` IN (62,76,201,273,287,437,455,503,578,869,870,944,984,1265,1448,1658,1699,1719,2240,2989,3505,4811,4842,5156,6025,6185,6421,8286,9160,9193,9260,9261,9262,9263,9264,9265,9400,9607,9608,9700,9701,9716,9731,9786,10269,10275,10750,10772,11652,11890,12032,12036,12263,12506,12665,12816,12817,13564,13607,13639,13816,13881,13961,14066,14389,24452,24618,25081,25325,25621,25715,25930,26258,26512,26930,26975,27007,27044,27152,27341,27349,27610,27704,28228,28635,28732,29392,29415,29536,29539);

UPDATE `quest_template_locale` l SET l.`AreaDescription` = ''
  WHERE l.`ID` IN (62,76,201,273,287,437,455,503,578,869,870,944,984,1265,1448,1658,1699,1719,2240,2989,3505,4811,4842,5156,6025,6185,6421,8286,9160,9193,9260,9261,9262,9263,9264,9265,9400,9607,9608,9700,9701,9716,9731,9786,10269,10275,10750,10772,11652,11890,12032,12036,12263,12506,12665,12816,12817,13564,13607,13639,13816,13881,13961,14066,14389,24452,24618,25081,25325,25621,25715,25930,26258,26512,26930,26975,27007,27044,27152,27341,27349,27610,27704,28228,28635,28732,29392,29415,29536,29539) AND l.`AreaDescription` <> ''
    AND EXISTS (SELECT 1 FROM `quest_objective` o JOIN `quest_objectives_locale` x ON x.`ID` = o.`id` AND x.`locale` = l.`locale`
                WHERE o.`questId` = l.`ID` AND o.`type` = 10 AND x.`Description` <> '');
