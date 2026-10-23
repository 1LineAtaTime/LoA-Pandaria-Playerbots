-- issue #105: quest 24589 ("Lord Jaraxxus Must Die!") shipped a quest_poi_points row whose
-- coordinates are the INT32_MIN sentinel (-2147483648, -2147483648). A bot travelling to that quest
-- POI averaged it into a position outside the world, and the resulting grid lookup tripped the
-- ASSERT in Map::getNGrid and killed the worldserver.
--
-- The other point of the quest (5697, 573) is the real one, so the invalid blob point is deleted.
-- The code side (Map::GetGrid) now refuses out-of-world lookups regardless, and the bot module skips
-- unusable POI points, so no bad row can crash the server again.
DELETE FROM `quest_poi_points`
 WHERE `QuestID` = 24589 AND `X` = -2147483648 AND `Y` = -2147483648;
