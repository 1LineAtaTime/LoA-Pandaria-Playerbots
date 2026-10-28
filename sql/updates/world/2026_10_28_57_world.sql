-- Cities in Dust: Song of Sylvanas keeps the Battlefront off the player who marches with her (spell_silverpine_song_of_sylvanas).
DELETE FROM `spell_script_names` WHERE `spell_id`=85874 AND `ScriptName`='spell_silverpine_song_of_sylvanas';
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(85874,'spell_silverpine_song_of_sylvanas');
