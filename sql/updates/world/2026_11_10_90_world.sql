-- Northridge Lumber Camp: the Hearthglen Mustang's Whinny for Learning the Ropes 27000 and This Means WAR 27001 (zone_western_plaguelands.cpp)
-- Sourced: 5.4.8 DBC 83605 Whinny (dummy, cone in front of the caster), 83621 Spider Slay (10000 damage, nearby entry); quest texts
-- ("Mount a Hearthglen Mustang ... use its Whinny ability to frighten 25 Northridge Spiders"); Wowhead comments: Whinny is the
-- horse's first button ("spam 1"); the spiders run away from the horse ("Use the whinny when behind the spiders"; "You 'push' em
-- toward the recruits"); a spider counts for 27001 when the Hearthglen Recruits kill it, for every player within about 40 yd.
-- Ours: Spider Slay limited to Northridge Spiders and Whinny to Northridge Spiders, the spider's AI in C++ (its SmartAI, Deadly
-- Poison 3583 at 5-8 s then every 15-18 s, kept), how far a Whinny pushes (15 yd), a spider counting once per 10 s for 27000.
UPDATE `creature_template` SET `spell1`=83605 WHERE `entry`=44832;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_wpl_northridge_spider' WHERE `entry`=44284;
DELETE FROM `smart_scripts` WHERE `entryorguid`=44284 AND `source_type`=0;

DELETE FROM `spell_script_names` WHERE `spell_id`=83605;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(83605,'spell_wpl_whinny');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry` IN (83605,83621);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,1,83605,0,0,31,0,3,44284,0,0,0,0,'','Whinny - only Northridge Spiders'),
(13,1,83621,0,0,31,0,3,44284,0,0,0,0,'','Spider Slay - only Northridge Spiders');
