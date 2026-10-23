-- Refs #337
-- Monk class trainers: 17 of the 19 (trainer_class 10) have gossip_menu_id 0, so talking to them opens no menu at all.
--
-- #337 as filed ("every class-10 trainer has zero npc_trainer rows") is NOT a bug: since patch 5.0.4 class abilities are
-- learned automatically on reaching the level (Wowpedia "Trainer", Patch changes: "Class abilities are now learnt
-- automatically by characters upon reaching the required level, without visiting trainers. Class trainers can now
-- separately reset characters' specialization, talents and glyphs."); the fork does it in Player::LearnSkillRewardedSpells
-- (Player.cpp:25760, GetLevel() < SpellLevel gate) and LearnSpecializationSpells (Player.cpp:25648). MoP 5.4.8 has the
-- same data (all trainer_class 10 NPCs npcflag 1/3, no trainer flag 16, no npc_trainer rows), and so does every other
-- class in LoA (only trainer_class 3 and 6 carry flag 16).
--
-- What IS missing: the "class trainer" menu. 9 of the 11 classes have a gossip menu on ~85-95% of their trainers with the
-- 5.0.4 options (OptionType 16-20: reset Class Specialization / Talents / Glyphs, Dual Specialization, Proving Grounds).
-- Monk has it on 2 of 19 (Aysa Cloudsinger 62419 menu 10373, Ji Firepaw 62445 menu 14248; same in MoP 5.4.8). The other 17
-- are the capital-city Monk trainers (Brewmaster Tzu, Ao Jian, Liu, Lanfen, Tseh Rah, Sho the Wise, Pan, Xi, Bao, Ting,
-- Lo, Gato, Zabrax, Shoyu, Laoxi, Pao, Mojo Stormstout).
--
-- Fix: give them Ji Firepaw's menu 14248 (same five 5.0.4 options; text 19888 "I have nothing to teach you, $c.", which is
-- the intended 5.0.4 behaviour of a trainer). Retail uses a per-NPC menu id whose text is not in either DB, so one shared
-- menu is used (menu rows are shared by many creatures elsewhere, e.g. 7263 by 7 warriors). Only rows still at 0 change.
--
-- VERIFY: SELECT entry, gossip_menu_id FROM creature_template WHERE trainer_class = 10;   -- all 14248 / 10373 / 14248
--   live: headless gossip-hello on Sho the Wise (62463, Eversong) lists "How do I reset my Class Specialization?" ...
-- ROLLBACK: UPDATE `creature_template` SET `gossip_menu_id` = 0 WHERE `entry` IN (62321, 62393, 62425, 62450, 62462, 62463, 62464, 63238, 63258, 63272, 63285, 63296, 63310, 63327, 63331, 63332, 63335);

UPDATE `creature_template` SET `gossip_menu_id` = 14248
WHERE `trainer_class` = 10 AND `gossip_menu_id` = 0
  AND `entry` IN (62321, 62393, 62425, 62450, 62462, 62463, 62464, 63238, 63258, 63272, 63285, 63296, 63310, 63327, 63331, 63332, 63335);
