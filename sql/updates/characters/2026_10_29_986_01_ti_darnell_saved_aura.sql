-- ported from PR 15 2026_10_23_00_characters.sql (#986)
-- Summon Darnell (91938) is no longer saved: drop saved copies, which would keep spell_area from summoning Darnell again on login
DELETE FROM `character_aura` WHERE `spell` = 91938;
