-- #989 (Refs #989): Wailing Caverns: Naralex (3679) loses npc_disciple_of_naralex + gossip 201
-- Source: DeltaTho playerbots-fork PR 15, file 2026_10_26_17 (commit as in deltatho/playerbots-fork), ported as-is unless noted below.
-- Design check 2026-10-29: Wowpedia Wailing Caverns (MediaWiki API): the tauren druid Muyoh (Disciple of Naralex, 3678) starts the event and is escorted; Naralex is the sleeping druid. Wowhead tooltip: 3678 Muyoh 'Disciple of Naralex' (Elite), 3679 Naralex (Normal). ScriptName npc_disciple_of_naralex stays on Muyoh (wailing_caverns.cpp:78).
-- Idempotent: DELETE+INSERT / absolute UPDATEs.

-- Wailing Caverns: Naralex (3679) no longer runs the Disciple of Naralex's (Muyoh, 3678) script and gossip
-- npc_disciple_of_naralex is Muyoh's: on the sleeping Naralex it offered Muyoh's gossip (menu 201, "Naralex sleeps again!") and
-- "Let the event begin!", which started an escort of Naralex. The awakening event finds Naralex by entry in the instance script
-- and only makes him talk, stand, fly and move, which any AI does.
UPDATE `creature_template` SET `ScriptName` = '', `gossip_menu_id` = 0, `npcflag` = 0 WHERE `entry` = 3679;
