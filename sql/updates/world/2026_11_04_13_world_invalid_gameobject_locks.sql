-- Restore only invalid lock fields with a matching 5.4.8 source identity.
-- Source: ProjectSkyfire/SkyFire_548 release sf_db_26, asset
-- SFDB_full_548_26.002_2026_008_18_Release.zip, SHA256
-- d12ba9ecad381f3b5460612af2a172cd373afdd796c289e2ccc11315db21dd5f.
-- Same entry, type, display and name verified; replacement is 0 (no lock) or
-- present in installed Lock.dbc. All other fields and custom rows are retained.
-- https://github.com/ProjectSkyfire/SkyFire_548/releases/tag/sf_db_26
UPDATE gameobject_template SET data0=0 WHERE entry=202357 AND type=2 AND displayId=9321 AND name=CONVERT(0x4472616b7572752773204c6173742057697368 USING utf8mb4) AND data0=17334; -- Drakuru's Last Wish
UPDATE gameobject_template SET data1=0 WHERE entry=211963 AND type=0 AND displayId=11634 AND name=CONVERT(0x476f6c64656e2047617465 USING utf8mb4) AND data1=5705968; -- Golden Gate
UPDATE gameobject_template SET data1=0 WHERE entry=211976 AND type=0 AND displayId=11041 AND name=CONVERT(0x446f6f6461645f50415f47686f7374427265776572795f446f6f725f303132 USING utf8mb4) AND data1=5705968; -- Doodad_PA_GhostBrewery_Door_012
UPDATE gameobject_template SET data1=0 WHERE entry=211977 AND type=0 AND displayId=11041 AND name=CONVERT(0x446f6f6461645f50415f47686f7374427265776572795f446f6f725f303133 USING utf8mb4) AND data1=5705968; -- Doodad_PA_GhostBrewery_Door_013
UPDATE gameobject_template SET data1=0 WHERE entry=211981 AND type=0 AND displayId=11041 AND name=CONVERT(0x446f6f6461645f50415f47686f7374427265776572795f446f6f725f303135 USING utf8mb4) AND data1=5705968; -- Doodad_PA_GhostBrewery_Door_015
UPDATE gameobject_template SET data1=0 WHERE entry=211982 AND type=0 AND displayId=11041 AND name=CONVERT(0x446f6f6461645f50415f47686f7374427265776572795f446f6f725f303134 USING utf8mb4) AND data1=5705968; -- Doodad_PA_GhostBrewery_Door_014
UPDATE gameobject_template SET data1=0 WHERE entry=213090 AND type=0 AND displayId=11897 AND name=CONVERT(0x446f6f6461645f50415f4561737454656d706c655f446f6f72546f776572303031 USING utf8mb4) AND data1=5743776; -- Doodad_PA_EastTemple_DoorTower001
UPDATE gameobject_template SET data1=0 WHERE entry=213091 AND type=0 AND displayId=11897 AND name=CONVERT(0x446f6f6461645f50415f4561737454656d706c655f446f6f72546f776572303032 USING utf8mb4) AND data1=5743776; -- Doodad_PA_EastTemple_DoorTower002
UPDATE gameobject_template SET data1=0 WHERE entry=213268 AND type=0 AND displayId=11896 AND name=CONVERT(0x446f6f6461645f50415f4561737454656d706c655f446f6f72526f756e64303031 USING utf8mb4) AND data1=5749660; -- Doodad_PA_EastTemple_DoorRound001
UPDATE gameobject_template SET data1=0 WHERE entry=213269 AND type=0 AND displayId=11896 AND name=CONVERT(0x446f6f6461645f50415f4561737454656d706c655f446f6f72526f756e64303032 USING utf8mb4) AND data1=5749660; -- Doodad_PA_EastTemple_DoorRound002
UPDATE gameobject_template SET data1=0 WHERE entry=213271 AND type=0 AND displayId=11896 AND name=CONVERT(0x446f6f6461645f50415f4561737454656d706c655f446f6f72526f756e64303034 USING utf8mb4) AND data1=5749660; -- Doodad_PA_EastTemple_DoorRound004
UPDATE gameobject_template SET data1=0 WHERE entry=213395 AND type=0 AND displayId=11879 AND name=CONVERT(0x446f6f6461645f477265617457616c6c5f446f6f725f303031 USING utf8mb4) AND data1=5749660; -- Doodad_GreatWall_Door_001
UPDATE gameobject_template SET data1=0 WHERE entry=213396 AND type=0 AND displayId=11879 AND name=CONVERT(0x446f6f6461645f477265617457616c6c5f446f6f725f303032 USING utf8mb4) AND data1=5749660; -- Doodad_GreatWall_Door_002
UPDATE gameobject_template SET data1=0 WHERE entry=213397 AND type=0 AND displayId=11879 AND name=CONVERT(0x446f6f6461645f477265617457616c6c5f446f6f725f303034 USING utf8mb4) AND data1=5749660; -- Doodad_GreatWall_Door_004
UPDATE gameobject_template SET data1=0 WHERE entry=213398 AND type=0 AND displayId=11879 AND name=CONVERT(0x446f6f6461645f477265617457616c6c5f446f6f725f303035 USING utf8mb4) AND data1=5749660; -- Doodad_GreatWall_Door_005
UPDATE gameobject_template SET data1=0 WHERE entry=215130 AND type=0 AND displayId=12703 AND name=CONVERT(0x446f6f6461645f477265617457616c6c5f446f6f725f303138 USING utf8mb4) AND data1=5756548; -- Doodad_GreatWall_Door_018
UPDATE gameobject_template SET data1=0 WHERE entry=215364 AND type=0 AND displayId=12703 AND name=CONVERT(0x446f6f6461645f477265617457616c6c5f446f6f725f303230 USING utf8mb4) AND data1=5756548; -- Doodad_GreatWall_Door_020
UPDATE gameobject_template SET data1=0 WHERE entry=216354 AND type=0 AND displayId=13418 AND name=CONVERT(0x446f6f6461645f50415f416c6c69616e63655f4172656e615f47617465303035 USING utf8mb4) AND data1=5773332; -- Doodad_PA_Alliance_Arena_Gate005
UPDATE gameobject_template SET data1=0 WHERE entry=216355 AND type=0 AND displayId=13418 AND name=CONVERT(0x446f6f6461645f50415f416c6c69616e63655f4172656e615f47617465303034 USING utf8mb4) AND data1=5773332; -- Doodad_PA_Alliance_Arena_Gate004
UPDATE gameobject_template SET data1=0 WHERE entry=216356 AND type=0 AND displayId=13418 AND name=CONVERT(0x446f6f6461645f50415f416c6c69616e63655f4172656e615f47617465303033 USING utf8mb4) AND data1=5773332; -- Doodad_PA_Alliance_Arena_Gate003
UPDATE gameobject_template SET data1=0 WHERE entry=216357 AND type=0 AND displayId=13418 AND name=CONVERT(0x446f6f6461645f50415f416c6c69616e63655f4172656e615f47617465303032 USING utf8mb4) AND data1=5773332; -- Doodad_PA_Alliance_Arena_Gate002
UPDATE gameobject_template SET data1=0 WHERE entry=216358 AND type=0 AND displayId=13418 AND name=CONVERT(0x446f6f6461645f50415f416c6c69616e63655f4172656e615f47617465303031 USING utf8mb4) AND data1=5773332; -- Doodad_PA_Alliance_Arena_Gate001
UPDATE gameobject_template SET data1=0 WHERE entry=216359 AND type=0 AND displayId=13418 AND name=CONVERT(0x446f6f6461645f50415f416c6c69616e63655f4172656e615f47617465303036 USING utf8mb4) AND data1=5773332; -- Doodad_PA_Alliance_Arena_Gate006
