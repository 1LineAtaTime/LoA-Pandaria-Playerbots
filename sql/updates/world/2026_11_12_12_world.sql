-- Val'kyr Incursion 27201, Brother Against Brother 27202 and Aradne 27204: Thurman Grant's minutemen fight beside the player.
-- Sourced: quest texts ("bring some of these minutemen with you", "take some of my minutemen with you", "Bring some of Grant's
-- men with you"); Wowhead comments (2010-12-02) on 27202: "You get several farmers/militia to help you. They come complete
-- with a pet bar. Leave them on defensive and they'll attack whatever you attack", on 27204: "you get several militia just
-- like with the previous quest", and on 27201: "Since you'll likely be doing Brother Against Brother at the same time, you'll
-- have several farmers/militia with you"; the 5.4.8 DBC aura 84480 "Farmers' Militia" (dummy, nothing used it), the same as
-- Combat Training 84460 (attributes, duration, range) except AttributesEx7 0x4 (REACTIVATE_AT_RESURRECT, which this core does
-- not read), beside the Provincial Minuteman summons 84452 and 84456-84458.
-- Ours: 84480 runs the Combat Training companion script (spell_wpl_combat_training_minutemen: the four minutemen 45231-45234,
-- the first as a pet with a pet bar, the others joining his fights; one that falls 60 yards behind, or dies, is summoned again
-- on the next 5-second check, the pet only once its corpse has gone), and its bounds: Andorhal (area 193, where Thurman, Thassarian and every target stand) while any of the three
-- quests is in the log. The DBC also has 84482 "Summon Provincial Minuteman 1 (Guardian)", which would leave no pet bar; the
-- comments describe one, so the pet summon is kept.
DELETE FROM `spell_script_names` WHERE `spell_id`=84480;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(84480,'spell_wpl_combat_training_minutemen');

DELETE FROM `spell_area` WHERE `spell`=84480 AND `area`=193;
INSERT INTO `spell_area` (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`autocast`,`quest_start_status`,`quest_end_status`) VALUES
(84480,193,27201,0,0,0,2,1,10,11),
(84480,193,27202,0,0,0,2,1,10,11),
(84480,193,27204,0,0,0,2,1,10,11);
