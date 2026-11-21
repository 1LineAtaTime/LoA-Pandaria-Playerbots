-- Zen'Kiki 44269 (A Different Approach 26954): a script of his own so he follows the player to the hawks' hill.
-- Ours: with the spellclick npcflag and no AI he got NullCreatureAI, which never follows the summoner. Neither TDB gives him
-- an AI; the summon (Escorting Zen'Kiki 83456, SummonProperties 2882, ally) is a follower on retail.
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_wpl_zenkiki_hawk_helper' WHERE `entry`=44269;
