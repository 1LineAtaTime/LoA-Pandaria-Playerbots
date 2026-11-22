-- Hallow's End, Creepy Crate chain: quest texts.
-- Problem 1: Taking Precautions (29426 Horde, 29402 Alliance) asks for "5 Crystal Vials, 5 Arcane Powder, and 5 Blood
--   Nettle" and tells the player to buy arcane powder, while the objectives are 2 Strange Dust, 5 Crystal Vials and
--   5 Blood Nettle. Arcane Powder was removed in patch 5.0.4; the text is the one from before.
-- Problem 2: The Creepy Crate (29429 Horde, 29413 Alliance) has no quest_offer_reward row, so the completion page shows
--   no text.
-- Change: new LogDescription and one sentence of QuestDescription for 29426/29402 (other paragraphs unchanged); the
--   completion text for 29429/29413.
-- Source: Wowhead MoP Classic quest 29426 and 29402 ("Obtain 2 Strange Dust, 5 Crystal Vials, and 5 Blood Nettle.",
--   "Acquire some strange dust, get vials from an alchemy merchant, ..."), quest 29429 (Completion).
-- Rollback:
--   UPDATE `quest_template` SET `LogDescription`='Obtain 5 Crystal Vials, 5 Arcane Powder, and 5 Blood Nettle.', `QuestDescription`=REPLACE(`QuestDescription`,'Acquire some strange dust, get vials','Buy some arcane powder from a reagent merchant, get vials') WHERE `ID` IN (29426,29402);
--   DELETE FROM `quest_offer_reward` WHERE `ID` IN (29413,29429);
UPDATE `quest_template` SET `LogDescription`='Obtain 2 Strange Dust, 5 Crystal Vials, and 5 Blood Nettle.',
  `QuestDescription`=REPLACE(`QuestDescription`,'Buy some arcane powder from a reagent merchant, get vials','Acquire some strange dust, get vials')
WHERE `ID` IN (29426,29402);
DELETE FROM `quest_offer_reward` WHERE `ID` IN (29413,29429);
INSERT INTO `quest_offer_reward` (`ID`,`Emote1`,`Emote2`,`Emote3`,`Emote4`,`EmoteDelay1`,`EmoteDelay2`,`EmoteDelay3`,`EmoteDelay4`,`RewardText`,`VerifiedBuild`) VALUES
(29413,0,0,0,0,0,0,0,0,'It''s been sought after, stolen, and sold. Anyone who tried to tap the magic held within has seen that power turn against him.$B$BYet, the crate itself appears unscathed and suffused with magical energy. From time to time, it shakes or bobs as if its contents were alive.$B$BIt seems to have an affinity for you and now, it''s yours.',0),
(29429,0,0,0,0,0,0,0,0,'It''s been sought after, stolen, and sold. Anyone who tried to tap the magic held within has seen that power turn against him.$B$BYet, the crate itself appears unscathed and suffused with magical energy. From time to time, it shakes or bobs as if its contents were alive.$B$BIt seems to have an affinity for you and now, it''s yours.',0);
