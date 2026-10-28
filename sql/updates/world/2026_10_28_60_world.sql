-- Hillsbrad Foothills: Welcome to the Machine (28096), the full 5.4.8 conversations, the visitors' arrivals and departures, and
-- the Quest Giver aura (npc_hillsbrad_machine_visitor, spell_hillsbrad_machine_quest_giver; follows 2026_10_28_58).
-- Each visitor had one gossip option that handed him his quest at once. Now each is talked into it through his windows as on
-- retail (gossip_menu 12272-12279 and their npc_text are 5.4.8's; the options are the retail TDB's, with the windows they lead
-- to by their text): Dumass 12273 -> 12272, Kingslayer Orkus 12276 -> 12275 -> 12274, Johnny Awesome 12279 -> 12278 -> 12277.
-- The option of the last window gives the credit; the script then has him say his parting line and leave.

-- The windows
DELETE FROM `gossip_menu_option` WHERE `MenuID` BETWEEN 12272 AND 12279;
INSERT INTO `gossip_menu_option` (`MenuID`,`OptionID`,`OptionIcon`,`OptionText`,`OptionBroadcastTextID`,`OptionType`,`OptionNpcflag`,`ActionMenuID`,`ActionPoiID`,`BoxCoded`,`BoxMoney`,`BoxText`,`BoxBroadcastTextID`,`VerifiedBuild`)
SELECT x.m, 0, 0, b.`Text`, b.`ID`, 1, 1, x.a, 0, 0, 0, NULL, 0, 0
FROM (
    SELECT 12273 m, 47662 bt, 12272 a UNION ALL     -- Dumass: "Maybe you should go take a nap..." -> "OK! THANKS! I'LL JUST WAIT HERE..."
    SELECT 12272, 47664, 0 UNION ALL                -- "Wait a minute... head southeast to the Azurelode Mine..." (credit)
    SELECT 12276, 47674, 12275 UNION ALL            -- Orkus: "Um... some trouble at Southshore..." -> "Insurgents you say?..."
    SELECT 12275, 47676, 12274 UNION ALL            -- "None." -> "Then Orkus WILL DO IT! YES!"
    SELECT 12274, 47678, 0 UNION ALL                -- "Right, thanks. Just go ahead and head southeast..." (credit)
    SELECT 12279, 47684, 12278 UNION ALL            -- Johnny: "...problems at the Sludge Fields..." -> "Fine, fine, what else?"
    SELECT 12278, 47686, 12277 UNION ALL            -- "That's all." -> "That's all? One quest? Surely you jest..."
    SELECT 12277, 47688, 0                          -- "THAT'S ALL, JOHNNY AWESOME. TAKE IT OR LEAVE IT!" (credit)
) x JOIN `broadcast_text` b ON b.`ID`=x.bt;

-- The visitors: the script replaces their SmartAI (credit at once, gone 2 s later)
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_hillsbrad_machine_visitor' WHERE `entry` IN (47442,47443,47444);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (47442,47443,47444) AND `source_type`=0;

-- Their lines: group 0 the parting line, from group 1 the arrival lines. The greetings 47661 (Dumass) and 47683 (Johnny) are the
-- npc_text of their first windows (17239, 17244), so they are no longer said as well. TextRange 5: the speakers are the player's
-- private summons.
DELETE FROM `creature_text` WHERE `CreatureID` IN (47442,47443,47444);
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`)
SELECT x.c, x.g, 0, b.`Text`, 12, 0, 100, x.e, 0, 0, 0, b.`ID`, 5, x.n
FROM (
    SELECT 47444 c, 0 g, 47665 bt, 3 e, 'Dumass - Welcome to the Machine, leaves' n UNION ALL
    SELECT 47444, 1, 47660, 0, 'Dumass - Welcome to the Machine, arrives' UNION ALL
    SELECT 47443, 0, 47679, 25, 'Kingslayer Orkus - Welcome to the Machine, leaves' UNION ALL
    SELECT 47443, 1, 47668, 0, 'Kingslayer Orkus - Welcome to the Machine, arrives' UNION ALL
    SELECT 47443, 2, 47669, 0, 'Kingslayer Orkus - Welcome to the Machine, arrives' UNION ALL
    SELECT 47443, 3, 47670, 0, 'Kingslayer Orkus - Welcome to the Machine, arrives' UNION ALL
    SELECT 47442, 0, 47689, 1, 'Johnny Awesome - Welcome to the Machine, leaves' UNION ALL
    SELECT 47442, 1, 47682, 0, 'Johnny Awesome - Welcome to the Machine, arrives'
) x JOIN `broadcast_text` b ON b.`ID`=x.bt;

-- Quest Giver (88476): cast by the player once seated, it brings the player's own Darthalia (88563); Cancel Questgiver (88574)
-- ends it with the scene and she leaves with it. Not saved: a logout mid-scene must not keep it.
DELETE FROM `spell_script_names` WHERE `spell_id`=88476;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(88476,'spell_hillsbrad_machine_quest_giver');
DELETE FROM `spell_custom_attribute` WHERE `spell`=88476;
INSERT INTO `spell_custom_attribute` (`spell`,`attribute`,`comment`) VALUES
(88476,'SPELL_ATTR0_CU_DONT_SAVE_AURA_TO_DB','Quest Giver - Welcome to the Machine, ends with the scene');
