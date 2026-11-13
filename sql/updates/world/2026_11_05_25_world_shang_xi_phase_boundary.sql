-- Original LoA v1.0.4 has the same post-29409/pre-29419 invisibility phase
-- in all four subareas. Two local rows incorrectly require29419 to be taken
-- or rewarded, leaving a gap at the Tranquil Grotto/Wu-Song boundary.
-- Preserve all other quest, area, aura and player data.
UPDATE spell_area SET quest_end_status=9
 WHERE spell=105064 AND area IN (5843,5846) AND quest_start=29409
 AND quest_end=29419 AND aura_spell=0 AND racemask=0 AND gender=2 AND autocast=1
 AND quest_start_status=66 AND ((area=5843 AND quest_end_status=74) OR (area=5846 AND quest_end_status=66));
