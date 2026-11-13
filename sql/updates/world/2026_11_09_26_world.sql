-- The Battle for Andorhal: one breadcrumb at a time, and Thassarian no longer starts the Aerie Peak one.
-- Sourced: TDB 4.4.2 and 1210. Thassarian 44453 only ends 28505 (Gryphon Master Talonaxe starts it). ExclusiveGroup 26920 on the Horde
-- breadcrumbs 26920, 28508, 28575, 28750 and 27158 on the Alliance ones 27158, 28505, 28576, 28749; mop had it only on 26920 and 28575.
DELETE FROM `creature_queststarter` WHERE `id`=44453 AND `quest`=28505;
UPDATE `quest_template_addon` SET `ExclusiveGroup`=26920 WHERE `ID` IN (26920,28508,28575,28750);
UPDATE `quest_template_addon` SET `ExclusiveGroup`=27158 WHERE `ID` IN (27158,28505,28576,28749);
