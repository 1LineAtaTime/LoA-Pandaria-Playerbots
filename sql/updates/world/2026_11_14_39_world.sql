-- Eastern Plaguelands, Beat it Out of Them (27522): the Scourge Siege Engineers say their dying words instead of yelling
-- them (wiki Notes: "say"; no sniff of the type). Their credits are in npc_scourge_siege_engineer (same commit).
UPDATE `creature_text` SET `Type`=12 WHERE `CreatureID`=17878;
UPDATE `creature_template` SET `AIName`='',`ScriptName`='npc_scourge_siege_engineer' WHERE `entry`=17878;
