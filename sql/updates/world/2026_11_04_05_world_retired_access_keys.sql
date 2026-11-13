-- Match ObjectMgr::LoadAccessRequirements: nonexistent key items are already
-- cleared in memory. Persist that effective configuration, preserving all valid
-- item keys, quest/achievement gates, difficulty, and level requirements.
UPDATE `access_requirement` a
LEFT JOIN `item_template` i ON i.entry=a.item
SET a.item=0 WHERE a.item<>0 AND i.entry IS NULL;
UPDATE `access_requirement` a
LEFT JOIN `item_template` i ON i.entry=a.item2
SET a.item2=0 WHERE a.item2<>0 AND i.entry IS NULL;
