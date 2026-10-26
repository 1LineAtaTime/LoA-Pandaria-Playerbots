-- #938: drop the 'Vault Cracking Toolset' (67476) rider auras saved before world update 2026_10_26_01 (they block
-- The Great Bank Heist 14122 for good; see that update).
DELETE FROM `character_aura` WHERE `spell` = 67476;
