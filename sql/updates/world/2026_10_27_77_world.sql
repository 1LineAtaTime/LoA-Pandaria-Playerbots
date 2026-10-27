-- Ragefire Chasm: the Lava triggers (61560, 61601) and the Flame Visual (61413) came over from the sniff with unit_flags 0, so the
-- rising lava on the path was a hostile, attackable level 93 elite that aggroed and killed parties walking past it.
-- Give them retail's flags (TDB 1210.26091): the Lava is not selectable and immune to players and NPCs, the Flame Visual is
-- not selectable; the Flame Visual also gets the immune flags so it cannot start a fight either.
UPDATE `creature_template` SET `unit_flags` = 33555200 WHERE `entry` IN (61560, 61601); -- UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_SELECTABLE
UPDATE `creature_template` SET `unit_flags` = 33555200 WHERE `entry` = 61413;
