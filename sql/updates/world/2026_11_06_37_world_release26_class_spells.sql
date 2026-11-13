-- Published SkyFire release26 class spell data, adapted to registered local handlers.
-- Preserve custom non-client spell bindings and unrelated proc/bonus/link entries.

-- 2026_08_02_world_08.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_08.sql
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(12975,'spell_warr_last_stand');

-- 2026_08_02_world_14.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_14.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=134815 AND `ScriptName`='spell_whee_trigger';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(134815,'spell_whee_trigger');

-- 2026_08_02_world_34.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_34.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=62606 AND `ScriptName`='spell_dru_savage_defense';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(62606,'spell_dru_savage_defense');

-- 2026_08_02_world_35.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_35.sql
INSERT INTO `spell_bonus_data` (`entry`,`direct_bonus`,`dot_bonus`,`ap_bonus`,`ap_dot_bonus`,`comments`) VALUES
(33745,0,0,0.616,0,'Druid - Lacerate (MoP: AP*0.616 direct; DoT AP via script)')
ON DUPLICATE KEY UPDATE `direct_bonus`=VALUES(`direct_bonus`),`dot_bonus`=VALUES(`dot_bonus`),`ap_bonus`=VALUES(`ap_bonus`),`ap_dot_bonus`=VALUES(`ap_dot_bonus`),`comments`=VALUES(`comments`);
DELETE FROM `spell_script_names` WHERE `spell_id`=33745 AND `ScriptName`='spell_dru_lacerate';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(33745,'spell_dru_lacerate');

-- 2026_08_03_world_02.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_03_world_02.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=18499 AND `ScriptName`='spell_warr_berserker_rage';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(18499,'spell_warr_berserker_rage');

-- 2026_08_03_world_03.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_03_world_03.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=7384 AND `ScriptName`='spell_warr_overpower';
DELETE FROM `spell_script_names` WHERE `spell_id`=56636 AND `ScriptName`='spell_warr_taste_for_blood';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(56636,'spell_warr_taste_for_blood'),
(7384,'spell_warr_overpower');
INSERT INTO `spell_proc_event` (`entry`,`SchoolMask`,`SpellFamilyName`,`SpellFamilyMask0`,`SpellFamilyMask1`,`SpellFamilyMask2`,`SpellFamilyMask3`,`procFlags`,`procEx`,`ppmRate`,`CustomChance`,`Cooldown`,`phase`) VALUES
(56636,0,0,0,0,0,0,0x00000014,0x00000013,0,0,0,'PROC_PHASE_HIT')
ON DUPLICATE KEY UPDATE `SchoolMask`=VALUES(`SchoolMask`),`SpellFamilyName`=VALUES(`SpellFamilyName`),`SpellFamilyMask0`=VALUES(`SpellFamilyMask0`),`SpellFamilyMask1`=VALUES(`SpellFamilyMask1`),`SpellFamilyMask2`=VALUES(`SpellFamilyMask2`),`SpellFamilyMask3`=VALUES(`SpellFamilyMask3`),`procFlags`=VALUES(`procFlags`),`procEx`=VALUES(`procEx`),`ppmRate`=VALUES(`ppmRate`),`CustomChance`=VALUES(`CustomChance`),`Cooldown`=VALUES(`Cooldown`),`phase`=VALUES(`phase`);

-- 2026_08_03_world_07.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_03_world_07.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=64380 AND `ScriptName`='spell_warr_shattering_throw';
DELETE FROM `spell_script_names` WHERE `spell_id`=65941 AND `ScriptName`='spell_warr_shattering_throw';
DELETE FROM `spell_script_names` WHERE `spell_id`=113000 AND `ScriptName`='spell_warr_shattering_throw';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(64380,'spell_warr_shattering_throw'),
(64382,'spell_warr_shattering_throw'),
(65940,'spell_warr_shattering_throw'),
(65941,'spell_warr_shattering_throw'),
(112997,'spell_warr_shattering_throw'),
(113000,'spell_warr_shattering_throw');

-- 2026_08_03_world_10.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_03_world_10.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=871 AND `ScriptName`='spell_warr_shield_visual';
DELETE FROM `spell_script_names` WHERE `spell_id`=23920 AND `ScriptName`='spell_warr_shield_visual';
DELETE FROM `spell_script_names` WHERE `spell_id`=23920 AND `ScriptName`='spell_warr_spell_reflection';
DELETE FROM `spell_script_names` WHERE `spell_id`=114028 AND `ScriptName`='spell_warr_spell_reflection';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(871,'spell_warr_shield_visual'),
(23920,'spell_warr_shield_visual'),
(23920,'spell_warr_spell_reflection'),
(114028,'spell_warr_spell_reflection');

-- 2026_08_03_world_13.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_03_world_13.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=31850 AND `ScriptName`='spell_pal_ardent_defender';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(31850,'spell_pal_ardent_defender');

-- 2026_08_03_world_14.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_03_world_14.sql
INSERT INTO `spell_proc_event` (`entry`,`SchoolMask`,`SpellFamilyName`,`SpellFamilyMask0`,`SpellFamilyMask1`,`SpellFamilyMask2`,`SpellFamilyMask3`,`procFlags`,`procEx`,`ppmRate`,`CustomChance`,`Cooldown`,`phase`) VALUES
(109306,0,9,0x00025880,0x88000800,0x01000101,0x0000208D,0x00011510,0x00000000,0,30,0,'PROC_PHASE_HIT')
ON DUPLICATE KEY UPDATE `SchoolMask`=VALUES(`SchoolMask`),`SpellFamilyName`=VALUES(`SpellFamilyName`),`SpellFamilyMask0`=VALUES(`SpellFamilyMask0`),`SpellFamilyMask1`=VALUES(`SpellFamilyMask1`),`SpellFamilyMask2`=VALUES(`SpellFamilyMask2`),`SpellFamilyMask3`=VALUES(`SpellFamilyMask3`),`procFlags`=VALUES(`procFlags`),`procEx`=VALUES(`procEx`),`ppmRate`=VALUES(`ppmRate`),`CustomChance`=VALUES(`CustomChance`),`Cooldown`=VALUES(`Cooldown`),`phase`=VALUES(`phase`);

-- 2026_08_04_world_00.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_00.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=46584 AND `ScriptName`='spell_dk_raise_dead';
DELETE FROM `spell_script_names` WHERE `spell_id`=47632 AND `ScriptName`='spell_dk_runic_empowerment';
DELETE FROM `spell_script_names` WHERE `spell_id`=49143 AND `ScriptName`='spell_dk_runic_empowerment';
DELETE FROM `spell_script_names` WHERE `spell_id`=51462 AND `ScriptName`='spell_dk_runic_corruption';
DELETE FROM `spell_script_names` WHERE `spell_id`=56815 AND `ScriptName`='spell_dk_runic_empowerment';
DELETE FROM `spell_script_names` WHERE `spell_id`=108199 AND `ScriptName`='spell_dk_gorefiends_grasp';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(108199,'spell_dk_gorefiends_grasp'),
(47632,'spell_dk_runic_empowerment'),
(49143,'spell_dk_runic_empowerment'),
(56815,'spell_dk_runic_empowerment'),
(47632,'spell_dk_runic_corruption'),
(49143,'spell_dk_runic_corruption'),
(56815,'spell_dk_runic_corruption'),
(46584,'spell_dk_raise_dead');

-- 2026_08_04_world_01.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_01.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=84654 AND `ScriptName`='spell_rog_bandits_guile';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(84654,'spell_rog_bandits_guile');

-- 2026_08_04_world_02.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_02.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=35551 AND `ScriptName`='spell_rog_combat_potency';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(35551,'spell_rog_combat_potency');

-- 2026_08_04_world_03.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_03.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=79096 AND `ScriptName`='spell_rog_restless_blades';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(79096,'spell_rog_restless_blades');
INSERT INTO `spell_proc_event` (`entry`,`SchoolMask`,`SpellFamilyName`,`SpellFamilyMask0`,`SpellFamilyMask1`,`SpellFamilyMask2`,`procFlags`,`procEx`,`ppmRate`,`CustomChance`,`Cooldown`,`SpellFamilyMask3`,`phase`) VALUES
(79096,0,8,1179648,1,8192,0,3,0,0,0,'0','PROC_PHASE_HIT')
ON DUPLICATE KEY UPDATE `SchoolMask`=VALUES(`SchoolMask`),`SpellFamilyName`=VALUES(`SpellFamilyName`),`SpellFamilyMask0`=VALUES(`SpellFamilyMask0`),`SpellFamilyMask1`=VALUES(`SpellFamilyMask1`),`SpellFamilyMask2`=VALUES(`SpellFamilyMask2`),`procFlags`=VALUES(`procFlags`),`procEx`=VALUES(`procEx`),`ppmRate`=VALUES(`ppmRate`),`CustomChance`=VALUES(`CustomChance`),`Cooldown`=VALUES(`Cooldown`),`SpellFamilyMask3`=VALUES(`SpellFamilyMask3`),`phase`=VALUES(`phase`);

-- 2026_08_04_world_05.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_05.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=51723 AND `ScriptName`='spell_rog_subterfuge_cast_trigger';
DELETE FROM `spell_script_names` WHERE `spell_id`=115191 AND `ScriptName`='spell_rog_stealth_subterfuge';
DELETE FROM `spell_script_names` WHERE `spell_id`=121411 AND `ScriptName`='spell_rog_subterfuge_cast_trigger';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(115191,'spell_rog_stealth_subterfuge'),
(51723,'spell_rog_subterfuge_cast_trigger'),
(121411,'spell_rog_subterfuge_cast_trigger');
INSERT INTO `spell_proc_event` (`entry`,`SchoolMask`,`SpellFamilyName`,`SpellFamilyMask0`,`SpellFamilyMask1`,`SpellFamilyMask2`,`procFlags`,`procEx`,`ppmRate`,`CustomChance`,`Cooldown`,`SpellFamilyMask3`,`phase`) VALUES
(115191,0,0,0,0,0,0,3,0,0,0,'0','PROC_PHASE_HIT')
ON DUPLICATE KEY UPDATE `SchoolMask`=VALUES(`SchoolMask`),`SpellFamilyName`=VALUES(`SpellFamilyName`),`SpellFamilyMask0`=VALUES(`SpellFamilyMask0`),`SpellFamilyMask1`=VALUES(`SpellFamilyMask1`),`SpellFamilyMask2`=VALUES(`SpellFamilyMask2`),`procFlags`=VALUES(`procFlags`),`procEx`=VALUES(`procEx`),`ppmRate`=VALUES(`ppmRate`),`CustomChance`=VALUES(`CustomChance`),`Cooldown`=VALUES(`Cooldown`),`SpellFamilyMask3`=VALUES(`SpellFamilyMask3`),`phase`=VALUES(`phase`);
DELETE FROM `spell_linked_spell` WHERE `spell_trigger` IN (-108208, -115192) AND `spell_effect` IN (-115191, -115193);
INSERT INTO `spell_linked_spell` (`spell_trigger`,`spell_effect`,`type`,`comment`) VALUES
(-108208,-115191,0,'Subterfuge talent removed -> remove Subterfuge Stealth'),
(-115192,-115191,0,'Subterfuge buff removed -> remove Subterfuge Stealth'),
(-115192,-115193,0,'Subterfuge buff removed -> remove Subterfuge Vanish')
ON DUPLICATE KEY UPDATE `comment`=VALUES(`comment`);

-- 2026_08_04_world_06.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_06.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=1856 AND `ScriptName`='spell_rog_vanish_initial';
DELETE FROM `spell_script_names` WHERE `spell_id`=11327 AND `ScriptName`='spell_rog_vanish';
DELETE FROM `spell_script_names` WHERE `spell_id`=115193 AND `ScriptName`='spell_rog_vanish';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(11327,'spell_rog_vanish'),
(115193,'spell_rog_vanish'),
(1856,'spell_rog_vanish_initial');
INSERT INTO `spell_proc_event` (`entry`,`SchoolMask`,`SpellFamilyName`,`SpellFamilyMask0`,`SpellFamilyMask1`,`SpellFamilyMask2`,`procFlags`,`procEx`,`ppmRate`,`CustomChance`,`Cooldown`,`SpellFamilyMask3`,`phase`) VALUES
(115193,0,0,0,0,0,70996,3,0,0,0,'0','PROC_PHASE_HIT')
ON DUPLICATE KEY UPDATE `SchoolMask`=VALUES(`SchoolMask`),`SpellFamilyName`=VALUES(`SpellFamilyName`),`SpellFamilyMask0`=VALUES(`SpellFamilyMask0`),`SpellFamilyMask1`=VALUES(`SpellFamilyMask1`),`SpellFamilyMask2`=VALUES(`SpellFamilyMask2`),`procFlags`=VALUES(`procFlags`),`procEx`=VALUES(`procEx`),`ppmRate`=VALUES(`ppmRate`),`CustomChance`=VALUES(`CustomChance`),`Cooldown`=VALUES(`Cooldown`),`SpellFamilyMask3`=VALUES(`SpellFamilyMask3`),`phase`=VALUES(`phase`);
DELETE FROM `spell_linked_spell` WHERE `spell_trigger` = -1784 AND `spell_effect` = -11327;
INSERT INTO `spell_linked_spell` (`spell_trigger`,`spell_effect`,`type`,`comment`) VALUES
(-1784,-11327,0,'Stealth removed -> remove Vanish')
ON DUPLICATE KEY UPDATE `comment`=VALUES(`comment`);

-- 2026_08_04_world_07.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_07.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=121411 AND `ScriptName`='spell_rog_crimson_tempest';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(121411,'spell_rog_crimson_tempest');

-- 2026_08_04_world_08.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_08.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=51723 AND `ScriptName`='spell_rog_fan_of_knives';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(51723,'spell_rog_fan_of_knives');

-- 2026_08_04_world_09.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_09.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=51690 AND `ScriptName`='spell_rog_killing_spree';
DELETE FROM `spell_script_names` WHERE `spell_id`=51690 AND `ScriptName`='spell_rog_killing_spree_target_selector';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(51690,'spell_rog_killing_spree'),
(51690,'spell_rog_killing_spree_target_selector');

-- 2026_08_04_world_10.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_10.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=408 AND `ScriptName`='spell_rog_kidney_shot';
DELETE FROM `spell_script_names` WHERE `spell_id`=1752 AND `ScriptName`='spell_rog_sinister_strike';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(1752,'spell_rog_sinister_strike'),
(408,'spell_rog_kidney_shot');

-- 2026_08_04_world_11.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_11.sql
INSERT INTO `spell_proc_event` (`entry`,`SchoolMask`,`SpellFamilyName`,`SpellFamilyMask0`,`SpellFamilyMask1`,`SpellFamilyMask2`,`procFlags`,`procEx`,`ppmRate`,`CustomChance`,`Cooldown`,`SpellFamilyMask3`,`phase`) VALUES
(121152,0,8,0,0x6,0,0,3,0,0,0,'0','PROC_PHASE_HIT'),
(121153,0,8,0,0,0x100,0,0,0,0,0,'0','PROC_PHASE_HIT')
ON DUPLICATE KEY UPDATE `SchoolMask`=VALUES(`SchoolMask`),`SpellFamilyName`=VALUES(`SpellFamilyName`),`SpellFamilyMask0`=VALUES(`SpellFamilyMask0`),`SpellFamilyMask1`=VALUES(`SpellFamilyMask1`),`SpellFamilyMask2`=VALUES(`SpellFamilyMask2`),`procFlags`=VALUES(`procFlags`),`procEx`=VALUES(`procEx`),`ppmRate`=VALUES(`ppmRate`),`CustomChance`=VALUES(`CustomChance`),`Cooldown`=VALUES(`Cooldown`),`SpellFamilyMask3`=VALUES(`SpellFamilyMask3`),`phase`=VALUES(`phase`);

-- 2026_08_04_world_12.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_12.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=51667 AND `ScriptName`='spell_rog_cut_to_the_chase';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(51667,'spell_rog_cut_to_the_chase');
INSERT INTO `spell_proc_event` (`entry`,`SchoolMask`,`SpellFamilyName`,`SpellFamilyMask0`,`SpellFamilyMask1`,`SpellFamilyMask2`,`procFlags`,`procEx`,`ppmRate`,`CustomChance`,`Cooldown`,`SpellFamilyMask3`,`phase`) VALUES
(51667,0,8,0,0x8,0,0,0,0,0,0,'0','PROC_PHASE_HIT')
ON DUPLICATE KEY UPDATE `SchoolMask`=VALUES(`SchoolMask`),`SpellFamilyName`=VALUES(`SpellFamilyName`),`SpellFamilyMask0`=VALUES(`SpellFamilyMask0`),`SpellFamilyMask1`=VALUES(`SpellFamilyMask1`),`SpellFamilyMask2`=VALUES(`SpellFamilyMask2`),`procFlags`=VALUES(`procFlags`),`procEx`=VALUES(`procEx`),`ppmRate`=VALUES(`ppmRate`),`CustomChance`=VALUES(`CustomChance`),`Cooldown`=VALUES(`Cooldown`),`SpellFamilyMask3`=VALUES(`SpellFamilyMask3`),`phase`=VALUES(`phase`);

-- 2026_08_04_world_13.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_13.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=79134 AND `ScriptName`='spell_rog_venomous_wounds';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(79134,'spell_rog_venomous_wounds');
DELETE FROM `spell_script_names` WHERE `spell_id`=1943 AND `ScriptName`='spell_rog_rupture';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(1943,'spell_rog_rupture');

-- 2026_08_04_world_14.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_14.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=51701 AND `ScriptName`='spell_rog_honor_among_thieves';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(51701,'spell_rog_honor_among_thieves');
INSERT INTO `spell_proc_event` (`entry`,`SchoolMask`,`SpellFamilyName`,`SpellFamilyMask0`,`SpellFamilyMask1`,`SpellFamilyMask2`,`procFlags`,`procEx`,`ppmRate`,`CustomChance`,`Cooldown`,`SpellFamilyMask3`,`phase`) VALUES
(51701,0,0,0,0,0,0,2,0,100,0,'0','PROC_PHASE_HIT')
ON DUPLICATE KEY UPDATE `SchoolMask`=VALUES(`SchoolMask`),`SpellFamilyName`=VALUES(`SpellFamilyName`),`SpellFamilyMask0`=VALUES(`SpellFamilyMask0`),`SpellFamilyMask1`=VALUES(`SpellFamilyMask1`),`SpellFamilyMask2`=VALUES(`SpellFamilyMask2`),`procFlags`=VALUES(`procFlags`),`procEx`=VALUES(`procEx`),`ppmRate`=VALUES(`ppmRate`),`CustomChance`=VALUES(`CustomChance`),`Cooldown`=VALUES(`Cooldown`),`SpellFamilyMask3`=VALUES(`SpellFamilyMask3`),`phase`=VALUES(`phase`);

-- 2026_08_04_world_15.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_15.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=703 AND `ScriptName`='spell_rog_sanguinary_vein';
DELETE FROM `spell_script_names` WHERE `spell_id`=1943 AND `ScriptName`='spell_rog_sanguinary_vein';
DELETE FROM `spell_script_names` WHERE `spell_id`=89775 AND `ScriptName`='spell_rog_sanguinary_vein';
DELETE FROM `spell_script_names` WHERE `spell_id`=122233 AND `ScriptName`='spell_rog_sanguinary_vein';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(1943,'spell_rog_sanguinary_vein'),
(703,'spell_rog_sanguinary_vein'),
(122233,'spell_rog_sanguinary_vein'),
(89775,'spell_rog_sanguinary_vein');

-- 2026_08_04_world_16.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_16.sql
DELETE FROM `spell_linked_spell` WHERE `spell_trigger` = 108212 AND `spell_effect` = 137573;
INSERT INTO `spell_linked_spell` (`spell_trigger`,`spell_effect`,`type`,`comment`) VALUES
(108212,137573,0,'Burst of Speed')
ON DUPLICATE KEY UPDATE `comment`=VALUES(`comment`);

-- 2026_08_04_world_17.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_17.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=16511 AND `ScriptName`='spell_rog_hemorrhage';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(16511,'spell_rog_hemorrhage');

-- 2026_08_04_world_20.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_20.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=36554 AND `ScriptName`='spell_rog_shadowstep';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(36554,'spell_rog_shadowstep');

-- 2026_08_05_world_16.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_16.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=6544 AND `ScriptName`='spell_warr_heroic_leap';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(6544,'spell_warr_heroic_leap');

-- 2026_08_05_world_17.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_17.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=2094 AND `ScriptName`='spell_rog_blind';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(2094,'spell_rog_blind');

-- 2026_08_05_world_18.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_18.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=2818 AND `ScriptName`='spell_rog_master_poisoner';
DELETE FROM `spell_script_names` WHERE `spell_id`=3409 AND `ScriptName`='spell_rog_master_poisoner';
DELETE FROM `spell_script_names` WHERE `spell_id`=5760 AND `ScriptName`='spell_rog_master_poisoner';
DELETE FROM `spell_script_names` WHERE `spell_id`=8680 AND `ScriptName`='spell_rog_master_poisoner';
DELETE FROM `spell_script_names` WHERE `spell_id`=112961 AND `ScriptName`='spell_rog_master_poisoner';
DELETE FROM `spell_script_names` WHERE `spell_id`=113780 AND `ScriptName`='spell_rog_master_poisoner';
DELETE FROM `spell_script_names` WHERE `spell_id`=113952 AND `ScriptName`='spell_rog_master_poisoner';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(2818,'spell_rog_master_poisoner'),
(3409,'spell_rog_master_poisoner'),
(5760,'spell_rog_master_poisoner'),
(8680,'spell_rog_master_poisoner'),
(112961,'spell_rog_master_poisoner'),
(113780,'spell_rog_master_poisoner'),
(113952,'spell_rog_master_poisoner');

-- 2026_08_05_world_19.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_19.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=408 AND `ScriptName`='spell_rog_nerve_strike';
DELETE FROM `spell_script_names` WHERE `spell_id`=1833 AND `ScriptName`='spell_rog_nerve_strike';
DELETE FROM `spell_script_names` WHERE `spell_id`=112947 AND `ScriptName`='spell_rog_nerve_strike_effect';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(408,'spell_rog_nerve_strike'),
(1833,'spell_rog_nerve_strike'),
(112947,'spell_rog_nerve_strike_effect');

-- 2026_08_05_world_20.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_20.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=14185 AND `ScriptName`='spell_rog_preparation';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(14185,'spell_rog_preparation');

-- 2026_08_05_world_21.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_21.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=73981 AND `ScriptName`='spell_rog_redirect';
DELETE FROM `spell_script_names` WHERE `spell_id`=110730 AND `ScriptName`='spell_rog_redirect';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(73981,'spell_rog_redirect'),
(110730,'spell_rog_redirect');

-- 2026_08_05_world_22.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_22.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=14181 AND `ScriptName`='spell_rog_relentless_strikes';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(14181,'spell_rog_relentless_strikes');

-- 2026_08_05_world_23.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_23.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=115834 AND `ScriptName`='spell_rog_shroud_of_concealment';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(115834,'spell_rog_shroud_of_concealment');

-- 2026_08_05_world_24.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_24.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=76577 AND `ScriptName`='spell_rog_smoke_bomb';
DELETE FROM `spell_script_names` WHERE `spell_id`=128829 AND `ScriptName`='spell_rog_smoke_bomb';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(76577,'spell_rog_smoke_bomb'),
(128829,'spell_rog_smoke_bomb');

-- 2026_08_06_world_00.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_00.sql
INSERT INTO `spell_bonus_data` (`entry`,`direct_bonus`,`dot_bonus`,`ap_bonus`,`ap_dot_bonus`,`comments`) VALUES
(22568,-1,-1,0.196,-1,'Ferocious Bite')
ON DUPLICATE KEY UPDATE `direct_bonus`=VALUES(`direct_bonus`),`dot_bonus`=VALUES(`dot_bonus`),`ap_bonus`=VALUES(`ap_bonus`),`ap_dot_bonus`=VALUES(`ap_dot_bonus`),`comments`=VALUES(`comments`);
DELETE FROM `spell_script_names` WHERE `spell_id`=22568 AND `ScriptName`='spell_dru_ferocious_bite';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(22568,'spell_dru_ferocious_bite');

-- 2026_08_06_world_01.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_01.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=5185 AND `ScriptName`='spell_dru_lifebloom_refresh';
DELETE FROM `spell_script_names` WHERE `spell_id`=8936 AND `ScriptName`='spell_dru_lifebloom_refresh';
DELETE FROM `spell_script_names` WHERE `spell_id`=50464 AND `ScriptName`='spell_dru_lifebloom_refresh';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(5185,'spell_dru_lifebloom_refresh'),
(50464,'spell_dru_lifebloom_refresh'),
(8936,'spell_dru_lifebloom_refresh');

-- 2026_08_06_world_03.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_03.sql
DELETE FROM `npc_trainer` WHERE `entry` = 200011 AND `spell` IN (80964, 80965, 106839);
INSERT INTO `npc_trainer` (`entry`,`spell`,`spellcost`,`reqskill`,`reqskillvalue`,`reqlevel`) VALUES
(200011,106839,2100,0,0,22)
ON DUPLICATE KEY UPDATE `spellcost`=VALUES(`spellcost`),`reqskill`=VALUES(`reqskill`),`reqskillvalue`=VALUES(`reqskillvalue`),`reqlevel`=VALUES(`reqlevel`);
DELETE FROM `spell_script_names` WHERE `spell_id`=80964 AND `ScriptName`='spell_dru_skull_bash';
DELETE FROM `spell_script_names` WHERE `spell_id`=80965 AND `ScriptName`='spell_dru_skull_bash';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(106839,'spell_dru_skull_bash'),
(80964,'spell_dru_skull_bash'),
(80965,'spell_dru_skull_bash');

-- 2026_08_06_world_04.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_04.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=88747 AND `ScriptName`='spell_dru_wild_mushroom';
DELETE FROM `spell_script_names` WHERE `spell_id`=88751 AND `ScriptName`='spell_dru_wild_mushroom_detonate';
DELETE FROM `spell_script_names` WHERE `spell_id`=102791 AND `ScriptName`='spell_dru_wild_mushroom_bloom';
DELETE FROM `spell_script_names` WHERE `spell_id`=102792 AND `ScriptName`='spell_dru_wild_mushroom_heal';
DELETE FROM `spell_script_names` WHERE `spell_id`=138611 AND `ScriptName`='spell_dru_wild_mushroom_overheal';
DELETE FROM `spell_script_names` WHERE `spell_id`=145205 AND `ScriptName`='spell_dru_wild_mushroom';
DELETE FROM `spell_script_names` WHERE `spell_id`=147349 AND `ScriptName`='spell_dru_wild_mushroom';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(88747,'spell_dru_wild_mushroom'),
(88751,'spell_dru_wild_mushroom_detonate'),
(102791,'spell_dru_wild_mushroom_bloom'),
(102792,'spell_dru_wild_mushroom_heal'),
(138611,'spell_dru_wild_mushroom_overheal'),
(145205,'spell_dru_wild_mushroom'),
(147349,'spell_dru_wild_mushroom');
UPDATE `creature_template` SET
    `minlevel` = 85,
    `maxlevel` = 85,
    `unit_flags` = 540,
    `ScriptName` = 'npc_wild_mushroom'
WHERE `entry` = 47649;
UPDATE `creature_template` SET
    `unit_flags` = 33554456,
    `ScriptName` = 'npc_fungal_growth'
WHERE `entry` = 43484;
INSERT INTO `spell_proc_event` (`entry`,`SchoolMask`,`SpellFamilyName`,`SpellFamilyMask0`,`SpellFamilyMask1`,`SpellFamilyMask2`,`SpellFamilyMask3`,`procFlags`,`procEx`,`ppmRate`,`CustomChance`,`Cooldown`,`phase`) VALUES
(138611,0,7,16,0,0,0,0x00044000,0,0,0,0,'PROC_PHASE_HIT')
ON DUPLICATE KEY UPDATE `SchoolMask`=VALUES(`SchoolMask`),`SpellFamilyName`=VALUES(`SpellFamilyName`),`SpellFamilyMask0`=VALUES(`SpellFamilyMask0`),`SpellFamilyMask1`=VALUES(`SpellFamilyMask1`),`SpellFamilyMask2`=VALUES(`SpellFamilyMask2`),`SpellFamilyMask3`=VALUES(`SpellFamilyMask3`),`procFlags`=VALUES(`procFlags`),`procEx`=VALUES(`procEx`),`ppmRate`=VALUES(`ppmRate`),`CustomChance`=VALUES(`CustomChance`),`Cooldown`=VALUES(`Cooldown`),`phase`=VALUES(`phase`);

-- 2026_08_06_world_05.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_05.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=102795 AND `ScriptName`='spell_dru_bear_hug';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(102795,'spell_dru_bear_hug');

-- 2026_08_06_world_06.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_06.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=22842 AND `ScriptName`='spell_dru_frenzied_regeneration';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(22842,'spell_dru_frenzied_regeneration');

-- 2026_08_06_world_07.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_07.sql
INSERT INTO `spell_bonus_data` (`entry`,`direct_bonus`,`dot_bonus`,`ap_bonus`,`ap_dot_bonus`,`comments`) VALUES
(779,0,0,0.225,-1,'Druid - Swipe (Bear)')
ON DUPLICATE KEY UPDATE `direct_bonus`=VALUES(`direct_bonus`),`dot_bonus`=VALUES(`dot_bonus`),`ap_bonus`=VALUES(`ap_bonus`),`ap_dot_bonus`=VALUES(`ap_dot_bonus`),`comments`=VALUES(`comments`);
DELETE FROM `spell_script_names` WHERE `spell_id`=779 AND `ScriptName`='spell_dru_swipe_and_maul_bleed';
DELETE FROM `spell_script_names` WHERE `spell_id`=6807 AND `ScriptName`='spell_dru_swipe_and_maul_bleed';
DELETE FROM `spell_script_names` WHERE `spell_id`=62078 AND `ScriptName`='spell_dru_swipe_and_maul_bleed';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(779,'spell_dru_swipe_and_maul_bleed'),
(6807,'spell_dru_swipe_and_maul_bleed'),
(62078,'spell_dru_swipe_and_maul_bleed');

-- 2026_08_06_world_08.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_08.sql
INSERT INTO `spell_proc_event` (`entry`,`SchoolMask`,`SpellFamilyName`,`SpellFamilyMask0`,`SpellFamilyMask1`,`SpellFamilyMask2`,`SpellFamilyMask3`,`procFlags`,`procEx`,`ppmRate`,`CustomChance`,`Cooldown`,`phase`) VALUES
(122509,0,4,0,512,0,0,0,2,0,0,0,'PROC_PHASE_HIT')
ON DUPLICATE KEY UPDATE `SchoolMask`=VALUES(`SchoolMask`),`SpellFamilyName`=VALUES(`SpellFamilyName`),`SpellFamilyMask0`=VALUES(`SpellFamilyMask0`),`SpellFamilyMask1`=VALUES(`SpellFamilyMask1`),`SpellFamilyMask2`=VALUES(`SpellFamilyMask2`),`SpellFamilyMask3`=VALUES(`SpellFamilyMask3`),`procFlags`=VALUES(`procFlags`),`procEx`=VALUES(`procEx`),`ppmRate`=VALUES(`ppmRate`),`CustomChance`=VALUES(`CustomChance`),`Cooldown`=VALUES(`Cooldown`),`phase`=VALUES(`phase`);

-- 2026_08_06_world_09.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_09.sql
INSERT INTO `spell_proc_event` (`entry`,`SchoolMask`,`SpellFamilyName`,`SpellFamilyMask0`,`SpellFamilyMask1`,`SpellFamilyMask2`,`SpellFamilyMask3`,`procFlags`,`procEx`,`ppmRate`,`CustomChance`,`Cooldown`,`phase`) VALUES
(68164,0,4,128,0,0,0,0,0,0,0,0,'PROC_PHASE_HIT')
ON DUPLICATE KEY UPDATE `SchoolMask`=VALUES(`SchoolMask`),`SpellFamilyName`=VALUES(`SpellFamilyName`),`SpellFamilyMask0`=VALUES(`SpellFamilyMask0`),`SpellFamilyMask1`=VALUES(`SpellFamilyMask1`),`SpellFamilyMask2`=VALUES(`SpellFamilyMask2`),`SpellFamilyMask3`=VALUES(`SpellFamilyMask3`),`procFlags`=VALUES(`procFlags`),`procEx`=VALUES(`procEx`),`ppmRate`=VALUES(`ppmRate`),`CustomChance`=VALUES(`CustomChance`),`Cooldown`=VALUES(`Cooldown`),`phase`=VALUES(`phase`);

-- 2026_08_06_world_10.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_10.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=48517 AND `ScriptName`='spell_dru_eclipse_solar';
DELETE FROM `spell_script_names` WHERE `spell_id`=48518 AND `ScriptName`='spell_dru_eclipse_lunar';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(48517,'spell_dru_eclipse_solar'),
(48518,'spell_dru_eclipse_lunar');

-- 2026_08_06_world_11.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_11.sql
INSERT INTO `spell_proc_event` (`entry`,`SchoolMask`,`SpellFamilyName`,`SpellFamilyMask0`,`SpellFamilyMask1`,`SpellFamilyMask2`,`SpellFamilyMask3`,`procFlags`,`procEx`,`ppmRate`,`CustomChance`,`Cooldown`,`phase`) VALUES
(93399,0,7,2,0,0,0,0,2,0,0,0,'PROC_PHASE_HIT')
ON DUPLICATE KEY UPDATE `SchoolMask`=VALUES(`SchoolMask`),`SpellFamilyName`=VALUES(`SpellFamilyName`),`SpellFamilyMask0`=VALUES(`SpellFamilyMask0`),`SpellFamilyMask1`=VALUES(`SpellFamilyMask1`),`SpellFamilyMask2`=VALUES(`SpellFamilyMask2`),`SpellFamilyMask3`=VALUES(`SpellFamilyMask3`),`procFlags`=VALUES(`procFlags`),`procEx`=VALUES(`procEx`),`ppmRate`=VALUES(`ppmRate`),`CustomChance`=VALUES(`CustomChance`),`Cooldown`=VALUES(`Cooldown`),`phase`=VALUES(`phase`);
DELETE FROM `spell_script_names` WHERE `spell_id`=93400 AND `ScriptName`='spell_dru_shooting_stars';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(93400,'spell_dru_shooting_stars');

-- 2026_08_07_world_02.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_02.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=770 AND `ScriptName`='spell_dru_faerie_fire';
DELETE FROM `spell_script_names` WHERE `spell_id`=102355 AND `ScriptName`='spell_dru_faerie_swarm';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(770,'spell_dru_faerie_fire'),
(102355,'spell_dru_faerie_swarm');

-- 2026_08_07_world_03.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_03.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=770 AND `ScriptName`='spell_dru_faerie_fire';
DELETE FROM `spell_script_names` WHERE `spell_id`=33745 AND `ScriptName`='spell_dru_lacerate';
DELETE FROM `spell_script_names` WHERE `spell_id`=77758 AND `ScriptName`='spell_dru_thrash_bear';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(33745,'spell_dru_lacerate'),
(77758,'spell_dru_thrash_bear'),
(770,'spell_dru_faerie_fire');

-- 2026_08_07_world_07.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_07.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=145108 AND `ScriptName`='spell_dru_yseras_gift';
DELETE FROM `spell_script_names` WHERE `spell_id`=145110 AND `ScriptName`='spell_dru_yseras_gift_heal_ally';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(145108,'spell_dru_yseras_gift'),
(145110,'spell_dru_yseras_gift_heal_ally');

-- 2026_08_07_world_08.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_08.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=1079 AND `ScriptName`='spell_dru_soul_of_the_forest_feral';
DELETE FROM `spell_script_names` WHERE `spell_id`=18562 AND `ScriptName`='spell_dru_swiftmend';
DELETE FROM `spell_script_names` WHERE `spell_id`=22568 AND `ScriptName`='spell_dru_soul_of_the_forest_feral';
DELETE FROM `spell_script_names` WHERE `spell_id`=22570 AND `ScriptName`='spell_dru_soul_of_the_forest_feral';
DELETE FROM `spell_script_names` WHERE `spell_id`=33878 AND `ScriptName`='spell_dru_soul_of_the_forest_mangle';
DELETE FROM `spell_script_names` WHERE `spell_id`=52610 AND `ScriptName`='spell_dru_soul_of_the_forest_feral';
DELETE FROM `spell_script_names` WHERE `spell_id`=114107 AND `ScriptName`='spell_dru_soul_of_the_forest';
DELETE FROM `spell_script_names` WHERE `spell_id`=127538 AND `ScriptName`='spell_dru_soul_of_the_forest_feral';
DELETE FROM `spell_script_names` WHERE `spell_id`=127663 AND `ScriptName`='spell_dru_astral_communion';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(114107,'spell_dru_soul_of_the_forest'),
(33878,'spell_dru_soul_of_the_forest_mangle'),
(1079,'spell_dru_soul_of_the_forest_feral'),
(22568,'spell_dru_soul_of_the_forest_feral'),
(22570,'spell_dru_soul_of_the_forest_feral'),
(52610,'spell_dru_soul_of_the_forest_feral'),
(127538,'spell_dru_soul_of_the_forest_feral'),
(18562,'spell_dru_swiftmend'),
(127663,'spell_dru_astral_communion');

-- 2026_08_07_world_13.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_13.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=12846 AND `ScriptName`='spell_mage_ignite';

-- 2026_08_08_world_00.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_08_world_00.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=102351 AND `ScriptName`='spell_dru_cenarion_ward';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(102351,'spell_dru_cenarion_ward');

-- 2026_08_08_world_03.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_08_world_03.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=703 AND `ScriptName`='spell_rog_cloak_and_dagger';
DELETE FROM `spell_script_names` WHERE `spell_id`=1833 AND `ScriptName`='spell_rog_cloak_and_dagger';
DELETE FROM `spell_script_names` WHERE `spell_id`=8676 AND `ScriptName`='spell_rog_cloak_and_dagger';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(703,'spell_rog_cloak_and_dagger'),
(1833,'spell_rog_cloak_and_dagger'),
(8676,'spell_rog_cloak_and_dagger');

-- 2026_08_08_world_04.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_08_world_04.sql
DELETE FROM `spell_script_names` WHERE `spell_id`=1776 AND `ScriptName`='spell_rog_dirty_tricks';
DELETE FROM `spell_script_names` WHERE `spell_id`=2094 AND `ScriptName`='spell_rog_dirty_tricks';
INSERT IGNORE INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(1776,'spell_rog_dirty_tricks'),
(2094,'spell_rog_dirty_tricks');
