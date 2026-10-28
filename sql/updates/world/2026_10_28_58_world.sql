-- Hillsbrad Foothills: Welcome to the Machine (28096) plays as in 5.4.8 (npc_hillsbrad_machine_skeletal_steed).
-- The Skeletal Steed at Southpoint Gate (47445) was a gossip NPC: clicking it opened an empty "hello" window and summoned a second
-- steed beside it, which only seated a player standing within 5 yd, and the three visitors came on a timer onto one spot. Now a
-- click casts Summon Skeletal Steed (88542), a vehicle summon that seats the player on the new steed, and the steed brings the
-- visitors one after another, with High Executor Darthalia's comments. Spells, spots and lines are 5.4.8's; see the script for what is ours.

-- The steed at the gate: click to ride, while the quest is in progress
UPDATE `creature_template` SET `npcflag`=16777216, `gossip_menu_id`=0, `AIName`='', `ScriptName`='' WHERE `entry`=47445;
DELETE FROM `smart_scripts` WHERE `entryorguid`=47445 AND `source_type`=0;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceEntry`=47445;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=47445;
INSERT INTO `npc_spellclick_spells` (`npc_entry`,`spell_id`,`cast_flags`,`user_type`) VALUES
(47445,88542,1,0);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=18 AND `SourceGroup`=47445;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(18,47445,88542,0,0,47,0,28096,8,0,0,0,0,'','Skeletal Steed - ride it while Welcome to the Machine is in progress');

-- The steed the player sits on runs the scene
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_hillsbrad_machine_skeletal_steed' WHERE `entry`=47488;
DELETE FROM `smart_scripts` WHERE `entryorguid`=47488 AND `source_type`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid`=4748800 AND `source_type`=9;

-- The player's own High Executor Darthalia only comments (the real one at the gate takes the quest back)
UPDATE `creature_template` SET `npcflag`=0 WHERE `entry`=47499;

-- Lines: Darthalia's comments and the visitors' arrivals (group 0 of a visitor stays his parting line). TextRange 5: the
-- speakers are the player's private summons.
DELETE FROM `creature_text` WHERE `CreatureID`=47499;
DELETE FROM `creature_text` WHERE `CreatureID` IN (47442,47443,47444) AND `GroupID` BETWEEN 1 AND 3;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`)
SELECT x.c, x.g, 0, IF(b.`Text`<>'', b.`Text`, b.`Text1`), 12, 0, 100, 0, 0, 0, 0, b.`ID`, 5, x.n
FROM (
    SELECT 47499 c, 0 g, 47658 bt, 'High Executor Darthalia - Welcome to the Machine, mounted' n UNION ALL
    SELECT 47499, 1, 47659, 'High Executor Darthalia - Welcome to the Machine, Dumass comes' UNION ALL
    SELECT 47499, 2, 47666, 'High Executor Darthalia - Welcome to the Machine, Dumass gone' UNION ALL
    SELECT 47499, 3, 47667, 'High Executor Darthalia - Welcome to the Machine, Orkus comes' UNION ALL
    SELECT 47499, 4, 47680, 'High Executor Darthalia - Welcome to the Machine, Orkus gone' UNION ALL
    SELECT 47499, 5, 47681, 'High Executor Darthalia - Welcome to the Machine, Johnny comes' UNION ALL
    SELECT 47444, 1, 47660, 'Dumass - Welcome to the Machine, arrives' UNION ALL
    SELECT 47444, 2, 47661, 'Dumass - Welcome to the Machine, arrives' UNION ALL
    SELECT 47443, 1, 47668, 'Kingslayer Orkus - Welcome to the Machine, arrives' UNION ALL
    SELECT 47443, 2, 47669, 'Kingslayer Orkus - Welcome to the Machine, arrives' UNION ALL
    SELECT 47443, 3, 47670, 'Kingslayer Orkus - Welcome to the Machine, arrives' UNION ALL
    SELECT 47442, 1, 47682, 'Johnny Awesome - Welcome to the Machine, arrives' UNION ALL
    SELECT 47442, 2, 47683, 'Johnny Awesome - Welcome to the Machine, arrives'
) x JOIN `broadcast_text` b ON b.`ID`=x.bt;
