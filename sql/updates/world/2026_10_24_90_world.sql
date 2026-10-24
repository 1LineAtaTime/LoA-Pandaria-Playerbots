-- The Botanica: creature abilities
-- Most of the dungeon's creatures (the Bloodwarder and Sunseeker blood elves, the Mutate Fleshlashers and Thorn Flayers, the Frayer
-- lashers) had no AI or ran classic rows: port their abilities from the classic (AzerothCore) data, falling back to the Cataclysm
-- data; rows needing events, actions or action lists LoA lacks are left out, and AzerothCore's range and area events are converted,
-- keeping the dungeon (normal/heroic) flags. The creatures no reference scripts cast their sniffed spells (creature_template
-- spell1-8).

SET @ENTRIES := '17994,18405,18422,18587,19505,19507,19509,19511,19513,19557,19598,19608';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(17994, 0, 0, 0, 0, 0, 100, 0, 4000, 6000, 22000, 25000, 11, 31567, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bloodwarder Falconer - In Combat - Cast ''Deterrence'''),
(17994, 0, 1, 0, 0, 0, 100, 0, 8000, 10000, 15000, 20000, 11, 34879, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bloodwarder Falconer - In Combat - Cast ''Multi-Shot'''),
(17994, 0, 2, 0, 0, 0, 100, 0, 8500, 10900, 14150, 21850, 11, 32908, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bloodwarder Falconer - In Combat - Cast ''Wing Clip'''),
(18405, 0, 0, 0, 0, 0, 100, 0, 7600, 16400, 15700, 25300, 11, 34793, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Tempest-Forge Peacekeeper - In Combat - Cast ''Arcane Blast'''),
(18405, 0, 1, 0, 0, 0, 100, 0, 9700, 16200, 7300, 19400, 11, 34791, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Tempest-Forge Peacekeeper - In Combat - Cast ''Arcane Explosion'''),
(18405, 0, 2, 0, 0, 0, 100, 0, 6050, 15000, 18250, 26700, 11, 34785, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Tempest-Forge Peacekeeper - In Combat - Cast ''Arcane Volley'''),
(18422, 0, 0, 0, 74, 0, 100, 0, 0, 60, 13300, 35100, 11, 34361, 0, 0, 0, 0, 0, 9, 0, 0, 40, 0, 0, 0, 0, 'Sunseeker Botanist - On Friendly Between 0-60% Health - Cast ''Regrowth'''),
(18422, 0, 1, 0, 0, 0, 100, 0, 8500, 20600, 15800, 20600, 11, 34350, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Botanist - In Combat - Cast ''Nature`s Rage'''),
(18587, 0, 0, 0, 1, 0, 100, 1, 500, 500, 0, 0, 11, 34201, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Frayer - Out of Combat - Cast Shift Form'),
(19505, 0, 0, 0, 1, 0, 100, 6, 1000, 1000, 30000, 30000, 11, 34173, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Channeler - Out of Combat - Cast ''Sunseeker Blessing'''),
(19505, 0, 1, 0, 0, 0, 100, 0, 7000, 10000, 20000, 24000, 11, 34222, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Channeler - In Combat - Cast ''Sunseeker Blessing'''),
(19505, 0, 2, 0, 0, 0, 100, 0, 7300, 15350, 18200, 25700, 11, 34634, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Channeler - In Combat - Cast ''Sunseeker Aura'''),
(19505, 0, 3, 0, 0, 0, 100, 0, 2000, 10000, 15000, 24000, 11, 34637, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Channeler - In Combat - Cast ''Soul Channel'''),
(19505, 0, 4, 0, 38, 0, 100, 0, 1, 1, 0, 0, 11, 34156, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Channeler - On Data Set 1 1 - Cast ''Crystal Channel'''),
(19507, 0, 2, 3, 2, 0, 100, 7, 0, 50, 0, 0, 12, 19598, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Gene-Splicer - Between 0-50% Health - Summon Creature ''Mutate Fleshlasher'' (No Repeat) (Dungeon)'),
(19507, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 12, 19598, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Gene-Splicer - Between 0-50% Health - Summon Creature ''Mutate Fleshlasher'' (No Repeat) (Dungeon)'),
(19507, 0, 4, 0, 0, 0, 100, 0, 8500, 13350, 19350, 27650, 11, 34642, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Gene-Splicer - In Combat - Cast ''Death & Decay'''),
(19507, 0, 5, 0, 0, 0, 100, 0, 21850, 25500, 35000, 35000, 11, 34247, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Gene-Splicer - In Combat - Cast ''Summon Lasher Beast'''),
(19509, 0, 2, 3, 2, 0, 100, 7, 0, 50, 0, 0, 12, 19598, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Harvester - Between 0-50% Health - Summon Creature ''Mutate Fleshlasher'' (No Repeat) (Dungeon)'),
(19509, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 12, 19598, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Harvester - Between 0-50% Health - Summon Creature ''Mutate Fleshlasher'' (No Repeat) (Dungeon)'),
(19509, 0, 4, 0, 0, 0, 100, 0, 6050, 19500, 7300, 23100, 11, 34640, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Harvester - In Combat - Cast ''Wilting Touch'''),
(19509, 0, 5, 0, 0, 0, 100, 0, 6100, 14450, 16950, 46200, 11, 34639, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 'Sunseeker Harvester - In Combat - Cast ''Polymorph'''),
(19511, 0, 0, 0, 0, 0, 100, 0, 6100, 19900, 20000, 24000, 11, 34616, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Nethervine Inciter - In Combat - Cast ''Deadly Poison'''),
(19511, 0, 1, 0, 0, 0, 100, 0, 4300, 9700, 21700, 38600, 11, 34615, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Nethervine Inciter - In Combat - Cast ''Mind-numbing Poison'''),
(19511, 0, 2, 0, 0, 0, 100, 0, 13400, 20600, 20600, 29000, 11, 30621, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Nethervine Inciter - In Combat - Cast ''Kidney Shot'''),
(19511, 0, 3, 0, 8, 0, 100, 0, 34222, 0, 1200, 1200, 11, 34173, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Nethervine Inciter - On Spellhit ''Sunseeker Blessing'' - Cast ''Sunseeker Blessing'''),
(19511, 0, 4, 0, 8, 0, 100, 0, 34200, 0, 1200, 1200, 11, 34173, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Nethervine Inciter - On Spellhit ''Crystal Channel'' - Cast ''Sunseeker Blessing'''),
(19513, 0, 0, 0, 0, 0, 100, 0, 8000, 24000, 28000, 36000, 11, 30584, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Mutate Fear-Shrieker - In Combat - Cast ''Fear'''),
(19557, 0, 0, 0, 0, 0, 100, 0, 3950, 12050, 1200, 13600, 11, 34644, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Greater Frayer - In Combat - Cast ''Lash'''),
(19598, 0, 0, 0, 0, 0, 100, 0, 4850, 13350, 1100, 17900, 11, 34351, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Mutate Fleshlasher - In Combat - Cast ''Vicious Bite'''),
(19608, 0, 0, 0, 0, 0, 100, 0, 3850, 11250, 1050, 14350, 11, 34644, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Frayer Wildling - In Combat - Cast ''Lash''');
