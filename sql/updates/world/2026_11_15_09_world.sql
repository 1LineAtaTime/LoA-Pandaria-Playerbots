-- Eastern Plaguelands, Tyr's Hand: Rohan the Assassin 46095 (The Assassin 27613) fights back when he is hit.
-- Sourced: he waits stealthed in the bell-tower attic and "won't attack till you do, even if you run straight through him"
-- (Wowhead comments 2010-12-30 [+42], 2010-12-20 [+4]); he is the quest's kill target, so once attacked he fights; "Die!"
-- (broadcast_text 50341) on aggro.
-- Fault: he rests REACT_PASSIVE (row 8) and was meant to wake on his On Aggro rows 5-7. A spell, ranged shot, DoT or pet
-- opener never reached them: the damage adds threat first (Unit::DealDamage -> ThreatManager::AddThreat), CombatManager puts
-- him in combat without calling the AI, and his threat entry is created offline because Creature::CanCreatureAttack asks
-- SmartAI::CanAIAttack, which refuses everything while passive. No engagement, so no On Aggro; the later Unit::CombatStart
-- finds him already in combat. He stood passive and in combat, neither fighting nor evading.
-- Ours: row 9 makes him aggressive on the first damage he takes, in Unit::DealDamage before the threat is added; the threat
-- entry then comes online, he engages and rows 5-7 run as designed (yell, aggressive, attack the engager). Not repeatable
-- within one life or fight (OnReset clears it at evade and respawn). Sap and other damage-free openers still leave him
-- waiting, as "you can sap him while stealthed" says.
-- What the player sees: hit Rohan with anything and he yells "Die!" and fights with Backstab, Deadly Throw and Hemorrhage.

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 46095;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 46095 AND `source_type` = 0 AND `id` = 9;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(46095, 0, 9, 0, 32, 0, 100, 1, 0, 2147483647, 0, 0, 0, 8, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rohan the Assassin - On Damaged - Set React Aggressive (no repeat)');
