-- The scripted static800100 scene replaces this older moving transport.
-- Both share display8253 and overlap in the quest area. The moving ship is
-- globally sent (no phase rule), so it could appear through the static deck
-- and attach a player who should remain on the fixed encounter geometry.
-- Remove only its world spawn; preserve templates, paths, crew and other ships.
-- Apply with the rope/world-space disembarkation native repair; restart world.
DELETE FROM transports WHERE entry=203428;
