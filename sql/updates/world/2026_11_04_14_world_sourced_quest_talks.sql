-- Restore four absent SmartAI talk groups from ProjectSkyfire sf_db_26,
-- asset SFDB_full_548_26.002_2026_008_18_Release.zip (SHA256
-- d12ba9ecad381f3b5460612af2a172cd373afdd796c289e2ccc11315db21dd5f).
-- https://github.com/ProjectSkyfire/SkyFire_548/releases/tag/sf_db_26
-- Preserve authored text/type/language/probability. Sound/emote are all zero.
-- Source and local chat type 13/15 mean MONSTER_PARTY/MONSTER_WHISPER.
-- Only empty groups on the matching named NPC are eligible; custom groups win.
INSERT IGNORE INTO creature_text
 (entry,groupid,id,text,type,language,probability,emote,duration,sound,BroadcastTextId,TextRange,comment,CreatureID,SoundType)
 SELECT 3584,0,0,CONVERT(0x4f6b2c206c6574277320676f21 USING utf8mb4),13,0,100,0,0,0,0,0,'[source: SkyFire sf_db_26]',3584,0
 FROM creature_template c WHERE c.entry=3584 AND c.name=CONVERT(0x54686572796c756e65 USING utf8mb4)
 AND NOT EXISTS (SELECT 1 FROM creature_text t WHERE t.entry=3584 AND t.groupid=0);
INSERT IGNORE INTO creature_text
 (entry,groupid,id,text,type,language,probability,emote,duration,sound,BroadcastTextId,TextRange,comment,CreatureID,SoundType)
 SELECT 3584,1,0,CONVERT(0x492063616e206d616b65206974207468652072657374206f6620746865207761792c20244e2e205468616e6b7321 USING utf8mb4),13,0,100,0,0,0,0,0,'[source: SkyFire sf_db_26]',3584,0
 FROM creature_template c WHERE c.entry=3584 AND c.name=CONVERT(0x54686572796c756e65 USING utf8mb4)
 AND NOT EXISTS (SELECT 1 FROM creature_text t WHERE t.entry=3584 AND t.groupid=1);
INSERT IGNORE INTO creature_text
 (entry,groupid,id,text,type,language,probability,emote,duration,sound,BroadcastTextId,TextRange,comment,CreatureID,SoundType)
 SELECT 3692,1,0,CONVERT(0x4f68206e6f21205468657920617265206865726521 USING utf8mb4),13,0,100,0,0,0,0,0,'[source: SkyFire sf_db_26]',3692,0
 FROM creature_template c WHERE c.entry=3692 AND c.name=CONVERT(0x566f6c636f72 USING utf8mb4)
 AND NOT EXISTS (SELECT 1 FROM creature_text t WHERE t.entry=3692 AND t.groupid=1);
INSERT IGNORE INTO creature_text
 (entry,groupid,id,text,type,language,probability,emote,duration,sound,BroadcastTextId,TextRange,comment,CreatureID,SoundType)
 SELECT 7804,0,0,CONVERT(0x486561722079652c2064656e697a656e73206f662054616e6172697321204c6574206974206265206b6e6f776e20244e20697320616e206578616c746564206d656d626572206f6620746865204d69746872696c204f726465722e204120626c61636b736d697468206f6620686f6e6f722c2064656469636174696f6e2c20616e6420696e66696e6974652070617469656e63652e2054687265652063686565727320666f7220244e21 USING utf8mb4),15,0,100,0,0,0,0,0,'[source: SkyFire sf_db_26]',7804,0
 FROM creature_template c WHERE c.entry=7804 AND c.name=CONVERT(0x5472656e746f6e204c6967687468616d6d6572 USING utf8mb4)
 AND NOT EXISTS (SELECT 1 FROM creature_text t WHERE t.entry=7804 AND t.groupid=0);
