-- Western Plaguelands: Daria L'Rayne (45147) follows the player with her mind vision, so her Hearthglen chain is handed in
-- from the quest log (QUEST_FLAGS_AUTO_COMPLETE 0x10000) and each next quest is offered at once (RewardNextQuest).
-- Retail (1210) and Cataclysm Classic (442) both set the flag on 27151-27154; 27155 (to Tirion) is handed in in person.
UPDATE `quest_template` SET `Flags`=`Flags`|0x10000 WHERE `ID` IN (27151,27152,27153,27154);
