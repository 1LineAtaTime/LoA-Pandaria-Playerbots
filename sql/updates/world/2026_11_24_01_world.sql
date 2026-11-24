-- Icy Blue Mechanostrider Replacement (7675) and Palomino Exchange (7678): turn-in NPCs.
-- Problem: both quests have a starter row but no quest ender row, so they can be accepted and completed but never handed in.
-- Change: add the missing creature_questender rows.
--   7675 Icy Blue Mechanostrider Replacement -> Milli Featherwhistle (7955), the NPC that also starts it
--   7678 Palomino Exchange                   -> Katie Hunter (384), the NPC that also starts it
-- Both NPCs are spawned (7955 in Dun Morogh, 384 in Elwynn Forest).
-- Source: Wowhead Classic quest 7675 (start and end: Milli Featherwhistle); for quest 7678 the creature_questender data of
--   other open-source cores lists Katie Hunter as the ender, and she is also the starter in this database.
-- Rollback:
--   DELETE FROM `creature_questender` WHERE (`id`, `quest`) IN ((7955, 7675), (384, 7678));
DELETE FROM `creature_questender` WHERE (`id`, `quest`) IN ((7955, 7675), (384, 7678));
INSERT INTO `creature_questender` (`id`, `quest`) VALUES
(7955, 7675),
(384, 7678);
