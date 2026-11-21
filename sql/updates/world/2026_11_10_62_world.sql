-- Western Plaguelands: remove the classic Scarlet Crusade spawns from Argent Hearthglen and Northridge, and the second Lord Maldazzar.
-- Hostile level 36-56 Scarlets (faction 16 and 89) still stood among the Argent Crusade in Hearthglen, with classic guids (below 19000):
-- Scarlet High Clerist 1839 x2 (18358 patrolling, 18413), Scarlet Executioner 1841 (18436), Scarlet Interrogator 1838 (18448), Scarlet
-- Judge 1837 (18461), Scarlet Smith 1885 (18702, patrolling), and Foreman Jerris 1843 (18578) or the Scarlet Lumberjack 1884 (18568) from
-- pool 132; Foreman Marcrid 1844 (17638) patrolled the Northridge Lumber Camp. Lord Maldazzar 1848 stood twice: 317172 at Malicia's
-- Outpost and the classic 14923 patrolling inside the Cataclysm Sorrow Hill Crypt.
-- Sourced: both TDBs (4.4.2 and 1210) spawn none of these entries in Hearthglen or Northridge; they place the Interrogator, the High Clerist
-- and the Smith once each where mop already has a copy (214182 Dalson's Farm, 212825 the Charred Outpost, 212554 Redpine Dell) and Lord
-- Maldazzar on 317172's exact spot (TDB 328858). Since Cataclysm, Hearthglen is the Argent Crusade's (warcraft.wiki.gg, as reported by the
-- auditor; mop's own Hearthglen quests are Argent).
-- Ours: the deletes. Their four paths, the addons and pool 132 (only these two members) go with them; nothing else refers to them. The EPL
-- Jerris and Marcrid stay, and so do Judge 301935 (Thondroril River) and Executioner 213762 (Redpine Dell): neither TDB spawns 1837 or
-- 1841 anywhere, but these two have Cataclysm-era guids and areas, not classic ones, so they are left as an unsourced lead.
DELETE FROM `waypoint_data` WHERE `id` IN (18358,18702,17638,14923);
DELETE FROM `creature_addon` WHERE `guid` IN (18358,18702,17638,14923);
DELETE FROM `pool_creature` WHERE `pool_entry`=132 AND `guid` IN (18568,18578);
DELETE FROM `pool_template` WHERE `entry`=132;
DELETE FROM `creature` WHERE (`guid`,`id`) IN ((18358,1839),(18413,1839),(18436,1841),(18448,1838),(18461,1837),(18568,1884),(18578,1843),(18702,1885),
(17638,1844),(14923,1848));
