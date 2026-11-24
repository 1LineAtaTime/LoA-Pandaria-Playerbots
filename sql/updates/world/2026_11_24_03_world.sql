-- Gardener Fran (62377): the "hops" gossip option never gives the Hop Hunting (30053) kill credit.
-- smart_scripts 62377 id 4 (event 62, gossip select) matches event_param1 = 62377 (the creature entry) and
-- event_param2 = 1, but the creature uses gossip_menu_id 13850, whose single option (OptionID 0) is the hops option.
-- The event never fires. Same pattern as Gai Lan 57385 (menu 13334, option 0): event_param1 = menu id, event_param2 = option id.
UPDATE `smart_scripts` SET `event_param1` = 13850, `event_param2` = 0 WHERE `entryorguid` = 62377 AND `source_type` = 0 AND `id` = 4;
