-- Restore authored Gilneas graveyard links from matching LoA v1.0.4.
-- WorldSafeLocs1692..1695 were verified present on map654 in matching client data.
-- Existing keys/custom faction assignments win. World schema only.
INSERT IGNORE INTO game_graveyard_zone(id,ghost_zone,faction) VALUES
(1692,4755,0),(1693,4755,0),(1694,4755,0),(1695,4755,0);
