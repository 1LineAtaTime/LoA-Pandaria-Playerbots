-- Tarren Mill Deathguard 51916: remove the one static level 85 spawn wandering among the level 30 Dreadguards at Tarren Mill.
-- Sourced: not spawned in TDB 4.4.2 or 1210. Wowhead MoP Classic has no location for the NPC ("The location of this NPC is unknown") and one
-- comment: "They only spawn if an alliance player or attack was detected." The entry and its SmartAI stay.
-- Nothing in this core summons it on an attack; that behaviour is not built.
DELETE FROM `creature` WHERE `guid`=177180 AND `id`=51916;
