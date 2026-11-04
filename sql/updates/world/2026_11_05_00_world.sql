-- Hillsbrad Foothills, Purgation Isle: the two Orkuses go with the player, as in 5.4.8. Kingslayer Orkus 48470 is the player's
-- companion on the isle from the end of The Road to Purgation (28375) until They Will Never Expect This... (28397) is turned in;
-- then he runs off, and the event Orkus 48503 runs to the player, gives Heroes of the Horde! (28400) and leads its event.
-- mop had a static 48470 on the beach instead, who also gave 28400 and started the event from his gossip.
-- Sourced: 5.4.8 DBC Purgation Isle Summon Controller I 90130 (castable only in area 896, Purgation Isle: area group 2852; a
-- dummy aura plus a 30 s periodic trigger of Orkus Broadcast 90129) with Summon Orkus 90132 (48470, SummonProperties 3045: ally
-- guardian, not personal, also area 896), and Purgation Isle Summon Controller II 90206 (castable in zone 267: area group 2822;
-- one dummy aura) with Summon Orkus 90205 (48503, SummonProperties 3081: personal ally). Neither TDB spawns either Orkus or has
-- spell_area for the controllers; retail TDB 1210 has only 48503 as the starter of 28400 (TDB 442 also lists 48470). Wowhead
-- comments: on 48470 (MoP Classic) "runs off and disappears when They Will Never Expect This... is turned in" and 48503 "Runs to
-- you and gives Heroes of the Horde!"; on 28397 "he acts like a pet during this section", "he evaporated about halfway to shore,
-- and didn't show up again till I was back on the island", "much more chatty than Dumass and Johnny Awesome"; on 28375 "Orkus
-- gives you a buff of 2500 health"; on 28400 "You have to talk to Orkus again to start the event", "When you get near the top
-- you will notice that Orkus slows down starting a scripted event ... After a short while you can talk to him". His lines are
-- broadcast_text 48698 .. 48719 as Wowhead quotes them for 48470, and 48757 "Tell me when you are ready and we will go!".
-- Ours: the quest bounds of both controllers, Controller II zone-wide (its area group), the 5 s checks, the chatter only out of
-- combat, the lines' type and personal range, where on the way up the event starts, the choice of 82061 for the buff.
-- The static stand-in (guid 3110070, 2026_10_28_42: ours, with Kasha's Apply Quest Invis Zone 4) goes.

DELETE FROM `creature_addon` WHERE `guid` = 3110070;
DELETE FROM `creature` WHERE `guid` = 3110070 AND `id` = 48470;

-- Controller I on Purgation Isle from 28375 complete until 28397 is rewarded; Controller II in Hillsbrad from 28397 rewarded
-- until 28400 is complete
DELETE FROM `spell_area` WHERE `spell` IN (90130, 90206);
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `autocast`, `quest_start_status`, `quest_end_status`) VALUES
(90130, 896, 28375, 28397, 0, 0, 2, 1, 66, 11),
(90206, 267, 28397, 28400, 0, 0, 2, 1, 64, 9);

DELETE FROM `spell_script_names` WHERE `spell_id` IN (90130, 90206);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(90130, 'spell_hillsbrad_purgation_isle_summon_controller'),
(90206, 'spell_hillsbrad_purgation_isle_summon_controller_heroes');

-- The turn-in of 28397 hands over from one Orkus to the other at once (spell_area does not follow quest changes here)
UPDATE `quest_template_addon` SET `ScriptName` = 'quest_hillsbrad_they_will_never_expect_this' WHERE `ID` = 28397;

-- 28400 comes from the event Orkus only (retail TDB 1210, Wowhead), and so does the event's "I'm ready" gossip (12440)
DELETE FROM `creature_queststarter` WHERE `id` = 48470 AND `quest` = 28400;
DELETE FROM `creature_queststarter` WHERE `id` = 48503 AND `quest` = 28400;
INSERT INTO `creature_queststarter` (`id`, `quest`) VALUES
(48503, 28400);
UPDATE `creature_template` SET `gossip_menu_id` = 0 WHERE `entry` = 48470;
UPDATE `creature_template` SET `gossip_menu_id` = 12440 WHERE `entry` = 48503;
-- The event Orkus follows the player through the zone, but the word to go up is only taken on Purgation Isle (ours; ANDed with
-- the option's quest condition, ElseGroup 0)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 12440 AND `SourceEntry` = 0 AND `ConditionTypeOrReference` = 23;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 12440, 0, 0, 0, 23, 0, 896, 0, 0, 0, 0, 0, '', 'Kingslayer Orkus: start option only on Purgation Isle');

-- Orkus Broadcast: the companion's chatter to his owner (TextRange 5: other players see someone else's Orkus, but his lines
-- name his owner)
DELETE FROM `creature_text` WHERE `CreatureID` = 48470 AND `GroupID` = 2;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(48470, 2, 0, 'This place reeks of evil. It must be purged!', 12, 0, 100, 0, 0, 0, 0, 48698, 5, 'Kingslayer Orkus - Purgation Isle companion, Orkus Broadcast'),
(48470, 2, 1, 'Worgen and dwarves working together... What has this world come to?', 12, 0, 100, 0, 0, 0, 0, 48702, 5, 'Kingslayer Orkus - Purgation Isle companion, Orkus Broadcast'),
(48470, 2, 2, 'I am sorry that I called you puny, $n. When we first met I thought you just another one of Darthalia''s pawns.', 12, 0, 100, 0, 0, 0, 0, 48705, 5, 'Kingslayer Orkus - Purgation Isle companion, Orkus Broadcast'),
(48470, 2, 3, '$n, you and I are friends.', 12, 0, 100, 0, 0, 0, 0, 48706, 5, 'Kingslayer Orkus - Purgation Isle companion, Orkus Broadcast'),
(48470, 2, 4, 'Garrosh Hellscream: great warchief or greatest warchief?', 12, 0, 100, 0, 0, 0, 0, 48708, 5, 'Kingslayer Orkus - Purgation Isle companion, Orkus Broadcast'),
(48470, 2, 5, 'Don''t get me wrong, $n, I like Thrall. He was a strong leader, but I don''t think he was cut out to lead the Horde. Not in a time of war!', 12, 0, 100, 0, 0, 0, 0, 48709, 5, 'Kingslayer Orkus - Purgation Isle companion, Orkus Broadcast'),
(48470, 2, 6, 'Did I ever tell you about the time I met Varok Saurfang? I asked him to sign my massive pectoral muscles. He backhanded me instead, which is why I sometimes forget things now. It was totally worth it.', 12, 0, 100, 0, 0, 0, 0, 48710, 5, 'Kingslayer Orkus - Purgation Isle companion, Orkus Broadcast'),
(48470, 2, 7, 'Varian Wrynn... I saw him fight once at Dire Maul. He killed five feral scar yetis with his bare hands. Pretty good for a human, no?', 12, 0, 100, 0, 0, 0, 0, 48711, 5, 'Kingslayer Orkus - Purgation Isle companion, Orkus Broadcast'),
(48470, 2, 8, 'I wasn''t there, but I heard that Sylvanas died in Silverpine and that her precious val''kyr resurrected her at the cost of their own lives. Know anything about it?', 12, 0, 100, 0, 0, 0, 0, 48712, 5, 'Kingslayer Orkus - Purgation Isle companion, Orkus Broadcast'),
(48470, 2, 9, 'If Ivar Bloodfang is here, where is Darius Crowley?', 12, 0, 100, 0, 0, 0, 0, 48713, 5, 'Kingslayer Orkus - Purgation Isle companion, Orkus Broadcast'),
(48470, 2, 10, 'I liked the worgen better when they were a disorganized mess that you''d run into in small packs. This new Gilnean worgen alliance is much more difficult to deal with.', 12, 0, 100, 0, 0, 0, 0, 48714, 5, 'Kingslayer Orkus - Purgation Isle companion, Orkus Broadcast'),
(48470, 2, 11, '$n, just between you and me, I never saw the fall of the Lich King. And... well... I''ve never been to Ulduar. I... well... I am a dungeon explorer. Yes. I prefer exploring dungeons with allies. The pay is excellent and the risk is low.', 12, 0, 100, 0, 0, 0, 0, 48715, 5, 'Kingslayer Orkus - Purgation Isle companion, Orkus Broadcast'),
(48470, 2, 12, 'Hillsbrad has changed. Back in my day Hillsbrad was a place of constant battle. The humans and dwarves of Southshore would constantly attack the Horde at Tarren Mill. There were days when you could look out past the intersection at Darrow Hill and see a thousand people battling. Glory days, $n - they''ll pass you by.', 12, 0, 100, 0, 0, 0, 0, 48716, 5, 'Kingslayer Orkus - Purgation Isle companion, Orkus Broadcast'),
(48470, 2, 13, 'I found Kasha in Icecrown. She was injured and dying. I managed to carry her back to the Shadow Vault where the death knights helped nurse her back to health. We are now the best of friends.', 12, 0, 100, 0, 0, 0, 0, 48717, 5, 'Kingslayer Orkus - Purgation Isle companion, Orkus Broadcast'),
(48470, 2, 14, 'I have heard tale of the Maelstrom itself opening up, threatening to devour the world. Could you imagine such a thing?', 12, 0, 100, 0, 0, 0, 0, 48718, 5, 'Kingslayer Orkus - Purgation Isle companion, Orkus Broadcast'),
(48470, 2, 15, 'If we stop these dwarves and worgen we will be celebrated as great heroes of the Horde!', 12, 0, 100, 0, 0, 0, 0, 48719, 5, 'Kingslayer Orkus - Purgation Isle companion, Orkus Broadcast');

-- The event Orkus waits for the player's word on the hilltop
DELETE FROM `creature_text` WHERE `CreatureID` = 48503 AND `GroupID` = 16;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(48503, 16, 0, 'Tell me when you are ready and we will go!|TInterface\\GossipFrame\\GOSSIPGOSSIPICON.blp:32|t', 12, 0, 100, 0, 0, 0, 0, 48757, 5, 'Kingslayer Orkus - Heroes of the Horde!, waits for the player''s word');
