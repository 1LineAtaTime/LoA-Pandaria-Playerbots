-- #695 (W1 lane, WoE Queen Azshara "You are mine now!"): the Hand of the Queen (54728) gets the new script
-- npc_queen_azshara_hand_of_the_queen (boss_queen_azshara.cpp, pW1.diff). Azshara puts Servant of the Queen 102334 on one player
-- (SET_VEHICLE_ID 1745 + AOE_CHARM + immunity) and the Hand rides the puppet's seat; the Hand's JustDied removes 102334, which
-- frees the puppet. Without this row the Hand has no script and killing it would leave the player charmed until Azshara resets.
-- Before: ScriptName '' (world, checked 2026-10-23); no `creature` spawn of 54728 (summon only).
-- Undo: UPDATE `creature_template` SET `ScriptName`='' WHERE `entry`=54728;
UPDATE `creature_template` SET `ScriptName`='npc_queen_azshara_hand_of_the_queen' WHERE `entry`=54728;
