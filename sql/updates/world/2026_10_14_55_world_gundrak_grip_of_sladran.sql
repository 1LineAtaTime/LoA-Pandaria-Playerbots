-- #556 Gundrak Slad'ran: Snake Wrap (29742) never appeared on MoP. Port AC's AuraScript on Grip of Slad'ran (55093):
-- AC src/server/scripts/Northrend/Gundrak/boss_slad_ran.cpp spell_sladran_grip_of_sladran_aura, AC spell_script_names 55093.
-- 5.4.8 DBC: 55093 = 5 stacks, 10 s, PERIODIC_DUMMY 1 s tick; 55126 = stun + transform + SUMMON 29742 (10 s).
DELETE FROM `spell_script_names` WHERE `spell_id`=55093;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(55093, 'spell_sladran_grip_of_sladran');
