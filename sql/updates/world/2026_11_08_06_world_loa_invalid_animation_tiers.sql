-- Invalid animation tiers can crash build18414 in its animation fallback table.
-- Correct only invalid split fields; byte3 is the legacy animation tier, not byte1.
-- In particular Soultethered Banshee38473 was sending tier8 in Gilneas.
UPDATE creature_addon SET animTier = IF(((bytes1 >> 24) & 255) < 5, ((bytes1 >> 24) & 255), 0) WHERE animTier >= 5;
UPDATE creature_template_addon SET animTier = IF(((bytes1 >> 24) & 255) < 5, ((bytes1 >> 24) & 255), 0) WHERE animTier >= 5;
UPDATE creature_addon SET standState = 0 WHERE standState >= 10;
UPDATE creature_template_addon SET standState = 0 WHERE standState >= 10;
