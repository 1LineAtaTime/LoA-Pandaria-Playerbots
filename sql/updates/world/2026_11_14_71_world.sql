-- Eastern Plaguelands, The Baroness' Missive (27551): the missive 61378 from Omasum Blighthoof 45867 or Karthis Darkrune 45868
-- starts the quest only after Blind Fury (27524), and it always drops.
--
--  * Sourced, prerequisite: two comments of 2010-11-24 ([+18], [+11]): the item answered "You don't meet the requirements" until
--    Blind Fury was done; the [+11] comment names Blind Fury. Without it anyone who kills a lieutenant could start 27551, hand
--    it to Tarenar and be offered Gidwin's Fate Revealed past the whole caravan chain. Both TDBs have PrevQuestID 0; the wiki's
--    "previous: Guardians of Stratholme" is not followed, because a 2011-01-21 comment did 27551 and 27526 with 27525 still
--    in the log. The missive still drops before Blind Fury (comments: "No idea why you can get the drop before then").
--  * Drop rate: 63.9 % / 73.2 % here, 100 % in both TDBs; a 2011-11-18 comment had it "not dropping after 10 kills", so the
--    true rate is not certain and the less grindy value is taken (the user's rule).

UPDATE `quest_template_addon` SET `PrevQuestID` = 27524 WHERE `ID` = 27551;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = 100 WHERE `entry` IN (45867,45868) AND `item` = 61378;
