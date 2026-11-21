-- The first battle for Andorhal (phase 259): the Scourge and the faction soldiers fight without end until a player joins in.
-- Sourced: Wowhead comment on The Battle for Andorhal 27159: both sides "are set not to deal damage to them beyond ~80% of
-- their life ... in perpetual conflict until a player or abomination breaks this up". That is retail's "no NPC damage below
-- 85%" rule (TCPP's CREATURE_STATIC_FLAG_4_NO_NPC_DAMAGE_BELOW_85PTC, TDB 12.1.0's creature_template_sparring), which this
-- core has as creature_sparring_template: Unit::DealDamage caps damage only from NPCs not owned by a player, so players,
-- their pets and companions kill normally. Both TDBs and TCPP's 4.3.4 sniff give the Desiccated Magus 44315 and Spearman
-- 44316 no UNIT_FLAG_IMMUNE_TO_NPC (mop kept it, so they never fought), and give the soldiers unit_flags 0 where mop has
-- UNIT_FLAG_UNK_29 (0x20000000, the feign-death flag) on 44328/44331 and UNIT_FLAG_RENAME (0x10) on 44331/44332.
-- Ours: the per-entry sparring rows (no source lists Andorhal's entries; 85 is the flag's value, the comment's "~80%" a rough
-- reading), and keeping UNIT_FLAG_IMMUNE_TO_PC on 44331 (TDBs 0, but TCPP's static flags have it), UNIT_FLAG_CAN_SWIM (0x8000,
-- in TCPP's sniff) on all.
DELETE FROM `creature_sparring_template` WHERE `creature_id` IN (44315,44316,44326,44328,44331,44332);
INSERT INTO `creature_sparring_template` (`creature_id`,`health_limit_pct`,`comment`) VALUES
(44315,85,'Desiccated Magus - Battle for Andorhal'),
(44316,85,'Desiccated Spearman - Battle for Andorhal'),
(44326,85,'Andorhal Deathguard - Battle for Andorhal'),
(44328,85,'Andorhal Defender - Battle for Andorhal'),
(44331,85,'Andorhal Deathguard - Battle for Andorhal'),
(44332,85,'Andorhal Defender - Battle for Andorhal');

UPDATE `creature_template` SET `unit_flags`=0 WHERE `entry`=44315;
UPDATE `creature_template` SET `unit_flags`=32768 WHERE `entry` IN (44316,44326,44328,44332);
UPDATE `creature_template` SET `unit_flags`=33024 WHERE `entry`=44331;
