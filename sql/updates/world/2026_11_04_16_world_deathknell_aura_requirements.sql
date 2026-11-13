-- Rigor Mortis belongs to the first-login/Agatha intro, not an area autocast
-- after Fresh out of the Grave has already completed. Mask 66 includes quest
-- COMPLETE (bit 1) and REWARDED (bit 6), causing the stun to return in Deathknell.
-- The original LoA v1.0.4 database has no spell_area row for 73523. Preserve
-- playercreateinfo_cast_spell 73523 and Agatha's 73524/resurrection actions.
DELETE FROM spell_area WHERE spell=73523 AND area=5692 AND quest_start=0
 AND quest_end=24959 AND aura_spell=0 AND racemask=16 AND gender=2
 AND autocast=1 AND quest_start_status=1 AND quest_end_status=66;

-- Original LoA visibility detection lasts before the quest is rewarded:
-- NONE / COMPLETE / INCOMPLETE = bits 0 / 1 / 3 = 11. The current mask 116
-- wrongly includes REWARDED and omits the normal pre-quest states.
-- Source: https://github.com/1LineAtaTime/LoA-Pandaria-Playerbots/releases/tag/v1.0.4
-- database asset SHA256 db8dd845e3a5da4bc3a6f28afa68ed839aceb222101db949ae74840a5a2365a0
UPDATE spell_area SET quest_end_status=11
 WHERE spell=49416 AND area=6454 AND quest_start=0 AND quest_end=24959
 AND aura_spell=0 AND racemask=16 AND gender=2 AND autocast=1
 AND quest_start_status=74 AND quest_end_status=116;
