-- #963 (lane d): Howling Gale (45572), The Vortex Pinnacle (map 657), Nimbus Rise.
--
-- INTENDED (sources read 2026-10-28):
--   * Spell 85084 "Howling Gale" (our Spell.dbc; Wowhead tooltip API nether.wowhead.com/cata/tooltip/spell/85084): "Forms a swirling
--     vortex that knocks away any enemies who stray too close." Periodic aura, 0.5 s amplitude, proc flags 0x222A8 (direct damage
--     taken: melee, ranged, spells; not periodic), proc chance 100. Knockbacks 85159 (strong, 17 yd: SpellRadius 84) and 85085
--     (weak, 5 yd: SpellRadius 8), both knockback 10/20 plus a 2.5 s dummy aura and TargetAuraStateNot 22.
--   * WoWManiaUK/Blackwing-Lair issue #2309 (2019-06-08/09): "attacking them with a direct attack should cause their spinning wind to
--     allow players to pass and not knock them"; fixed to "the blizzlike effect, you can hit the storms and it turns off their
--     knockback for a while". spp-issues/SPP-Legion-Issues #1093 (2026-08-09): "The intended mechanic is to nullify their knockback
--     mechanic with damage"; the gales regen HP there. Wowhead/Wowpedia have no further text (Wowpedia lists the NPC under Nimbus Rise).
--   * The Cataclysm Preservation Project (TrinityCore 4.3.4), src/server/scripts/Kalimdor/VortexPinnacle/vortex_pinnacle.cpp:129-137,
--     642-713 at commit 164289b; commits 9eeb4cc (2020-03-29, aura state vs repeated knockbacks), df60a4d (2020-09-28, "When damaging
--     a Howling Gale, the target will now stop casting any knockbacks for 10 seconds before slowly starting to build up again"; no
--     combat, RegenHealth 0), 1ae2b2f (curhealth 3000000), 8fc4b59 (2022-10-09, PreventDefaultAction). CPP TDB 434.22011 rows:
--     creature_template 45572 flags_extra 1073750272 (NO_COMBAT 0x2000 | IMMUNITY_KNOCKBACK 0x40000000), mechanic_immune_mask
--     2147483647, RegenHealth 0, ScriptName npc_vp_howling_gale; spell_script_names 85084 spell_vp_howling_gale, 85085/85159
--     spell_vp_howling_gale_knockback; spell_proc 85084 SpellTypeMask 1 (damage), Chance 100, Cooldown 10000; the same two spawns.
--
-- WHAT OUR SERVER DID: spell_howling_gale_howling_gale (85084) was bound here but its loader was commented out (startup error
--   "no core script found"), so the aura's core-corrected trigger spell 85159 (SpellMgr.cpp:4057-4062) knocked everyone within 10 yd
--   back every 0.5 s with no aura-state protection, its dummy aura effect searched the whole 533 yd cap, and npc_howling_gale cast
--   visual spells on a timer and removed the aura on the first hit until no player was within 30 yd.
--
-- CHANGE: CPP's implementation (the_vortex_pinnacle.cpp: npc_howling_gale, spell_vp_howling_gale, spell_vp_howling_gale_knockback):
--   strong 17 yd knockback every 0.5 s; direct damage silences it for 10 s, then 5 s of weak 5 yd knockbacks, then strong again.
--   This core has no spell_proc lookup for a periodic-trigger aura, so the proc comes from spell_proc_event (the DBC proc flags with
--   CPP's 10 s cooldown). Template flags/immunities/regen as CPP. Spawns unchanged (same two as CPP).
--
-- Idempotent. UNDO: DELETE the spell_script_names rows below and re-insert (85084,'spell_howling_gale_howling_gale'); DELETE the
--   spell_proc_event row 85084; UPDATE creature_template SET flags_extra=0, mechanic_immune_mask=0, RegenHealth=1 WHERE entry=45572.

-- 1) Spell scripts (CPP names).
DELETE FROM `spell_script_names` WHERE `spell_id` IN (85084,85085,85159) AND `ScriptName` IN ('spell_howling_gale_howling_gale','spell_vp_howling_gale','spell_vp_howling_gale_knockback');
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(85084,'spell_vp_howling_gale'),
(85085,'spell_vp_howling_gale_knockback'),
(85159,'spell_vp_howling_gale_knockback');

-- 2) 85084 procs on direct damage taken (DBC proc flags 0x222A8 = 139944), at most once per 10 s (CPP spell_proc Cooldown 10000).
DELETE FROM `spell_proc_event` WHERE `entry`=85084;
INSERT INTO `spell_proc_event` (`entry`,`SchoolMask`,`SpellFamilyName`,`SpellFamilyMask0`,`SpellFamilyMask1`,`SpellFamilyMask2`,`SpellFamilyMask3`,`procFlags`,`procEx`,`phase`,`ppmRate`,`CustomChance`,`Cooldown`) VALUES
(85084,0,0,0,0,0,0,139944,0,'PROC_PHASE_HIT',0,0,10000);

-- 3) Template as CPP: never enters combat, cannot be knocked back, immune to mechanics, does not regenerate (3,000,000 HP spawns).
UPDATE `creature_template` SET `flags_extra`=`flags_extra`|0x00002000|0x40000000, `mechanic_immune_mask`=2147483647, `RegenHealth`=0
WHERE `entry`=45572;
