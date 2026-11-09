-- Andorhal: leave the Ebon Gargoyles and Araj's Phylactery to the spells that summon them
-- Sourced: Spell.dbc 83994 "Summon Ebon Gargoyle" (SUMMON 44955), force-cast on the Provincial Minutemen by 84009 "Release
-- Ebon Gargoyle", the spell of The Farmers' Militia's collar 60815 (its other effect, 84289, gives the 44955 credit); 18661
-- "Araj's Phylactery" (SUMMON_OBJECT 177241), cast by Araj the Summoner 1852 on death (his SmartAI). Neither TDB spawns
-- a single Ebon Gargoyle. The six here stood on Provincial Minuteman spots, three of them flying a four-point path: copies of
-- the summon. The static Phylactery stood by Araj's spot, so its shard could be looted without killing him. Wowhead comments
-- (27164): one phylactery per Araj kill ("killing him three times because he respawned before I could click on the Phylactery.
-- But then I had 3 phylacteries to choose from"; "After you kill him, wait for 5-10 seconds for the phylactery to spawn").
-- Araj's death row cast 18661 at no target (target_type 0), which SmartAI skips, so it never spawned; it now casts on himself
-- (18661 summons at DEST_CASTER; a triggered cast leaves a dead caster, Spell::CheckCast). The summon lasts 300 s.
-- Ours: the Phylactery verdict (both TDBs keep a spawn there, which we read as the sniffed death summon) and the self target.
DELETE FROM `waypoint_data` WHERE `id` IN (312095,312096,312099);
DELETE FROM `creature_addon` WHERE `guid` BETWEEN 312094 AND 312099;
DELETE FROM `creature` WHERE `guid` BETWEEN 312094 AND 312099 AND `id`=44955;
DELETE FROM `gameobject` WHERE `guid`=165913 AND `id`=177241;
UPDATE `smart_scripts` SET `target_type`=1, `target_param1`=0 WHERE `entryorguid`=1852 AND `source_type`=0 AND `id`=3 AND `action_type`=11 AND `action_param1`=18661;
