-- Gor'mul 42130 only ends Hammerfall 26084. Sourced: both TDBs and Wowhead's Quick Facts start it from Dark Ranger Alina 41949 only.
DELETE FROM `creature_queststarter` WHERE `id`=42130 AND `quest`=26084;
