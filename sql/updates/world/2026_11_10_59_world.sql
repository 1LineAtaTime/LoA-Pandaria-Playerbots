-- Western Plaguelands: PvP flag on the faction quest NPCs, as TDB 12.1.0.
-- Sourced: TDB 1210 `creature_template_addon`.PvPFlags 1 (TDB 4.4.2 agrees) for High Priest Thel'danis 1854, High Executor Derrington
-- 10837, Commander Ashlam Valorfist 10838, High Priestess MacDonnell 11053, Shadow Priestess Vandis 11055, Apothecary Dithers 11057,
-- Nathaniel Dumah 11616, Highlord Tirion Fordring 44441, Lady Sylvanas Windrunner 44451, Koltira Deathweaver 44452, Thassarian 44453,
-- Jearl Donald 44462, Lindsay Ravensun 44466, Lang Loosegrip 44467, Lurid 44471, Kelly Dumah 44472 and Thurman Grant 45165; mop had 0.
-- The Argent and neutral NPCs stay at 0, as in both TDBs. Only the flag changes: the TDB's quest-invisibility auras on Sylvanas and
-- Thurman belong to the Andorhal phase work and are not set here.
UPDATE `creature_template_addon` SET `PvPFlags`=1 WHERE `entry` IN (1854,10837,10838,11053,11055,11057,11616,44441,44451,44452,44453,44462,44466,44467,44471,44472,45165);
