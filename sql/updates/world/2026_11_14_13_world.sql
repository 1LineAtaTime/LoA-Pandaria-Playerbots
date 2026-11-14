-- Eastern Plaguelands, Fiona's Caravan: the caravan buff the player chose comes back when he re-enters the zone.
--
--  * Before: each of the eight buffs (85612-85619, quests 27555-27562) is limited to the zone (AreaGroup 25) and went for good
--    when the player left it; he had to fetch it again at the wagon.
--  * Sourced: "the last buff you chose will be there when you enter the zone again" (Wowhead newhon63 [+3]; 39489, Duvvani
--    [+4]); the three hidden flag quests "[FLAG] EPL Zone Buff - Flag 1/2/3" 27552-27554 (tracking) that exist for it.
--  * Ours: how the flags are used (the buff's number 0-7 in their three bits; spell_epl_caravan_buff), and that it comes back on
--    entering the zone or logging in there (player_epl_caravan_buff).

DELETE FROM `spell_script_names` WHERE `spell_id` BETWEEN 85612 AND 85619 AND `ScriptName`='spell_epl_caravan_buff';
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(85612,'spell_epl_caravan_buff'),(85613,'spell_epl_caravan_buff'),(85614,'spell_epl_caravan_buff'),(85615,'spell_epl_caravan_buff'),
(85616,'spell_epl_caravan_buff'),(85617,'spell_epl_caravan_buff'),(85618,'spell_epl_caravan_buff'),(85619,'spell_epl_caravan_buff');
