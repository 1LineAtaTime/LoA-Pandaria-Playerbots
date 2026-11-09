-- Alina's Reward 26081 had a request text of another quest ("You do this, an' I'm sure Malin'll be more than pleased with you.").
-- Sourced: TDB 442 has no row. The quest has no objectives, so the core goes straight to the reward text and this never showed.
DELETE FROM `quest_request_items` WHERE `ID`=26081;
