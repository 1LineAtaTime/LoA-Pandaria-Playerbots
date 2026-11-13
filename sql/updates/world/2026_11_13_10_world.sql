-- Western Plaguelands, Zen'Kiki, the Druid (26953) and Zen'Kiki and the Cultists (26955): Zen'Kiki 44904 no longer pulls every
-- hostile he passes and stays with a mounted player (npc_wpl_zenkiki_escort, C++; the catch-up is in spell_wpl_escorting_zenkiki).
--
--  * The script derives from SmartAI, so AIName and his smart_scripts rows (Mark of the Wild, Rejuvenation, Wrath, the form
--    antics) stay as they are.
--  * Ours: that he is defensive (joins his master's fights, defends himself, starts none), that he follows instead of fighting
--    while his master rides, and the 60 yd catch-up. No source describes his aggro behaviour on retail.

UPDATE `creature_template` SET `AIName` = 'SmartAI', `ScriptName` = 'npc_wpl_zenkiki_escort' WHERE `entry` = 44904;
