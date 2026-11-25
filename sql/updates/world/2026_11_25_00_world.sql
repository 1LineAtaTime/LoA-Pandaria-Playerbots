-- Golden Lotus dailies (Jade Forest, Mistfall Village and Golden Pagoda): quest giver and turn-in rows for eight daily quests.
-- Before: none of these quests has a creature_queststarter row, so they cannot be picked up at the NPC the quest data names,
-- and five of them have no creature_questender row either (30194, 30263 and 30296 already have theirs and are not touched).
-- Giver and turn-in NPC per quest (one spawn each on map 870, npcflag has the questgiver bit):
--   30191 Steer Clear of the Beer Here  - Mayor Shiyo 58819
--   30194 Encroaching Storm             - Che Wildwalker 59338 (starter only)
--   30195 Blooming Blossoms             - Merchant Benny 58820
--   30231 Pomfruit Pickup               - Cook Tope 58818
--   30263 Clearing in the Forest        - Che Wildwalker 59338 (starter only)
--   30296 Gaohun the Soul-Severer       - Rook Stonetoe 58911 (starter only)
--   30297 Baolai the Immolator          - Rook Stonetoe 58911
--   30444 No Reprieve                   - Anji Autumnlight 58919
-- Source: quest pages of the Wowhead MoP Classic database (Start and End NPC of each quest).
-- Rollback:
--   DELETE FROM `creature_queststarter` WHERE (`id`,`quest`) IN ((58819,30191),(59338,30194),(58820,30195),(58818,30231),(59338,30263),(58911,30296),(58911,30297),(58919,30444));
--   DELETE FROM `creature_questender` WHERE (`id`,`quest`) IN ((58819,30191),(58820,30195),(58818,30231),(58911,30297),(58919,30444));

DELETE FROM `creature_queststarter` WHERE (`id`,`quest`) IN ((58819,30191),(59338,30194),(58820,30195),(58818,30231),(59338,30263),(58911,30296),(58911,30297),(58919,30444));
INSERT INTO `creature_queststarter` (`id`,`quest`) VALUES
(58819,30191),
(59338,30194),
(58820,30195),
(58818,30231),
(59338,30263),
(58911,30296),
(58911,30297),
(58919,30444);

DELETE FROM `creature_questender` WHERE (`id`,`quest`) IN ((58819,30191),(58820,30195),(58818,30231),(58911,30297),(58919,30444));
INSERT INTO `creature_questender` (`id`,`quest`) VALUES
(58819,30191),
(58820,30195),
(58818,30231),
(58911,30297),
(58919,30444);
