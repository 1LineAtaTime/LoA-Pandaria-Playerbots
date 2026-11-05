-- Hillsbrad Foothills, Purgation Isle: the Stormpike Mountaineer 48482, Stormpike Ram Rider 48483 and Bloodfang Sentry 48484
-- attacked a player only after being hit. Their unit_flags 0x8200 carried UNIT_FLAG_IMMUNE_TO_NPC, and Creature::CanStartAttack
-- refuses every target, players included, for a creature with that flag. Retail (TDB 1210 and 442) has unit_flags 0 and
-- unit_flags2 0x800 for all three, and no spawn overrides; take those values. UNIT_FLAG_CAN_SWIM 0x8000 goes too: retail does not
-- carry it, and Creature::RefreshSwimmingFlag sets it in combat for creatures that can enter water. None of the three is an event
-- actor (Heroes of the Horde! summons 48485, 48507, 48508 and 48515, which keep their flags); the spawns have no unit_flags override.
UPDATE `creature_template` SET `unit_flags` = 0, `unit_flags2` = 2048 WHERE `entry` IN (48482, 48483, 48484);
UPDATE `creature` SET `unit_flags` = 0, `unit_flags2` = 0 WHERE `id` IN (48482, 48483, 48484);
