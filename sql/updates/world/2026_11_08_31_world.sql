-- The Myzrael chain: quest texts and emotes. Sourced: TDB 442, copied as they are (VerifiedBuild 0: the 4.4.2 build numbers do not fit mop's columns).
-- The Princess Unleashed (Horde 26912): completion log "in The Hinterlands" -> "in Arathi Highlands" (TDB 442; the Alliance 26049 already says Arathi Highlands).
-- The Stone Shards (Alliance 26341): "everythwhere" -> "everywhere" (TDB 442).
-- quest_details emotes for all nine quests (mop had none); reward texts with their <stage directions> and $B breaks, and Zaruk's and Skuerto's emote 274 on Myzrael's Tale;
-- trailing spaces dropped. Nothing here is ours.
UPDATE `quest_template` SET `QuestCompletionLog` = 'Return to the Shards of Myzrael in Arathi Highlands.' WHERE `ID` = 26912;
UPDATE `quest_template` SET `QuestDescription` = 'I''ve scouted all over these highlands, $n, and there''s one thing I''ve found I haven''t been able to explain.$B$BTo the northeast of here, there''s a circle of stones, like the others you''ll find everywhere in Arathi... but the largest and most imposing of them, built into the mountainside itself.$B$BJust between you and me, I heard voices while I was there. Whispers of the earth. If you''re the curious type, you could go see for yourself...' WHERE `ID` = 26341;
DELETE FROM `quest_details` WHERE `ID` IN (26909,26341,26039,26041,26042,26911,26346,26912,26049);
INSERT INTO `quest_details` (`ID`,`Emote1`,`Emote2`,`Emote3`,`Emote4`,`EmoteDelay1`,`EmoteDelay2`,`EmoteDelay3`,`EmoteDelay4`,`VerifiedBuild`) VALUES
(26909,1,1,0,0,0,0,0,0,0),
(26341,1,1,0,0,0,0,0,0,0),
(26039,0,1,0,0,0,0,0,0,0),
(26041,0,1,0,0,0,0,0,0,0),
(26042,0,1,0,0,0,0,0,0,0),
(26911,0,1,0,0,0,0,0,0,0),
(26346,0,1,0,0,0,0,0,0,0),
(26912,0,1,0,0,0,0,0,0,0),
(26049,0,1,0,0,0,0,0,0,0);
DELETE FROM `quest_request_items` WHERE `ID` = 26912;
INSERT INTO `quest_request_items` (`ID`,`EmoteOnComplete`,`EmoteOnIncomplete`,`CompletionText`,`VerifiedBuild`) VALUES
(26912,0,0,'The Shards of Myzrael are now strangely quiet.',0);
DELETE FROM `quest_offer_reward` WHERE `ID` IN (26909,26341,26039,26041,26042,26911,26346,26912,26049);
INSERT INTO `quest_offer_reward` (`ID`,`Emote1`,`Emote2`,`Emote3`,`Emote4`,`EmoteDelay1`,`EmoteDelay2`,`EmoteDelay3`,`EmoteDelay4`,`RewardText`,`VerifiedBuild`) VALUES
(26909,0,0,0,0,0,0,0,0,'<The shards pulse softly, and a whisper slowly echoes through your head.>$B$BHelp...',0),
(26341,0,0,0,0,0,0,0,0,'<The shards pulse softly, and a whisper slowly echoes through your head.>$B$BHelp...',0),
(26039,0,0,0,0,0,0,0,0,'Ah!  You activated these shards.  Already I feel more myself!$B$BBut alas, I am still far from free...',0),
(26041,0,0,0,0,0,0,0,0,'<Myzrael speaks through the stone.>$B$BYou have done it!  You opened the Stones of Binding!  I can feel the shackles about me loosen, and freedom draws ever closer.$B$BYou are a noble $c, $N.  My rescuer!',0),
(26042,0,0,0,0,0,0,0,0,'You place the rod in the keystone.  And a shudder is felt beneath you...',0),
(26911,274,0,0,0,0,0,0,0,'I know, $n. I should not have sent you.$B$BForgive me... the whispers of the past are sometimes too strong.',0),
(26346,274,0,0,0,0,0,0,0,'Say no more, $n. I followed up with an... "old friend" while you were gone. I should''ve known better than to send you to those shards.$B$BWe''ve both made a grave mistake in involving ourselves in this. Luckily, I "persuaded" my boy Gerrig to tell me what to do if this happened.',0),
(26912,0,0,0,0,0,0,0,0,'As the eldritch shackles are placed around the Shards, an unhuman wail is heard from far below.  The scream is followed by a roar.$B$BThe Shards of Myzrael no longer invoke feelings of warmth and security as they once did.  Now they pulse with an open menace... as if the thing beneath them holds ill will toward the surface.',0),
(26049,0,0,0,0,0,0,0,0,'As the eldritch shackles are placed around the Shards, an unhuman wail is heard from far below.  The scream is followed by a roar.$B$BThe Shards of Myzrael no longer invoke feelings of warmth and security as they once did.  Now they pulse with an open menace... as if the thing beneath them holds ill will toward the surface.',0);
