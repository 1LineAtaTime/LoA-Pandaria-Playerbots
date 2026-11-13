-- Deep Mine Rescue (28156, Hillsbrad Foothills): Dumass can be escorted out of the Azurelode Mine
--
-- The bug: Dumass (47681), webbed deep in the mine, gave the quest and nothing happened. He had no script and no texts, and the
-- quest had no objective and no event flag, so there was nothing to escort and nothing that completed it.
--
-- Retail (warcraft.wiki.gg, 4.0.3): accepting cuts him free, he follows the player out, attacks every webbed human he comes near
-- (which frees it, and the player kills it) and at Captain Keyton (47554) at the mine entrance he and Keyton talk and the quest
-- completes. No source scripts it (neither the retail TDB nor TCPP); npc_hillsbrad_dumass is ours (see its header comment).
--
-- The fix:
-- * npc_hillsbrad_dumass: the spawn stays webbed as the quest giver; accepting summons a free Dumass of the same entry that follows
--   the player, and the quest completes (GroupEventHappens) when he reaches Keyton. Talking to the webbed one again brings a new
--   escort to a player whose quest is still open but whose escort was lost (logout).
-- * SpecialFlags 2 (QUEST_SPECIAL_FLAGS_EXPLORATION_OR_EVENT): the quest waits for that event.
-- * The web (Paralytic Poison 87969) moves from the template addon to the spawn's addon, so the summoned Dumass is not stunned
--   (and evading, which reloads the addon, does not web him again).
-- * Lines from broadcast_text 48057-48075 with their emotes (EmotesID); which line goes with which moment is ours, except the
--   accept lines and the scene at Keyton, which the wiki gives.

UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_hillsbrad_dumass' WHERE `entry`=47681;
DELETE FROM `smart_scripts` WHERE `entryorguid`=47681 AND `source_type`=0;

UPDATE `creature_template_addon` SET `auras`='' WHERE `entry`=47681;
DELETE FROM `creature_addon` WHERE `guid`=176533;
INSERT INTO `creature_addon` (`guid`,`path_id`,`mount`,`MountCreatureID`,`StandState`,`AnimTier`,`VisFlags`,`SheathState`,`PvPFlags`,`emote`,`aiAnimKit`,`movementAnimKit`,`meleeAnimKit`,`visibilityDistanceType`,`auras`) VALUES
(176533,0,0,0,0,0,0,1,0,0,0,0,0,0,'87969');

DELETE FROM `quest_template_addon` WHERE `ID`=28156;
INSERT INTO `quest_template_addon` (`ID`,`SpecialFlags`) VALUES
(28156,2);

-- Type 12 say, 42 boss whisper to the player
DELETE FROM `creature_text` WHERE `CreatureID` IN (47681,47554);
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`)
SELECT x.c, x.g, x.i, b.`Text`, x.t, 0, 100, b.`EmotesID`, 0, 0, 0, b.`ID`, 0, x.n
FROM (
    SELECT 47681 c, 0 g, 0 i, 48057 bt, 12 t, 'Dumass - Deep Mine Rescue, accepted' n UNION ALL
    SELECT 47681, 1, 0, 48058, 42, 'Dumass - Deep Mine Rescue, accepted (to the player)' UNION ALL
    SELECT 47681, 2, 0, 48064, 12, 'Dumass - Deep Mine Rescue, following' UNION ALL
    SELECT 47681, 2, 1, 48066, 12, 'Dumass - Deep Mine Rescue, following' UNION ALL
    SELECT 47681, 2, 2, 48069, 12, 'Dumass - Deep Mine Rescue, following' UNION ALL
    SELECT 47681, 2, 3, 48071, 12, 'Dumass - Deep Mine Rescue, following' UNION ALL
    SELECT 47681, 2, 4, 48072, 12, 'Dumass - Deep Mine Rescue, following' UNION ALL
    SELECT 47681, 2, 5, 48073, 12, 'Dumass - Deep Mine Rescue, following' UNION ALL
    SELECT 47681, 2, 6, 48074, 12, 'Dumass - Deep Mine Rescue, following' UNION ALL
    SELECT 47681, 2, 7, 48075, 12, 'Dumass - Deep Mine Rescue, following' UNION ALL
    SELECT 47681, 3, 0, 48063, 12, 'Dumass - Deep Mine Rescue, goes for a webbed human' UNION ALL
    SELECT 47681, 3, 1, 48065, 12, 'Dumass - Deep Mine Rescue, goes for a webbed human' UNION ALL
    SELECT 47681, 3, 2, 48067, 12, 'Dumass - Deep Mine Rescue, goes for a webbed human' UNION ALL
    SELECT 47681, 3, 3, 48070, 12, 'Dumass - Deep Mine Rescue, goes for a webbed human' UNION ALL
    SELECT 47681, 4, 0, 48060, 12, 'Dumass - Deep Mine Rescue, at Captain Keyton' UNION ALL
    SELECT 47681, 5, 0, 48062, 12, 'Dumass - Deep Mine Rescue, leaves for Tarren Mill' UNION ALL
    SELECT 47681, 6, 0, 48059, 12, 'Dumass - Deep Mine Rescue, escort failed' UNION ALL
    SELECT 47554, 0, 0, 48061, 12, 'Captain Keyton - Deep Mine Rescue, Dumass is back'
) x JOIN `broadcast_text` b ON b.`ID`=x.bt;
