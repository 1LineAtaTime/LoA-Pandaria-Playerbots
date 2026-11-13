-- ported from PR 15 2026_10_25_00_characters.sql (#986)
-- Summon Lilian Voss (Guardian) (73306) is no longer saved: drop saved copies, which would keep spell_area from summoning Lilian again on login
DELETE FROM `character_aura` WHERE `spell` = 73306;
