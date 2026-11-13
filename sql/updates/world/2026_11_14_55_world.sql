-- Eastern Plaguelands, the Noxious Glade (Argent Call: The Noxious Glade 27465): the Skullmages' Chattering Swarms and the
-- cultists' channel glow.
--  * Chattering Swarm 45683: a Skullmage 45691 summons one on aggro (Summon Chattering Swarm 85243, SummonProperties 67 = pet
--    category: a guardian of the Skullmage). Its VehicleId 1131 made CreatureAISelector give it VehicleAI, which never attacks,
--    so the swarm stood beside the fight. Without it the swarm gets PetAI and attacks its owner's target, and it is unsummoned
--    with its Skullmage (Unit::setDeathState -> RemoveAllControlled). Sourced: both TDBs have VehicleId 0; Wowhead and comment
--    2012-07-07 describe a summoned swarm that attacks.
--  * The six static swarms (guids 372998-373003, wandering the glade with no owner) go. Sourced: neither TDB spawns 45683; it
--    is a summon. Nothing else refers to these guids.
--  * Skullmage 45691 and Noxious Assassin 45692 carried 8734 Blackfathom Channeling (a blue channel glow) on their template.
--    Sourced: neither TDB has it, on the template or on any of their 24 spawns. Skullmages keep their own Shadow Channelling
--    51733 from their SmartAI.
UPDATE `creature_template` SET `VehicleId`=0 WHERE `entry`=45683;
DELETE FROM `creature_addon` WHERE `guid` IN (372998,372999,373000,373001,373002,373003);
DELETE FROM `creature` WHERE `guid` IN (372998,372999,373000,373001,373002,373003) AND `id`=45683;
UPDATE `creature_template_addon` SET `auras`='' WHERE `entry` IN (45691,45692) AND `auras`='8734';
