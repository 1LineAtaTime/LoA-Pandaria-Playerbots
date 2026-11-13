-- The existing EmuCoach column has no default, while AccountMgr creates
-- Playerbots accounts without specifying it. Preserve all existing values.
ALTER TABLE account ALTER COLUMN votepoints SET DEFAULT '0';
