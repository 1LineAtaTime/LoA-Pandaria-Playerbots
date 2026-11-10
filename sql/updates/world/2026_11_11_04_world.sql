-- Diseased Hawks 44481 (A Different Approach 26954): a script of their own so they never aggro until Zen'Kiki pulls one down.
-- Sourced: both TDBs give them UNIT_FLAG_IMMUNE_TO_PC (kept, unit_flags 33024); Wowhead: "you can't tame or attack them unless
-- he pulls them down"; Diseased Spit 81236 from the TDB SmartAI row.
-- Ours: SmartAI::MoveInLineOfSight attacks on sight in every react state but passive and this core ignores IMMUNE_TO_PC for the
-- attacker, so the defensive react state of 2026_11_11_03 changed nothing. npc_wpl_diseased_hawk ignores players while it carries
-- the flag, keeps Diseased Spit, and its SmartAI rows (spit, flag and react state on respawn/evade) are retired.
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_wpl_diseased_hawk' WHERE `entry`=44481;
DELETE FROM `smart_scripts` WHERE `entryorguid`=44481 AND `source_type`=0;
