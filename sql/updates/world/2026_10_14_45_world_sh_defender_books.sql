-- #657 Scarlet Halls (normal): Scarlet Defender (Armsmaster Harlan's Call for Help adds) was level 90 with 1.58M HP on
-- DUNGEON_NORMAL; Wowhead npc/58998 lists "Level 31 - 93" (fetched 2026-10-14). Normal = Harlan's level 31; heroic keeps 90.
UPDATE `creature_template_difficulty` SET `level_min`=31, `level_max`=31 WHERE `id`=58998 AND `difficulty`='DUNGEON_NORMAL';
-- #657 Books (59209) in Flameweaver Koegler's room: MoP 5.4.8 flags (NOT_SELECTABLE, unit_flags2 2099200). Without them the
-- 71 level-86 books get AggressorAI and aggro the group; CIVILIAN (flags_extra 2) keeps them out of combat (beyond MoP 5.4.8).
UPDATE `creature_template` SET `unit_flags`=33554432, `unit_flags2`=2099200, `flags_extra`=`flags_extra`|2 WHERE `entry`=59209;
