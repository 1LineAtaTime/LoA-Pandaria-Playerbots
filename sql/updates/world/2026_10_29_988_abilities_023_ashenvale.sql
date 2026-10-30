-- #988 wave 2 (merge): Ashenvale creature abilities from DeltaTho PR 15 file 2026_10_24_31_world.sql, ADDED to creatures that already have smart_scripts.
-- 61 creatures, 95 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 3696:3 3713:1 3717:3 3733:1 3734:1 3735:2 3743:2 3745:3 3750:3 3752:4 3754:4 3755:4 3757:3 3758:2 3762:3 3763:3 3765:1 3767:2 3770:1 3771:4 3772:2 3773:2 3780:2 3799:1 3803:1 3810:1 3817:2 3821:2 3823:1 3846:1 3847:1 3917:2 3921:4 3922:3 3924:4 3925:4 3926:3 3943:3 3944:3 3987:2 4273:3 5314:1 6073:1 6115:1 10559:3 10639:1 10640:1 10641:2 10642:2 11697:2 12037:1 12474:3 12475:2 12476:2 12759:4 12940:3 14733:17 14753:19 17300:4 33688:2 34295:7

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 3696 AND `id` >= 3)
  OR (`entryorguid` = 3713 AND `id` >= 1)
  OR (`entryorguid` = 3717 AND `id` >= 3)
  OR (`entryorguid` = 3733 AND `id` >= 1)
  OR (`entryorguid` = 3734 AND `id` >= 1)
  OR (`entryorguid` = 3735 AND `id` >= 2)
  OR (`entryorguid` = 3743 AND `id` >= 2)
  OR (`entryorguid` = 3745 AND `id` >= 3)
  OR (`entryorguid` = 3750 AND `id` >= 3)
  OR (`entryorguid` = 3752 AND `id` >= 4)
  OR (`entryorguid` = 3754 AND `id` >= 4)
  OR (`entryorguid` = 3755 AND `id` >= 4)
  OR (`entryorguid` = 3757 AND `id` >= 3)
  OR (`entryorguid` = 3758 AND `id` >= 2)
  OR (`entryorguid` = 3762 AND `id` >= 3)
  OR (`entryorguid` = 3763 AND `id` >= 3)
  OR (`entryorguid` = 3765 AND `id` >= 1)
  OR (`entryorguid` = 3767 AND `id` >= 2)
  OR (`entryorguid` = 3770 AND `id` >= 1)
  OR (`entryorguid` = 3771 AND `id` >= 4)
  OR (`entryorguid` = 3772 AND `id` >= 2)
  OR (`entryorguid` = 3773 AND `id` >= 2)
  OR (`entryorguid` = 3780 AND `id` >= 2)
  OR (`entryorguid` = 3799 AND `id` >= 1)
  OR (`entryorguid` = 3803 AND `id` >= 1)
  OR (`entryorguid` = 3810 AND `id` >= 1)
  OR (`entryorguid` = 3817 AND `id` >= 2)
  OR (`entryorguid` = 3821 AND `id` >= 2)
  OR (`entryorguid` = 3823 AND `id` >= 1)
  OR (`entryorguid` = 3846 AND `id` >= 1)
  OR (`entryorguid` = 3847 AND `id` >= 1)
  OR (`entryorguid` = 3917 AND `id` >= 2)
  OR (`entryorguid` = 3921 AND `id` >= 4)
  OR (`entryorguid` = 3922 AND `id` >= 3)
  OR (`entryorguid` = 3924 AND `id` >= 4)
  OR (`entryorguid` = 3925 AND `id` >= 4)
  OR (`entryorguid` = 3926 AND `id` >= 3)
  OR (`entryorguid` = 3943 AND `id` >= 3)
  OR (`entryorguid` = 3944 AND `id` >= 3)
  OR (`entryorguid` = 3987 AND `id` >= 2)
  OR (`entryorguid` = 4273 AND `id` >= 3)
  OR (`entryorguid` = 5314 AND `id` >= 1)
  OR (`entryorguid` = 6073 AND `id` >= 1)
  OR (`entryorguid` = 6115 AND `id` >= 1)
  OR (`entryorguid` = 10559 AND `id` >= 3)
  OR (`entryorguid` = 10639 AND `id` >= 1)
  OR (`entryorguid` = 10640 AND `id` >= 1)
  OR (`entryorguid` = 10641 AND `id` >= 2)
  OR (`entryorguid` = 10642 AND `id` >= 2)
  OR (`entryorguid` = 11697 AND `id` >= 2)
  OR (`entryorguid` = 12037 AND `id` >= 1)
  OR (`entryorguid` = 12474 AND `id` >= 3)
  OR (`entryorguid` = 12475 AND `id` >= 2)
  OR (`entryorguid` = 12476 AND `id` >= 2)
  OR (`entryorguid` = 12759 AND `id` >= 4)
  OR (`entryorguid` = 12940 AND `id` >= 3)
  OR (`entryorguid` = 14733 AND `id` >= 17)
  OR (`entryorguid` = 14753 AND `id` >= 19)
  OR (`entryorguid` = 17300 AND `id` >= 4)
  OR (`entryorguid` = 33688 AND `id` >= 2)
  OR (`entryorguid` = 34295 AND `id` >= 7));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(3696, 0, 3, 0, 9, 0, 100, 0, 0, 5, 15000, 25000, 0, 11, 78842, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Ran Bloodtooth - Within 0-5 Range - Cast ''Carnivorous Bite'''),
(3696, 0, 4, 0, 9, 0, 100, 0, 5, 30, 9000, 22000, 0, 11, 78841, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Ran Bloodtooth - Within 5-30 Range - Cast ''Aimed Shot'''),
(3696, 0, 5, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ran Bloodtooth - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(3713, 0, 1, 0, 0, 0, 100, 0, 4000, 5000, 18000, 24500, 0, 11, 78732, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Wrathtail Wave Rider - In Combat - Cast ''Whirlpool'''),
(3717, 0, 3, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 0, 11, 34827, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Wrathtail Sorceress - Out of Combat - Cast ''Water Shield'''),
(3733, 0, 1, 0, 0, 0, 100, 0, 5000, 5500, 21000, 25000, 0, 11, 78705, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Forsaken Herbalist - In Combat - Cast ''Poison Bottle'''),
(3734, 0, 1, 0, 0, 0, 100, 0, 3000, 4000, 34000, 35000, 0, 11, 78710, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Orc Overseer - In Combat - Cast ''Threatening Shout'''),
(3735, 0, 2, 0, 0, 0, 100, 0, 2000, 4500, 12000, 20000, 0, 11, 21067, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Apothecary Falthis - In Combat - Cast ''Poison Bolt'''),
(3735, 0, 3, 0, 0, 0, 100, 0, 5000, 8000, 19000, 25000, 0, 11, 78705, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Apothecary Falthis - In Combat - Cast ''Poison Bottle'''),
(3743, 0, 2, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Foulweald Warrior - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(3745, 0, 3, 0, 9, 0, 100, 0, 0, 8, 18000, 21000, 0, 11, 6817, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Foulweald Pathfinder - Within 0-8 Range - Cast ''Corrupted Agility'''),
(3750, 0, 3, 0, 9, 0, 100, 0, 0, 8, 18000, 21000, 0, 11, 6817, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Foulweald Totemic - Within 0-8 Range - Cast ''Corrupted Agility'''),
(3750, 0, 4, 0, 0, 0, 100, 0, 5000, 7000, 24000, 26000, 0, 11, 39591, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Foulweald Totemic - In Combat - Cast ''Searing Totem'''),
(3752, 0, 4, 0, 0, 0, 100, 0, 4000, 6000, 22000, 29000, 0, 11, 60842, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Xavian Rogue - In Combat - Cast ''Expose Armor'''),
(3754, 0, 4, 0, 0, 0, 100, 0, 4000, 4500, 12000, 18000, 0, 11, 77522, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Xavian Betrayer - In Combat - Cast ''Swoop'''),
(3755, 0, 4, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 35913, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Xavian Felsworn - In Combat - Cast ''Fel Fireball'''),
(3757, 0, 3, 0, 0, 0, 100, 0, 5000, 5000, 21000, 32000, 0, 11, 39273, 1, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 'Xavian Hellcaller - In Combat - Cast ''Rain of Fire'''),
(3758, 0, 2, 0, 0, 0, 100, 0, 4000, 5000, 12000, 19500, 0, 11, 31279, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Felmusk Satyr - In Combat - Cast ''Swipe'''),
(3758, 0, 3, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Felmusk Satyr - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(3762, 0, 3, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 32707, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Felmusk Felsworn - In Combat - Cast ''Incinerate'''),
(3762, 0, 4, 0, 0, 0, 100, 0, 7000, 8000, 22000, 26000, 0, 11, 11962, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Felmusk Felsworn - In Combat - Cast ''Immolate'''),
(3762, 0, 5, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Felmusk Felsworn - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(3763, 0, 3, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 0, 11, 10277, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Felmusk Shadowstalker - In Combat - Cast ''Throw'''),
(3763, 0, 4, 0, 11, 0, 100, 1, 0, 0, 0, 0, 0, 11, 77806, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Felmusk Shadowstalker - On Respawn - Cast ''Stealth'' (No Repeat)'),
(3763, 0, 5, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Felmusk Shadowstalker - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(3765, 0, 1, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bleakheart Satyr - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(3765, 0, 2, 0, 0, 0, 100, 0, 5000, 5500, 12000, 16000, 0, 11, 31279, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Bleakheart Satyr - In Combat - Cast ''Swipe'''),
(3767, 0, 2, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bleakheart Trickster - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(3767, 0, 3, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 77457, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bleakheart Trickster - On Aggro - Cast ''Dazzling Lights'' (No Repeat)'),
(3770, 0, 1, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bleakheart Shadowstalker - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(3770, 0, 2, 0, 2, 0, 100, 0, 0, 50, 35000, 40000, 0, 11, 77471, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bleakheart Shadowstalker - Between 0-50% Health - Cast ''Shadow Shield'''),
(3770, 0, 3, 0, 11, 0, 100, 1, 0, 0, 0, 0, 0, 11, 77806, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bleakheart Shadowstalker - On Respawn - Cast ''Stealth'' (No Repeat)'),
(3771, 0, 4, 0, 2, 0, 100, 1, 0, 15, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bleakheart Hellcaller - Between 0-15% Health - cast enrage (No Repeat)'),
(3772, 0, 2, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lesser Felguard - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(3773, 0, 2, 0, 0, 0, 100, 0, 5000, 9000, 21000, 28000, 0, 11, 37628, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Akkrilus - In Combat - Cast ''Fel Immolate'''),
(3773, 0, 3, 0, 0, 0, 100, 0, 2000, 4000, 24000, 38000, 0, 11, 35913, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Akkrilus - In Combat - Cast ''Fel Fireball'''),
(3780, 0, 2, 0, 0, 0, 100, 0, 2000, 3000, 120000, 125000, 0, 11, 78810, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Singed Shambler - In Combat - Cast ''Smouldering'''),
(3780, 0, 3, 0, 0, 0, 100, 0, 5000, 7000, 15000, 21000, 0, 11, 78808, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Singed Shambler - In Combat - Cast ''Sunbeam'''),
(3799, 0, 1, 0, 2, 0, 100, 0, 0, 55, 20000, 34000, 0, 11, 12160, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Severed Druid - Between 0-55% Health - Cast ''Rejuvenation'''),
(3803, 0, 1, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 78907, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Severed Keeper - In Combat - Cast ''Starfire'''),
(3803, 0, 2, 0, 0, 0, 100, 0, 8000, 9000, 24000, 28000, 0, 11, 15798, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Severed Keeper - In Combat - Cast ''Moonfire'''),
(3810, 0, 1, 0, 0, 0, 100, 0, 2000, 4500, 12000, 20000, 0, 11, 84867, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Elder Ashenvale Bear - In Combat - Cast ''Sundering Swipe'''),
(3817, 0, 2, 0, 0, 0, 100, 0, 5000, 6000, 300000, 300000, 0, 11, 6922, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Shadowhorn Stag - In Combat - Cast ''Curse of the Shadowhorn'''),
(3821, 0, 2, 0, 11, 0, 100, 1, 0, 0, 0, 0, 0, 11, 77806, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Wildthorn Lurker - On Respawn - Cast ''Stealth'' (No Repeat)'),
(3823, 0, 1, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 78703, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ghostpaw Runner - On Aggro - Cast ''Ghostform'' (No Repeat)'),
(3823, 0, 2, 0, 0, 0, 100, 0, 4000, 5000, 15000, 21000, 0, 11, 78702, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ghostpaw Runner - In Combat - Cast ''Arcane Explosion'''),
(3823, 0, 3, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ghostpaw Runner - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(3846, 0, 1, 0, 0, 0, 100, 0, 2000, 4500, 12000, 20000, 0, 11, 11976, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Talen - In Combat - Cast ''Strike'''),
(3846, 0, 2, 0, 0, 0, 100, 0, 6000, 8000, 25000, 29000, 0, 11, 11977, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Talen - In Combat - Cast ''Rend'''),
(3847, 0, 1, 0, 2, 0, 100, 0, 0, 40, 14000, 21000, 0, 11, 62804, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Orendil Broadleaf - Between 0-40% Health - Cast ''Rejuvenation'''),
(3917, 0, 2, 0, 0, 0, 100, 0, 8000, 8000, 20000, 25000, 0, 11, 78542, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Befouled Water Elemental - In Combat - Cast ''Splash'''),
(3921, 0, 4, 0, 0, 0, 100, 0, 5000, 6000, 24000, 32000, 0, 11, 8078, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Thistlefur Ursa - In Combat - Cast ''Thunderclap'''),
(3921, 0, 5, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 3490, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Thistlefur Ursa - Between 0-30% Health - Cast ''Frenzied Rage'' (No Repeat)'),
(3922, 0, 3, 0, 0, 0, 100, 0, 5000, 9000, 21000, 35000, 0, 11, 78770, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Thistlefur Totemic - In Combat - Cast ''Magma Totem'''),
(3924, 0, 4, 0, 0, 0, 100, 0, 5000, 8000, 25000, 29000, 0, 11, 39591, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Thistlefur Shaman - In Combat - Cast ''Searing Totem'''),
(3925, 0, 4, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 75002, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Thistlefur Avenger - On Aggro - Cast ''Leaping Rush'' (No Repeat)'),
(3926, 0, 3, 0, 9, 0, 100, 0, 5, 30, 14000, 19000, 0, 11, 78754, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Thistlefur Pathfinder - Within 5-30 Range - Cast ''Arcane Shot'''),
(3943, 0, 3, 0, 9, 0, 100, 0, 0, 5, 15000, 20000, 0, 11, 77522, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Ruuzel - Within 0-5 Range - Cast ''Swoop'''),
(3943, 0, 4, 0, 9, 0, 100, 0, 0, 30, 28000, 32000, 0, 11, 78747, 0, 0, 0, 0, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 'Ruuzel - Within 0-30 Range - Cast ''Hand of Azshara'''),
(3944, 0, 3, 0, 0, 0, 100, 0, 4000, 7000, 17000, 28000, 0, 11, 17140, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Wrathtail Priestess - In Combat - Cast ''Holy Fire'''),
(3944, 0, 4, 0, 2, 0, 100, 0, 0, 50, 45000, 57000, 0, 11, 11835, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Wrathtail Priestess - Between 0-50% Health - Cast ''Power Word: Shield'''),
(3987, 0, 2, 0, 11, 0, 100, 1, 0, 0, 0, 0, 0, 11, 6813, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dal Bloodclaw - On Respawn - Cast ''Coat of Thistlefur'' (No Repeat)'),
(3987, 0, 3, 0, 9, 0, 100, 1, 0, 5, 14000, 17000, 0, 11, 77558, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dal Bloodclaw - Within 0-5 Range - Cast ''Bloody Strike'''),
(3987, 0, 4, 0, 2, 0, 100, 1, 0, 50, 0, 0, 0, 11, 21049, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dal Bloodclaw - Between 0-50% Health - Cast ''Bloodlust'' (No Repeat)'),
(3987, 0, 5, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dal Bloodclaw - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(4273, 0, 3, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 9739, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Keeper Ordanus - In Combat - Cast ''Wrath'''),
(4273, 0, 4, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Keeper Ordanus - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(5314, 0, 1, 0, 2, 0, 100, 0, 0, 50, 24000, 32000, 0, 11, 20671, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Phantim - Between 0-50% Health - Cast ''Summon Phantim Illusion'''),
(6073, 0, 1, 0, 0, 0, 100, 0, 6000, 8000, 18000, 22000, 0, 11, 36247, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Searing Infernal - In Combat - Cast ''Fel Fireball'''),
(6115, 0, 1, 0, 0, 0, 100, 0, 2000, 3000, 12000, 19000, 0, 11, 22859, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Roaming Felguard - In Combat - Cast ''Mortal Cleave'''),
(6115, 0, 2, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Roaming Felguard - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(10559, 0, 3, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 32011, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Lady Vespia - In Combat - Cast ''Water Bolt'''),
(10559, 0, 4, 0, 9, 0, 100, 0, 0, 10, 15000, 16000, 0, 11, 78542, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lady Vespia - Within 0-10 Range - Cast ''Splash'''),
(10639, 0, 1, 0, 11, 0, 100, 1, 0, 0, 0, 0, 0, 11, 6813, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rorgish Jowl - On Respawn - Cast ''Coat of Thistlefur'' (No Repeat)'),
(10639, 0, 2, 0, 0, 0, 100, 0, 4000, 6000, 19000, 27000, 0, 11, 8078, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rorgish Jowl - In Combat - Cast ''Thunderclap'''),
(10639, 0, 3, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 3490, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rorgish Jowl - Between 0-30% Health - Cast ''Frenzied Rage'' (No Repeat)'),
(10640, 0, 1, 0, 0, 0, 100, 0, 3000, 4000, 11000, 12000, 0, 11, 6819, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Oakpaw - In Combat - Cast ''Corrupted Stamina'''),
(10641, 0, 2, 0, 9, 0, 100, 0, 0, 30, 9000, 11000, 0, 11, 33844, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Branch Snapper - Within 0-30 Range - Cast ''Entangling Roots'''),
(10641, 0, 3, 0, 0, 0, 100, 0, 5000, 7000, 15000, 21000, 0, 11, 78808, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Branch Snapper - In Combat - Cast ''Sunbeam'''),
(10642, 0, 2, 0, 0, 0, 100, 0, 2000, 3000, 120000, 125000, 0, 11, 6873, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Eck''alom - In Combat - Cast ''Foul Chill'''),
(10642, 0, 3, 0, 0, 0, 100, 0, 8000, 8000, 20000, 25000, 0, 11, 78542, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Eck''alom - In Combat - Cast ''Splash'''),
(11697, 0, 2, 0, 0, 0, 100, 0, 5000, 7000, 18000, 24000, 0, 11, 18543, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Mannoroc Lasher - In Combat - Cast ''Flame Lash'''),
(12037, 0, 1, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 75002, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Ursol''lok - On Aggro - Cast ''Leaping Rush'' (No Repeat)'),
(12037, 0, 2, 0, 0, 0, 100, 0, 5000, 7000, 24000, 32000, 0, 11, 84867, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Ursol''lok - In Combat - Cast ''Sundering Swipe'''),
(12474, 0, 3, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 6268, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Emeraldon Boughguard - On Aggro - Cast ''Rushing Charge'' (No Repeat)'),
(12475, 0, 2, 0, 0, 0, 100, 0, 4000, 4500, 12000, 14000, 0, 11, 78921, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Emeraldon Tree Warder - In Combat - Cast ''Dream Lash'''),
(12476, 0, 2, 0, 0, 0, 100, 0, 2000, 4500, 9000, 18000, 0, 11, 9591, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Emeraldon Oracle - In Combat - Cast ''Acid Spit'''),
(12759, 0, 4, 0, 0, 0, 100, 0, 8000, 8000, 20000, 25000, 0, 11, 78542, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Tideress - In Combat - Cast ''Splash'''),
(12940, 0, 3, 0, 0, 0, 100, 0, 2000, 4000, 18000, 28000, 0, 11, 33126, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Vorsha the Lasher - In Combat - Cast ''Dropped Weapon'''),
(14733, 0, 17, 0, 9, 0, 100, 0, 0, 5, 12000, 17000, 0, 11, 15496, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Sentinel Farsong - Within 0-5 Range - Cast ''Cleave'''),
(14753, 0, 19, 0, 9, 0, 100, 0, 5, 30, 22000, 27000, 0, 11, 21390, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Illiyana Moonblaze - Within 5-30 Range - Cast ''Multi-Shot'''),
(17300, 0, 4, 0, 0, 0, 100, 0, 8000, 9000, 24000, 32000, 0, 11, 37624, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Gorgannon - In Combat - Cast ''Carrion Swarm'''),
(33688, 0, 2, 0, 9, 0, 100, 0, 0, 30, 13600, 14500, 0, 11, 11922, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Raging Ancient - Within 0-30 Range - Cast ''Entangling Roots'''),
(33688, 0, 3, 0, 0, 0, 100, 0, 5000, 8000, 17000, 25000, 0, 11, 45, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Raging Ancient - In Combat - Cast ''War Stomp'''),
(34295, 0, 7, 0, 0, 0, 100, 0, 2000, 2000, 15000, 18000, 0, 11, 65214, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lord Magmathar - In Combat - Cast ''Lordly Immolate''');
