-- #826 / #681 Deadmines (map 36), heroic: Vanessa's Nightmare Gauntlet, Vanessa's detonations / ropes, Glubtok's Fire Wall.
-- DRAFT - not applied. The PM applies it to the test `world` DB with a backup. It needs the matching build (the script names exist
-- only from that build on); creature_template.ScriptName and conditions are read at startup, so restart the worldserver after it.
--
-- 1) Two creatures bound to the scripts added in src/server/scripts/EasternKingdoms/Deadmines/boss_vanessa_vancleef.cpp (port of
--    The-Cataclysm-Preservation-Project TrinityCore 4.3.4 e503f96: npc_vanessa_rope, npc_deadmines_steam_valve):
--    49550 Rope (VehicleId 1399, npc_spellclick_spells 46598 Ride Vehicle): npc_dm_vanessa_rope swings the rider off the ship and
--          back onto the deck after Vanessa's 50% / 25% detonations (PassengerBoarded -> MoveSmoothPath, despawn 8.9 s).
--    49457 Steam Valve (npcflag SPELLCLICK): npc_dm_steam_valve counts a click on one of the four valves Vanessa's trap summons. The
--          four DB spawns of 49457 (326856, 326858-326860, spawnMask 6) keep their template UNIT_FLAG_NOT_SELECTABLE and are ignored.
--    Before (world, checked 2026-10-23): ScriptName '' for both entries.
-- 2) Players-only implicit targets (conditions source 13 SPELL_IMPLICIT_TARGET, type 32 CONDITION_TYPE_MASK, value 16 =
--    TYPEMASK_PLAYER; SourceGroup = effect mask). Each of these spells is TARGET_UNIT_SRC/DEST_AREA_ENTRY (5.4.8 SpellEffect.dbc
--    target B 7 / 8) and this DB has no conditions row for any of them, so they hit every unit in range (Spell.cpp
--    WorldObjectSpellTargetCheck: TARGET_CHECK_ENTRY without conditions = any unit): the Fire Wall would burn Glubtok, Fiery Blaze
--    Vanessa's adds, the icicles and flames the nightmare bosses, Nightmare Slow (-75%) / Adrenaline (+50%) every creature on the map,
--    Noxious Concoction would stun the boss. The intended targets are the players (Wowpedia "Glubtok", "Vanessa VanCleef (tactics)",
--    MediaWiki API 2026-10-23); TrinityCore marks such spells the same way (e.g. this DB's 43468 "Spell 43468 target player").
--      91397 Fire Wall (eff 0)          93485 Fiery Blaze (eff 0)        92171 Nightmare Flames (eff 0)   92278 Spark (eff 0)
--      95520 Spark, platter (eff 0)     92425 Molten Magma (eff 0)       92203 Ice Shards (eff 0-2)       92559 Nightmare Slow (eff 0)
--      92563 Nightmare Aura (eff 0)     92113 Nightmare Elixir (eff 0)   92120 Vanessa's Blackout (eff 0)  92100 Noxious Concoction (eff 0)
--      92604 Adrenaline (eff 0)
--    Before (world, checked 2026-10-23): no conditions rows with SourceEntry in this list.
-- Undo:
--   UPDATE `creature_template` SET `ScriptName` = '' WHERE `entry` IN (49550, 49457);
--   DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `ConditionTypeOrReference` = 32 AND `SourceEntry` IN
--     (91397, 93485, 92171, 92278, 95520, 92425, 92203, 92559, 92563, 92113, 92120, 92100, 92604);
UPDATE `creature_template` SET `ScriptName` = 'npc_dm_vanessa_rope' WHERE `entry` = 49550;
UPDATE `creature_template` SET `ScriptName` = 'npc_dm_steam_valve' WHERE `entry` = 49457;

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `ConditionTypeOrReference` = 32 AND `SourceEntry` IN
  (91397, 93485, 92171, 92278, 95520, 92425, 92203, 92559, 92563, 92113, 92120, 92100, 92604);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`,
  `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 91397, 0, 0, 32, 0, 16, 0, 0, 0, 0, 0, '', 'DM Glubtok Fire Wall - players only (#681)'),
(13, 1, 93485, 0, 0, 32, 0, 16, 0, 0, 0, 0, 0, '', 'DM Vanessa Fiery Blaze - players only (#681)'),
(13, 1, 92171, 0, 0, 32, 0, 16, 0, 0, 0, 0, 0, '', 'DM Nightmare Flames - players only (#826)'),
(13, 1, 92278, 0, 0, 32, 0, 16, 0, 0, 0, 0, 0, '', 'DM Spark (stalker) - players only (#826)'),
(13, 1, 95520, 0, 0, 32, 0, 16, 0, 0, 0, 0, 0, '', 'DM Spark (platter) - players only (#826)'),
(13, 1, 92425, 0, 0, 32, 0, 16, 0, 0, 0, 0, 0, '', 'DM Molten Magma (trap) - players only (#826)'),
(13, 7, 92203, 0, 0, 32, 0, 16, 0, 0, 0, 0, 0, '', 'DM Ice Shards (icicle) - players only (#826)'),
(13, 1, 92559, 0, 0, 32, 0, 16, 0, 0, 0, 0, 0, '', 'DM Nightmare Slow - players only (#826)'),
(13, 1, 92563, 0, 0, 32, 0, 16, 0, 0, 0, 0, 0, '', 'DM Nightmare Aura - players only (#826)'),
(13, 1, 92113, 0, 0, 32, 0, 16, 0, 0, 0, 0, 0, '', 'DM Nightmare Elixir - players only (#826)'),
(13, 1, 92120, 0, 0, 32, 0, 16, 0, 0, 0, 0, 0, '', 'DM Vanessa''s Blackout Aura - players only (#826)'),
(13, 1, 92100, 0, 0, 32, 0, 16, 0, 0, 0, 0, 0, '', 'DM Noxious Concoction - players only (#826)'),
(13, 1, 92604, 0, 0, 32, 0, 16, 0, 0, 0, 0, 0, '', 'DM Adrenaline - players only (#826)');
