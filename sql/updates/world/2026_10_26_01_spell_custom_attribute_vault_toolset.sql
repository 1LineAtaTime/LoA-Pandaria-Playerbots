-- #938: 'Vault Cracking Toolset' (67476, The Great Bank Heist 14122) puts a dummy aura on the rider next to the vault's
-- control-vehicle aura. Only the control-vehicle half was unsaveable (Aura::CanBeSaved), so a player who logged out or disconnected
-- inside the vault kept the dummy aura for good, and the vault spell (67555) then failed with TARGET_AURASTATE: quest blocked.
INSERT IGNORE INTO `spell_custom_attribute` (`spell`, `attribute`, `comment`) VALUES
    (67476, 'SPELL_ATTR0_CU_DONT_SAVE_AURA_TO_DB', 'Vault Cracking Toolset: the rider dummy aura must not outlive the vault (#938)');
