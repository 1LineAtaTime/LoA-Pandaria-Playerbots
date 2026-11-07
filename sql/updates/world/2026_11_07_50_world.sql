-- Eastpoint Tower: the inn's rest. INFERENCE, no table lists it (both TDBs lack it): trigger 6578 is the inn's rest area. Inn "Yes" (warcraft.wiki.gg Eastpoint Tower).
-- AreaTrigger.dbc has it at floor height (-605.65,-1062.59,65.89), radius 25; all three Wowhead pins of Innkeeper Durgens and his TDB spot are inside; no other table or script uses it, and
-- no other trigger stands near; id 6577 next to it is the Sludge Fields tavern, also in Hillsbrad. Can be dropped alone.
DELETE FROM `areatrigger_tavern` WHERE `id`=6578;
INSERT INTO `areatrigger_tavern` (`id`,`name`) VALUES (6578,'Eastpoint Tower');
