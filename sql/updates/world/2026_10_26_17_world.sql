-- Wailing Caverns: Naralex (3679) no longer runs the Disciple of Naralex's (Muyoh, 3678) script and gossip
-- npc_disciple_of_naralex is Muyoh's: on the sleeping Naralex it offered Muyoh's gossip (menu 201, "Naralex sleeps again!") and
-- "Let the event begin!", which started an escort of Naralex. The awakening event finds Naralex by entry in the instance script
-- and only makes him talk, stand, fly and move, which any AI does.
UPDATE `creature_template` SET `ScriptName` = '', `gossip_menu_id` = 0, `npcflag` = 0 WHERE `entry` = 3679;
