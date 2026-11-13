-- Restore two absent runtime creature talk entries (eight authored lines) from the
-- original LoA v1.0.4 database release. Asset SHA256:
-- db8dd845e3a5da4bc3a6f28afa68ed839aceb222101db949ae74840a5a2365a0
-- https://github.com/1LineAtaTime/LoA-Pandaria-Playerbots/releases/tag/v1.0.4
-- Map the release CreatureID/GroupID/ID key to the live legacy primary key.
-- Verified matching NPC names, valid emotes and existing broadcast IDs.
-- Existing groups win whole; reject groups with unavailable broadcast text.
CREATE TEMPORARY TABLE codex_loa_talk_restore (
 entry INT UNSIGNED,groupid INT UNSIGNED,id INT UNSIGNED,text LONGTEXT,
 type INT UNSIGNED,language INT UNSIGNED,probability FLOAT,emote INT UNSIGNED,
 duration INT UNSIGNED,sound INT UNSIGNED,SoundType INT UNSIGNED,
 BroadcastTextId INT UNSIGNED,TextRange INT UNSIGNED,NpcName VARCHAR(255));
INSERT INTO codex_loa_talk_restore VALUES
(19477,0,0,CONVERT(0x546865207461736b6d617374657227732064656164212020517569636b2c2044726567686f6f647321202052756e21 USING utf8mb4),12,0,100,0,0,0,0,13372,0,CONVERT(0x466c6565696e672044726567686f6f642057617272696f72 USING utf8mb4)),
(19477,0,1,CONVERT(0x466c6565212020466c656521 USING utf8mb4),12,0,100,0,0,0,0,13373,0,CONVERT(0x466c6565696e672044726567686f6f642057617272696f72 USING utf8mb4)),
(19477,0,2,CONVERT(0x54686973206973206f7572206368616e636521202052756e20666f7220697421 USING utf8mb4),12,0,100,0,0,0,0,13374,0,CONVERT(0x466c6565696e672044726567686f6f642057617272696f72 USING utf8mb4)),
(34913,0,0,CONVERT(0x5374616e6420796f75722067726f756e642c206d656e21 USING utf8mb4),14,0,100,0,0,19617,0,35166,0,CONVERT(0x5072696e6365204c69616d20477265796d616e65 USING utf8mb4)),
(34913,0,1,CONVERT(0x44656665617420746865736520666f756c2062656173747321 USING utf8mb4),14,0,100,0,0,19618,0,35167,0,CONVERT(0x5072696e6365204c69616d20477265796d616e65 USING utf8mb4)),
(34913,0,2,CONVERT(0x50726f746563742074686520636976696c69616e7321 USING utf8mb4),14,0,100,0,0,19619,0,35168,0,CONVERT(0x5072696e6365204c69616d20477265796d616e65 USING utf8mb4)),
(34913,0,3,CONVERT(0x50757368207468656d206261636b21 USING utf8mb4),14,0,100,0,0,19620,0,35169,0,CONVERT(0x5072696e6365204c69616d20477265796d616e65 USING utf8mb4)),
(34913,0,4,CONVERT(0x54616b652068656172742c206d656e2120205765206d7573742070726f74656374206f7572206369747921 USING utf8mb4),14,0,100,0,0,19621,0,35170,0,CONVERT(0x5072696e6365204c69616d20477265796d616e65 USING utf8mb4));
CREATE TEMPORARY TABLE codex_existing_loa_talk_groups AS
 SELECT DISTINCT entry,groupid FROM creature_text;
CREATE TEMPORARY TABLE codex_unavailable_loa_talk_groups AS
 SELECT DISTINCT s.entry,s.groupid FROM codex_loa_talk_restore s
 LEFT JOIN broadcast_text b ON b.ID=s.BroadcastTextId
 WHERE s.BroadcastTextId<>0 AND b.ID IS NULL;
INSERT IGNORE INTO creature_text
 (entry,groupid,id,text,type,language,probability,emote,duration,sound,SoundType,BroadcastTextId,TextRange,comment,CreatureID)
 SELECT s.entry,s.groupid,s.id,s.text,s.type,s.language,s.probability,s.emote,s.duration,s.sound,s.SoundType,s.BroadcastTextId,s.TextRange,'[source: LoA v1.0.4]',s.entry
 FROM codex_loa_talk_restore s
 JOIN creature_template c ON c.entry=s.entry AND c.name=s.NpcName
 LEFT JOIN codex_existing_loa_talk_groups e ON e.entry=s.entry AND e.groupid=s.groupid
 LEFT JOIN codex_unavailable_loa_talk_groups u ON u.entry=s.entry AND u.groupid=s.groupid
 WHERE e.entry IS NULL AND u.entry IS NULL;
DROP TEMPORARY TABLE codex_loa_talk_restore;
DROP TEMPORARY TABLE codex_existing_loa_talk_groups;
DROP TEMPORARY TABLE codex_unavailable_loa_talk_groups;
