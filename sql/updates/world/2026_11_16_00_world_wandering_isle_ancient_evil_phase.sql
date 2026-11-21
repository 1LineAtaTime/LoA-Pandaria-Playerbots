-- Wandering Isle, An Ancient Evil (29798): the quest's phase aura (59073, the Vordraka fight phase) is applied in
-- area 5833 while the quest is taken. This core keeps a spell_area aura only while the quest_end quest is in one of
-- the quest_end_status states, and the row allowed only "none" and "incomplete" (9), so the aura dropped the moment
-- Vordraka's death completed the quest: the player left the phase, Aysa and Vordraka vanished, and the quest could
-- not be turned in. Keep the aura until the quest is rewarded (none | complete | incomplete = 11).
UPDATE `spell_area` SET `quest_end_status` = 11
WHERE `spell` = 59073 AND `area` = 5833 AND `quest_start` = 29798 AND `quest_end` = 29798 AND `quest_end_status` = 9;
-- Undo: UPDATE `spell_area` SET `quest_end_status` = 9 WHERE `spell` = 59073 AND `area` = 5833 AND `quest_start` = 29798 AND `quest_end` = 29798;
