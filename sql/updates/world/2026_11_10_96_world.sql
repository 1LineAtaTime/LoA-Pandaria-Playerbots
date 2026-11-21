-- The first battle for Andorhal (phase 259): each Desiccated Magus 44315 and Spearman 44316 now fights the soldier at its side.
-- Sourced: both TDBs (4.4.2, 12.1.0) and TCPP's 4.3.4 sniff spawn every one of the 54 Scourge within 1.7-4.7 yd of an
-- Andorhal Deathguard 44326 or Defender 44328, facing it; Wowhead's comment on The Battle for Andorhal 27159 has them "in
-- perpetual conflict until a player or abomination breaks this up" (the 85% sparring cap is in 2026_11_10_95). The sniffed
-- factions are 974 (Scourge, hostile to players only), 2213 and 2202 (hostile only to the other side), so on retail the
-- server must have started these fights itself; none of the TDBs' SmartAI or TCPP's scripts does.
-- Ours: this core lets two NPCs fight only when one is hostile to the other (Unit::IsValidAttackTarget, as TrinityCore and
-- TCPP), and static creatures never see each other move. So the Scourge take FactionTemplate 1988 from the 5.4.8 DBC: the
-- same Scourge faction 20, monster group, friends (20) and hostility to players as 974, plus hostility to Alliance and Horde
-- creatures (Blizzard's own Scourge-vs-Valiance template, used by Northrend Scourge such as Mindless Ghoul 27556). Players,
-- bots and pets see them exactly as before; the soldiers keep their templates and only fight back. Every few seconds out of
-- combat each of them attacks the Deathguard or Defender within 5 yd (target 19, closest alive of that entry), so the fights
-- stay the sniffed pairs. The mop-only Deathguards 44331 (immune to players, which this core's threat list also treats as
-- unattackable for NPCs) are never chosen, and neither are the Opengut Behemoths or Stickbone Berserkers (still 974).
UPDATE `creature_template` SET `faction`=1988 WHERE `entry` IN (44315,44316);

DELETE FROM `smart_scripts` WHERE `entryorguid` IN (44315,44316) AND `source_type`=0 AND `id` IN (2,3);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(44315,0,2,0,1,0,100,0,1000,3000,5000,8000,0,49,0,0,0,0,0,0,19,44326,5,0,0,0,0,0,0,'Desiccated Magus - Out of Combat - Attack the Andorhal Deathguard within 5 yd'),
(44315,0,3,0,1,0,100,0,1000,3000,5000,8000,0,49,0,0,0,0,0,0,19,44328,5,0,0,0,0,0,0,'Desiccated Magus - Out of Combat - Attack the Andorhal Defender within 5 yd'),
(44316,0,2,0,1,0,100,0,1000,3000,5000,8000,0,49,0,0,0,0,0,0,19,44326,5,0,0,0,0,0,0,'Desiccated Spearman - Out of Combat - Attack the Andorhal Deathguard within 5 yd'),
(44316,0,3,0,1,0,100,0,1000,3000,5000,8000,0,49,0,0,0,0,0,0,19,44328,5,0,0,0,0,0,0,'Desiccated Spearman - Out of Combat - Attack the Andorhal Defender within 5 yd');
