-- Western Plaguelands: Zen'Kiki, the Druid counts all four kinds of wildlife.
-- The objective is 20 x credit 44777 "Wildlife killed with Zen'Kiki". Only Hulking Plaguebear 44482 gave it (its SmartAI id 1, on death,
-- to the killing blow); Diseased Wolves 1817, Venom Mist Lurkers 1822 and Plague Lurkers 1824 gave nothing.
-- Sourced: the quest's objective text (mop and Wowhead MoP Classic), "Kill 20 plagued wildlife with Zen'Kiki by your side. You may kill
-- Hulking Plaguebears, Diseased Wolves, Venom Mist Lurkers, or Plague Lurkers.", and the DBC credit spell 83484 "The Worst Druid: Wildlife Kill" (KILL_CREDIT 44777), which nothing casts.
-- Ours: the credit comes from SmartAI on each entry's death, to the loot recipient and its group (target self), instead of the spell.
-- Not to the killing blow: Zen'Kiki 44904 casts Wrath in combat and can land it himself, and a creature gets no credit. No condition on
-- Zen'Kiki's aura 83819 "Escorting Zen'Kiki": its spell_area rows keep it on every player who has the quest incomplete in the zone (and
-- it summons him), so it would add nothing, and a source-22 condition would test the killing blow, which may be Zen'Kiki. No per-guid
-- scripts exist on these entries.
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND ((`entryorguid`=1817 AND `id`=1) OR (`entryorguid`=1822 AND `id`=2) OR (`entryorguid`=1824 AND `id`=1) OR (`entryorguid`=44482 AND `id`=1));
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(1817,0,1,0,6,0,100,0,0,0,0,0,0,33,44777,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Diseased Wolf - On Death - Kill Credit ''Wildlife killed with Zen''''Kiki'' (loot recipient and group)'),
(1822,0,2,0,6,0,100,0,0,0,0,0,0,33,44777,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Venom Mist Lurker - On Death - Kill Credit ''Wildlife killed with Zen''''Kiki'' (loot recipient and group)'),
(1824,0,1,0,6,0,100,0,0,0,0,0,0,33,44777,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Plague Lurker - On Death - Kill Credit ''Wildlife killed with Zen''''Kiki'' (loot recipient and group)'),
(44482,0,1,0,6,0,100,0,0,0,0,0,0,33,44777,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hulking Plaguebear - On Death - Kill Credit ''Wildlife killed with Zen''''Kiki'' (loot recipient and group)');
