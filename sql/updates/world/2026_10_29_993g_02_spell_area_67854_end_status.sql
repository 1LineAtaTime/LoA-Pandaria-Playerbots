-- 14245 "It's a Town-In-A-Box": the Town-In-A-Box Plunger (GO 201938) and its scene live in phase 1024, which the aura 67854 provides
-- in area 4720 from quest 14326 until quest 14245. The row's quest_end_status was 1 (only "quest 14245 not taken"), so the moment a
-- goblin accepted 14245 the aura was removed, the plunger became invisible (goguid found nothing within 300 yd) and the kill-credit
-- 38024 could never be earned. SpellArea::IsFitToRequirements keeps the aura while ((1 << status of the end quest) & mask) != 0: the
-- neighbouring rows use 11 (not taken, complete, in progress), which is what this row needs. Idempotent.
UPDATE `spell_area` SET `quest_end_status` = 11 WHERE `spell` = 67854 AND `area` = 4720 AND `quest_start` = 14326 AND `quest_end` = 14245 AND `quest_end_status` <> 11;
