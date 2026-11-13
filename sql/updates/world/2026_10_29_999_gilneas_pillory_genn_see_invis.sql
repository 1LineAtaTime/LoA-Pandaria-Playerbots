-- Refs #999. 14375 "Last Chance at Humanity": King Genn Greymane 36332 (guid 375757) is invisible to the cursed player, so the
-- quest cannot be taken. Goes with the zone_gilneas.cpp change of the same issue (Krennan 36331 and Godfrey 36330, the
-- player's private summons, get the same aura in npc_krennan_aranas_pillory).
--
-- Cause: SQL 2026_10_09_12 casts 68630 Curse of the Worgen in area 4786. Its linked aura 68631 is SPELL_AURA_MOD_INVISIBILITY,
-- type 9, amount 100 (Spell/SpellEffect.dbc 5.4.8). WorldObject::CanDetectInvisibilityOf (Object.cpp:2468-2471, upstream
-- c6c7d32e) lets an invisible seer see a unit only if that unit detects the seer's invisibility types. AC
-- Object.cpp:2016-2045 has the same rule ("it isn't possible in invisibility to detect something that can't detect the
-- invisible object"). Genn has no detection aura, so the cursed player cannot see him.
-- Retail/TDB 4.3.4 had Genn as the player's own summon (68636 -> 68634 "Summon Personal Greymane", SummonProperties 3048),
-- and the fork keeps the static reference spawn 375757 instead (zone_gilneas.cpp #504 notes).
-- Live evidence (MoP test server v15, 2026-10-29, character Gilnsee, a headless client, at the pillory with the curse on):
--   cansee 36332 samephase=1 seen=0; after `.botscenario Gilnsee npcaura 36332 60922` seen=1 (Krennan and Godfrey: same, 0 -> 1).
--
-- Fix: give 375757 the aura 60922 "Generic Quest Invisibility Detection 3" (detect type 9, amount 100, permanent). The same
-- aura is already a template aura on 55786/64505-64507 in this DB. Nothing else changes for players who are not cursed.
--
-- Backup (state before this file): creature_addon guid 375757 = no row
-- (not shipped), 0 INSERT rows).
-- Rollback:
--   DELETE FROM `creature_addon` WHERE `guid` = 375757;
-- Re-runnable: DELETE-then-INSERT.

DELETE FROM `creature_addon` WHERE `guid` = 375757;
INSERT INTO `creature_addon` (`guid`, `path_id`, `mount`, `MountCreatureID`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(375757, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '60922');
