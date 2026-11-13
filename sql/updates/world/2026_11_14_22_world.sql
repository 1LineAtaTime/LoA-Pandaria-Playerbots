-- Eastern Plaguelands: three quests that were handed in from the quest log on retail had to be taken back to the giver.
--
--  * What I Do Best (27371, Tarenar Sunstrike at Death's Step): "you don't have to talk to the quest NPC ... click the gigantic
--    question mark on your quest tracker" (comments 2010-11-26 [+11], 2011-01-16 [+6]); wiki "End: Automatic". It matters more
--    here than elsewhere: while his follower is out the standing Tarenar is hidden (the caravan staging row 49416 / 5446 carries
--    aura_spell -84902), so the log is the way to hand it in on the spot. RewardNextQuest 27372 is already set, and
--    Player::GetNextQuest offers it from a log hand-in once the flag is there.
--  * To Kill With Purpose (27451) and Dark Garb (27452), Betina Bigglezink at Light's Shield Tower: her offer-reward texts
--    speak of "home-brewed magic to communicate with you in the field" and the portraits (PortraitTurnIn 10465) are already in
--    the rows; wiki "End: Automatic".
-- Sourced: both TDBs carry QUEST_FLAGS_AUTO_COMPLETE 0x10000 on all three where mop lacks it (tdb-diff.md; no other flag
-- differs in the comparison). 65544 = mop's 8 | 0x10000.

UPDATE `quest_template` SET `Flags` = 65544 WHERE `ID` IN (27371, 27451, 27452);
