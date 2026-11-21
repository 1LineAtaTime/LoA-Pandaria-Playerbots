-- Light's Hope Chapel: the doubled gossip options of Archmage Angela Dosantos' Atiesh story, Rohan the Assassin and Huntsman
-- Leopold go.
--
-- Reported in game: Archmage Angela Dosantos 16116 has double gossip options. Every menu of her "What is Atiesh?" story (7127
-- to 7135) held its one option twice, as OptionID 0 and 1 (same text, same next menu); so did the "Where can I find ..."
-- option of Rohan the Assassin (menu 7101) and Huntsman Leopold (menu 7102). No conditions name these options.
--
-- Sourced: both TDBs (4.4.2 and 12.1): one option per menu on 7127-7135. Her top menu 7096 (What is Atiesh? / Where can I
--   find core of elements?) and Korfax' 7099 are in both TDBs as we have them, so the Naxxramas-era talk stays.
-- Not done: on 7101 and 7102 the TDBs' first option ("What is it that you do exactly, Rohan?" -> 7118, "What is Cryptstalker
--   armor?" -> 7111) was lost under the copy; those story menus (7118, 7111 and on) are not in this database.

DELETE o1 FROM `gossip_menu_option` o1
JOIN `gossip_menu_option` o0 ON o0.`MenuID` = o1.`MenuID` AND o0.`OptionID` = 0
  AND o0.`OptionText` = o1.`OptionText` AND o0.`ActionMenuID` = o1.`ActionMenuID` AND o0.`OptionType` = o1.`OptionType`
WHERE o1.`OptionID` = 1 AND o1.`MenuID` IN (7101, 7102, 7127, 7128, 7129, 7130, 7131, 7132, 7133, 7134, 7135);

-- The translations of the removed copies
DELETE l FROM `gossip_menu_option_locale` l
LEFT JOIN `gossip_menu_option` o ON o.`MenuID` = l.`MenuID` AND o.`OptionID` = l.`OptionID`
WHERE o.`MenuID` IS NULL AND l.`OptionID` = 1 AND l.`MenuID` IN (7101, 7102, 7127, 7128, 7129, 7130, 7131, 7132, 7133, 7134, 7135);
