-- Refs #712
-- MoP PvP quartermasters 69971, 69973, 69974, 69975, 69977, 69978 (Xazi Smolderpipe, Zom Bocom, T'Maire Sydes,
-- Dirgehammer, Zar'shi, Thunderhorn): 972 npc_vendor rows carry ExtendedCost 5962 / 5963 / 5964 / 5966. The core's
-- ItemExtendedCost store (ItemExtendedCost.db2, build 18414) holds ids 1..5280 only, so ObjectMgr::IsVendorItemValid
-- logs "Item (Entry: x) with wrong ExtendedCost (n) for vendor (v), ignore" for every row and the vendors list nothing.
-- 5965 is not used. The ids exist in no MoP 5.4.8 table either (npc_vendor has no row above 5280 for
-- these items), so they are placeholders from the upstream LoA vendor import, not DB2 data to be added.
--
-- Fix: keep every row (nothing deleted), point each at the real ItemExtendedCost id that the working vendors use for
-- the SAME item (item + type), chosen per row in this order:
--   A (644 rows) the id used by the MoP/LoA "Honor Quartermaster" / "Conquest Quartermaster" vendors that already
--     work in the world (e.g. 52534-52537, 52540, 52546, 52549, 54651-54662, 65164-65167, 71617-71624, 73141-73148).
--     Unique per item on this data set. These are the real MoP-era costs (honor points, with the 725000/780000 total
--     honor gate on weapons).
--   B (289 rows) no such vendor sells the item: the pure-honor id (currency 392 only, no arena rating / item / flag) used
--     by the legacy vendors (Legacy Armor/Arena Armor/Weapon Quartermaster, Arena Vendor, Apprentice/Veteran Armor
--     Quartermaster), the CHEAPEST when several exist (this drops the inflated "Veteran" ids 26xx-27xx and the Justice
--     currency 395 ids 2740/2742).
--   C (39 rows) only a rating-gated id exists (Relentless weapons/shields/wands: 2693, 2695, 2698, 2700, 2701, each
--     RequiredPersonalArenaRating 1800, as in the original WotLK season): kept as that id.
-- Source of truth: world.npc_vendor + world.itemextendedcost_18414 on the test DB, cross-checked against
-- the MoP 5.4.8 npc_vendor (same rows for the legacy and 5xxxx quartermasters). The six vendors are unspawned
-- (npcflag 0, 0 creature rows), so this changes no live behaviour until they are given npcflag 128 and a spawn; the
-- spawned working equivalents are 54651-54662 / 73141-73148.
--
-- Each UPDATE matches the OLD id, so the file is idempotent. The 972 rows have no key collision after the change
-- (checked: PK entry,item,ExtendedCost,type).
-- BACKUP (before applying):
--   mysqldump -h<host> -u<user> -p --no-create-info --skip-extended-insert world npc_vendor --where="entry IN (69971,69973,69974,69975,69977,69978)" > npc_vendor-pvp-qm-before-712.sql
-- ROLLBACK: restore from that dump (DELETE FROM npc_vendor WHERE entry IN (...) first), or run the rollback file kept next to it.
-- VERIFY: SELECT COUNT(*) FROM npc_vendor WHERE entry IN (69971,69973,69974,69975,69977,69978) AND ExtendedCost > 5280;  -- 0
--         and no "wrong ExtendedCost ... for vendor (6997x)" lines in DBErrors.log after a restart (.reload vendors does not clear them).


-- 69971 Xazi Smolderpipe (Conquest)
UPDATE `npc_vendor` SET `ExtendedCost` = 2964 WHERE `entry` = 69971 AND `type` = 1 AND `ExtendedCost` = 5962 AND `item` IN (51330,51332,51334,51346,51348,51354,51356); -- B 7 rows, cur392=72500
UPDATE `npc_vendor` SET `ExtendedCost` = 2965 WHERE `entry` = 69971 AND `type` = 1 AND `ExtendedCost` = 5962 AND `item` IN (51327,51337,51340,51343,51350,51359,51362,51365,51368,51371,51374); -- B 11 rows, cur392=94500
UPDATE `npc_vendor` SET `ExtendedCost` = 3374 WHERE `entry` = 69971 AND `type` = 1 AND `ExtendedCost` = 5962 AND `item` IN (51329,51339,51342,51345,51352,51361,51364,51367,51370,51373,51376); -- B 11 rows, cur392=60000
UPDATE `npc_vendor` SET `ExtendedCost` = 2950 WHERE `entry` = 69971 AND `type` = 1 AND `ExtendedCost` = 5963 AND `item` IN (51396,51407,51408,51409,51452,51455,51533); -- B 7 rows, cur392=19000
UPDATE `npc_vendor` SET `ExtendedCost` = 2954 WHERE `entry` = 69971 AND `type` = 1 AND `ExtendedCost` = 5963 AND `item` IN (51414,51418,51420,51424,51426,51430,51434,51438,51459,51462,51464,51467,51469,51473,51475,51479,51483,51486,51488,51491,51493,51496,51498,51502,51504,51508,51510,51514,51537,51540,51542,51545); -- B 32 rows, cur392=104000
UPDATE `npc_vendor` SET `ExtendedCost` = 2965 WHERE `entry` = 69971 AND `type` = 1 AND `ExtendedCost` = 5963 AND `item` IN (51328,51338,51341,51344,51351,51360,51363,51366,51369,51372,51375); -- B 11 rows, cur392=94500
UPDATE `npc_vendor` SET `ExtendedCost` = 3163 WHERE `entry` = 69971 AND `type` = 1 AND `ExtendedCost` = 5964 AND `item` IN (51413,51415,51416,51419,51421,51422,51425,51427,51428,51433,51435,51436,51458,51460,51461,51463,51465,51466,51468,51470,51471,51474,51476,51477,51482,51484,51485,51487,51489,51490,51492,51494,51495,51497,51499,51500,51503,51505,51506,51509,51511,51512,51536,51538,51539,51541,51543,51544); -- B 48 rows, cur392=165000
UPDATE `npc_vendor` SET `ExtendedCost` = 2936 WHERE `entry` = 69971 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (51388,51390,51392,51394,51400,51402,51404,51411,51431,51449,51456,51480); -- B 12 rows, cur392=255000
UPDATE `npc_vendor` SET `ExtendedCost` = 2943 WHERE `entry` = 69971 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (51397,51406,51453); -- B 3 rows, cur392=183000
UPDATE `npc_vendor` SET `ExtendedCost` = 2945 WHERE `entry` = 69971 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (51515,51517,51519,51521,51523); -- B 5 rows, cur392=178500
UPDATE `npc_vendor` SET `ExtendedCost` = 2946 WHERE `entry` = 69971 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (51439,51441,51443,51445,51447,51525,51527,51530); -- B 8 rows, cur392=76000
UPDATE `npc_vendor` SET `ExtendedCost` = 2951 WHERE `entry` = 69971 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (51410,51451,51531,51532); -- B 4 rows, cur392=26000

-- 69973 Zom Bocom (Honor)
UPDATE `npc_vendor` SET `ExtendedCost` = 2966 WHERE `entry` = 69973 AND `type` = 1 AND `ExtendedCost` = 5962 AND `item` IN (40883,40978,41052,41071,41236,41618,41631,41833,41882,41899,49179); -- B 11 rows, cur392=63000
UPDATE `npc_vendor` SET `ExtendedCost` = 2968 WHERE `entry` = 69973 AND `type` = 1 AND `ExtendedCost` = 5962 AND `item` IN (42076,42077,42078,42079,42080,42081,42082); -- B 7 rows, cur392=48500
UPDATE `npc_vendor` SET `ExtendedCost` = 3373 WHERE `entry` = 69973 AND `type` = 1 AND `ExtendedCost` = 5962 AND `item` IN (40890,40984,41061,41066,41226,41626,41641,41841,41894,41910,49181); -- B 11 rows, cur392=40000
UPDATE `npc_vendor` SET `ExtendedCost` = 2701 WHERE `entry` = 69973 AND `type` = 1 AND `ExtendedCost` = 5963 AND `item` IN (42527,42533,42539,42561,42566,42572,49187); -- C 7 rows, cur392=25650 arenarating>=1800
UPDATE `npc_vendor` SET `ExtendedCost` = 2955 WHERE `entry` = 69973 AND `type` = 1 AND `ExtendedCost` = 5963 AND `item` IN (40870,40871,40872,40964,41039,41045,41212,41218,41276,41282,41684,41716,41870,41935,41966,42012); -- B 16 rows, cur392=69500
UPDATE `npc_vendor` SET `ExtendedCost` = 2957 WHERE `entry` = 69973 AND `type` = 1 AND `ExtendedCost` = 5963 AND `item` IN (40810,40811,40812,40928,41002,41008,41138,41144,41288,41294,41768,41774,41875,41941,41972,42018); -- B 16 rows, cur392=69500
UPDATE `npc_vendor` SET `ExtendedCost` = 2966 WHERE `entry` = 69973 AND `type` = 1 AND `ExtendedCost` = 5963 AND `item` IN (40884,40979,41056,41076,41231,41622,41636,41837,41886,41904,49183); -- B 11 rows, cur392=63000
UPDATE `npc_vendor` SET `ExtendedCost` = 2956 WHERE `entry` = 69973 AND `type` = 1 AND `ExtendedCost` = 5964 AND `item` IN (40790,40791,40792,40829,40830,40831,40850,40851,40852,40910,40934,40940,40994,40995,41014,41020,41028,41034,41082,41088,41152,41158,41200,41206,41299,41305,41311,41317,41322,41328,41651,41656,41662,41668,41673,41679,41855,41860,41865,41916,41922,41928,41947,41954,41960,41994,41999,42006); -- B 48 rows, cur392=110000
UPDATE `npc_vendor` SET `ExtendedCost` = 2693 WHERE `entry` = 69973 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (42319,42324,42329,42334,42366,42386,42392,42487,42492,42498,44423,44424); -- C 12 rows, cur392=953033 arenarating>=1800
UPDATE `npc_vendor` SET `ExtendedCost` = 2695 WHERE `entry` = 69973 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (42262,42348,42354,49189); -- C 4 rows, cur392=685933 arenarating>=1800
UPDATE `npc_vendor` SET `ExtendedCost` = 2698 WHERE `entry` = 69973 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (42210,42229,42234,42244,42250,42257,42267,42272,42277,42282,42287,42292); -- C 12 rows, cur392=344050 arenarating>=1800
UPDATE `npc_vendor` SET `ExtendedCost` = 2700 WHERE `entry` = 69973 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (42504,42515,42521,49185); -- C 4 rows, cur392=98325 arenarating>=1800

-- 69974 Knight-Lieutenant T'Maire Sydes (Honor)
UPDATE `npc_vendor` SET `ExtendedCost` = 3163 WHERE `entry` = 69974 AND `type` = 1 AND `ExtendedCost` = 5962 AND `item` IN (70319,70320,70326,70328,70331,70332,70336,70339,70343,70346,70347,70349,70360,70362,70368); -- A 15 rows, cur392=165000
UPDATE `npc_vendor` SET `ExtendedCost` = 3344 WHERE `entry` = 69974 AND `type` = 1 AND `ExtendedCost` = 5962 AND `item` IN (70322,70325,70327,70330,70334,70338,70340,70342,70345,70350,70352,70359,70363,70365,70366,70383,70384,70385,70386,70387,70388,70389); -- A 22 rows, cur392=125000
UPDATE `npc_vendor` SET `ExtendedCost` = 3163 WHERE `entry` = 69974 AND `type` = 1 AND `ExtendedCost` = 5963 AND `item` IN (70245,70248,70250,70253,70255,70258,70260,70263,70265,70268,70270,70273,70275,70278,70279,70283,70284,70288,70289,70293,70295,70298,70299,70303,70304,70308,70309,70313,70321,70323,70324,70329,70333,70335,70337,70341,70344,70348,70351,70354,70357,70358,70361,70364,70367); -- A 45 rows, cur392=165000
UPDATE `npc_vendor` SET `ExtendedCost` = 3551 WHERE `entry` = 69974 AND `type` = 1 AND `ExtendedCost` = 5963 AND `item` IN (70239,70240,70241,70242,70243); -- A 5 rows, cur392=95000+cur392=725000
UPDATE `npc_vendor` SET `ExtendedCost` = 4306 WHERE `entry` = 69974 AND `type` = 1 AND `ExtendedCost` = 5963 AND `item` IN (70314,70318); -- B 2 rows, cur392=82500
UPDATE `npc_vendor` SET `ExtendedCost` = 2956 WHERE `entry` = 69974 AND `type` = 1 AND `ExtendedCost` = 5964 AND `item` IN (70315,70316,70317); -- B 3 rows, cur392=110000
UPDATE `npc_vendor` SET `ExtendedCost` = 3382 WHERE `entry` = 69974 AND `type` = 1 AND `ExtendedCost` = 5964 AND `item` IN (70244,70246,70247,70249,70251,70252,70254,70256,70257,70259,70261,70262,70264,70266,70267,70269,70271,70272,70274,70276,70277,70280,70281,70282,70285,70286,70287,70290,70291,70292,70294,70296,70297,70300,70301,70302,70305,70306,70307,70310,70311,70312,70353,70355,70356); -- A 45 rows, cur392=220000
UPDATE `npc_vendor` SET `ExtendedCost` = 3549 WHERE `entry` = 69974 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (70211,70212,70214,70216,70221,70222,70223,70229,70230); -- A 9 rows, cur392=245000+cur392=725000
UPDATE `npc_vendor` SET `ExtendedCost` = 3550 WHERE `entry` = 69974 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (70213,70217,70220,70224,70225,70226,70227,70228,70231,70236,70237,70238); -- A 12 rows, cur392=340000+cur392=725000
UPDATE `npc_vendor` SET `ExtendedCost` = 3551 WHERE `entry` = 69974 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (70215,70218,70219); -- A 3 rows, cur392=95000+cur392=725000
UPDATE `npc_vendor` SET `ExtendedCost` = 3552 WHERE `entry` = 69974 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (70234,70235); -- A 2 rows, cur392=70000+cur392=725000

-- 69975 Captain Dirgehammer (Conquest)
UPDATE `npc_vendor` SET `ExtendedCost` = 3168 WHERE `entry` = 69975 AND `type` = 1 AND `ExtendedCost` = 5962 AND `item` IN (73494,73495,73518,73519,73528,73529,73550,73551,73561,73562,73585,73586,73600,73608,73628,73629,73630,73631,73632,73633,73646,73647); -- A 22 rows, cur392=125000
UPDATE `npc_vendor` SET `ExtendedCost` = 3169 WHERE `entry` = 69975 AND `type` = 1 AND `ExtendedCost` = 5962 AND `item` IN (73507,73522,73532,73533,73554,73555,73565,73566,73589,73590,73602,73610,73637,73638,73639); -- A 15 rows, cur392=165000
UPDATE `npc_vendor` SET `ExtendedCost` = 3169 WHERE `entry` = 69975 AND `type` = 1 AND `ExtendedCost` = 5963 AND `item` IN (73478,73481,73483,73487,73502,73505,73508,73511,73513,73516,73520,73521,73523,73526,73530,73531,73540,73544,73545,73549,73552,73553,73556,73559,73563,73564,73567,73570,73572,73576,73580,73583,73587,73588,73595,73599,73601,73603,73607,73609,73611,73615,73616,73619,73634,73635,73636); -- A 47 rows, cur392=165000
UPDATE `npc_vendor` SET `ExtendedCost` = 3417 WHERE `entry` = 69975 AND `type` = 1 AND `ExtendedCost` = 5963 AND `item` IN (73446,73458,73465,73468,73469); -- A 5 rows, cur392=95000+cur392=780000
UPDATE `npc_vendor` SET `ExtendedCost` = 3162 WHERE `entry` = 69975 AND `type` = 1 AND `ExtendedCost` = 5964 AND `item` IN (88170); -- A 1 rows, cur392=220000
UPDATE `npc_vendor` SET `ExtendedCost` = 3170 WHERE `entry` = 69975 AND `type` = 1 AND `ExtendedCost` = 5964 AND `item` IN (73479,73480,73482,73484,73485,73486,73503,73504,73506,73509,73510,73512,73514,73515,73517,73524,73525,73527,73541,73542,73543,73546,73547,73548,73557,73558,73560,73568,73569,73571,73573,73574,73575,73581,73582,73584,73596,73597,73598,73604,73605,73606,73612,73613,73614,73617,73618,73620); -- A 48 rows, cur392=220000
UPDATE `npc_vendor` SET `ExtendedCost` = 3415 WHERE `entry` = 69975 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (73456,73457,73460,73462,73463,73466,73470,73475,73476,73477); -- A 10 rows, cur392=340000+cur392=780000
UPDATE `npc_vendor` SET `ExtendedCost` = 3416 WHERE `entry` = 69975 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (73447,73448,73449,73452,73454,73455,73459,73467,73472,73473,73474); -- A 11 rows, cur392=245000+cur392=780000
UPDATE `npc_vendor` SET `ExtendedCost` = 3417 WHERE `entry` = 69975 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (73451,73453,73461); -- A 3 rows, cur392=95000+cur392=780000
UPDATE `npc_vendor` SET `ExtendedCost` = 3652 WHERE `entry` = 69975 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (73450,73464); -- A 2 rows, cur392=70000+cur392=780000

-- 69977 Blood Guard Zar'shi (Honor)
UPDATE `npc_vendor` SET `ExtendedCost` = 3163 WHERE `entry` = 69977 AND `type` = 1 AND `ExtendedCost` = 5962 AND `item` IN (70319,70320,70326,70328,70331,70332,70336,70339,70343,70346,70347,70349,70360,70362,70368); -- A 15 rows, cur392=165000
UPDATE `npc_vendor` SET `ExtendedCost` = 3344 WHERE `entry` = 69977 AND `type` = 1 AND `ExtendedCost` = 5962 AND `item` IN (70322,70325,70327,70330,70334,70338,70340,70342,70345,70350,70352,70359,70363,70365,70366,70383,70384,70385,70386,70387,70388,70389); -- A 22 rows, cur392=125000
UPDATE `npc_vendor` SET `ExtendedCost` = 3163 WHERE `entry` = 69977 AND `type` = 1 AND `ExtendedCost` = 5963 AND `item` IN (70245,70248,70250,70253,70255,70258,70260,70263,70265,70268,70270,70273,70275,70278,70279,70283,70284,70288,70289,70293,70295,70298,70299,70303,70304,70308,70309,70313,70321,70323,70324,70329,70333,70335,70337,70341,70344,70348,70351,70354,70357,70358,70361,70364,70367); -- A 45 rows, cur392=165000
UPDATE `npc_vendor` SET `ExtendedCost` = 3551 WHERE `entry` = 69977 AND `type` = 1 AND `ExtendedCost` = 5963 AND `item` IN (70239,70240,70241,70242,70243); -- A 5 rows, cur392=95000+cur392=725000
UPDATE `npc_vendor` SET `ExtendedCost` = 4306 WHERE `entry` = 69977 AND `type` = 1 AND `ExtendedCost` = 5963 AND `item` IN (70314,70318); -- B 2 rows, cur392=82500
UPDATE `npc_vendor` SET `ExtendedCost` = 2956 WHERE `entry` = 69977 AND `type` = 1 AND `ExtendedCost` = 5964 AND `item` IN (70315,70316,70317); -- B 3 rows, cur392=110000
UPDATE `npc_vendor` SET `ExtendedCost` = 3382 WHERE `entry` = 69977 AND `type` = 1 AND `ExtendedCost` = 5964 AND `item` IN (70244,70246,70247,70249,70251,70252,70254,70256,70257,70259,70261,70262,70264,70266,70267,70269,70271,70272,70274,70276,70277,70280,70281,70282,70285,70286,70287,70290,70291,70292,70294,70296,70297,70300,70301,70302,70305,70306,70307,70310,70311,70312,70353,70355,70356); -- A 45 rows, cur392=220000
UPDATE `npc_vendor` SET `ExtendedCost` = 3549 WHERE `entry` = 69977 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (70211,70212,70214,70216,70221,70222,70223,70229,70230); -- A 9 rows, cur392=245000+cur392=725000
UPDATE `npc_vendor` SET `ExtendedCost` = 3550 WHERE `entry` = 69977 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (70213,70217,70220,70224,70225,70226,70227,70228,70231,70236,70237,70238); -- A 12 rows, cur392=340000+cur392=725000
UPDATE `npc_vendor` SET `ExtendedCost` = 3551 WHERE `entry` = 69977 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (70215,70218,70219); -- A 3 rows, cur392=95000+cur392=725000
UPDATE `npc_vendor` SET `ExtendedCost` = 3552 WHERE `entry` = 69977 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (70234,70235); -- A 2 rows, cur392=70000+cur392=725000

-- 69978 Sergeant Thunderhorn (Conquest)
UPDATE `npc_vendor` SET `ExtendedCost` = 3168 WHERE `entry` = 69978 AND `type` = 1 AND `ExtendedCost` = 5962 AND `item` IN (73494,73495,73518,73519,73528,73529,73550,73551,73561,73562,73585,73586,73600,73608,73628,73629,73630,73631,73632,73633,73646,73647); -- A 22 rows, cur392=125000
UPDATE `npc_vendor` SET `ExtendedCost` = 3169 WHERE `entry` = 69978 AND `type` = 1 AND `ExtendedCost` = 5962 AND `item` IN (73507,73522,73532,73533,73554,73555,73565,73566,73589,73590,73602,73610,73637,73638,73639); -- A 15 rows, cur392=165000
UPDATE `npc_vendor` SET `ExtendedCost` = 3169 WHERE `entry` = 69978 AND `type` = 1 AND `ExtendedCost` = 5963 AND `item` IN (73478,73481,73483,73487,73502,73505,73508,73511,73513,73516,73520,73521,73523,73526,73530,73531,73540,73544,73545,73549,73552,73553,73556,73559,73563,73564,73567,73570,73572,73576,73580,73583,73587,73588,73595,73599,73601,73603,73607,73609,73611,73615,73616,73619,73634,73635,73636); -- A 47 rows, cur392=165000
UPDATE `npc_vendor` SET `ExtendedCost` = 3417 WHERE `entry` = 69978 AND `type` = 1 AND `ExtendedCost` = 5963 AND `item` IN (73446,73458,73465,73468,73469); -- A 5 rows, cur392=95000+cur392=780000
UPDATE `npc_vendor` SET `ExtendedCost` = 3162 WHERE `entry` = 69978 AND `type` = 1 AND `ExtendedCost` = 5964 AND `item` IN (88170); -- A 1 rows, cur392=220000
UPDATE `npc_vendor` SET `ExtendedCost` = 3170 WHERE `entry` = 69978 AND `type` = 1 AND `ExtendedCost` = 5964 AND `item` IN (73479,73480,73482,73484,73485,73486,73503,73504,73506,73509,73510,73512,73514,73515,73517,73524,73525,73527,73541,73542,73543,73546,73547,73548,73557,73558,73560,73568,73569,73571,73573,73574,73575,73581,73582,73584,73596,73597,73598,73604,73605,73606,73612,73613,73614,73617,73618,73620); -- A 48 rows, cur392=220000
UPDATE `npc_vendor` SET `ExtendedCost` = 3415 WHERE `entry` = 69978 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (73456,73457,73460,73462,73463,73466,73470,73475,73476,73477); -- A 10 rows, cur392=340000+cur392=780000
UPDATE `npc_vendor` SET `ExtendedCost` = 3416 WHERE `entry` = 69978 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (73447,73448,73449,73452,73454,73455,73459,73467,73472,73473,73474); -- A 11 rows, cur392=245000+cur392=780000
UPDATE `npc_vendor` SET `ExtendedCost` = 3417 WHERE `entry` = 69978 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (73451,73453,73461); -- A 3 rows, cur392=95000+cur392=780000
UPDATE `npc_vendor` SET `ExtendedCost` = 3652 WHERE `entry` = 69978 AND `type` = 1 AND `ExtendedCost` = 5966 AND `item` IN (73450,73464); -- A 2 rows, cur392=70000+cur392=780000
