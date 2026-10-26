-- #934 / public #9: '.playerbots bot' (and the '.npcbot' alias) is a player command, as in AC mod-playerbots (SEC_PLAYER).
-- Permission 1346 (cs_playerbots.cpp RBAC_PERM_COMMAND_PLAYERBOTS_BOT) linked to role 199 (Player Commands), which every
-- security level inherits. Which characters a player may add is checked by PlayerbotHolder::AddPlayerBot (own account,
-- own guild or a random-bot account). Idempotent.
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES
    (1346, 'Command: playerbots bot') ON DUPLICATE KEY UPDATE `name` = VALUES(`name`);
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES
    (199, 1346) ON DUPLICATE KEY UPDATE `linkedId` = VALUES(`linkedId`);
