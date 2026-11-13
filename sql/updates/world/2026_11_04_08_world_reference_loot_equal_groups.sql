-- These complete templates contain only equal-chance (chance=0) item choices,
-- but groupid=0 makes every row invalid in LootStoreItem::IsValid. None loads,
-- which also makes valid references to these templates appear missing.
-- Declare one equal-chance group. Do NOT turn every row into a guaranteed drop.
-- Item IDs, chances, counts, difficulty modes and referencing rows are unchanged.
-- Skip an entire template if another installation has changed its definitions.
UPDATE `reference_loot_template` l
JOIN (
    SELECT entry FROM (
        SELECT entry FROM `reference_loot_template`
        WHERE entry IN (82241,87695,90741,90818,95343,95601,95602,104271,104272,104273,105826)
        GROUP BY entry
        HAVING MIN(groupid)=0 AND MAX(groupid)=0
            AND MIN(ChanceOrQuestChance)=0 AND MAX(ChanceOrQuestChance)=0
            AND MIN(mincountOrRef)>0
    ) candidates
) eligible ON eligible.entry=l.entry
SET l.groupid=1;
