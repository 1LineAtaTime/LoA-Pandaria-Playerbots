-- Companion commands are player commands; level/rank/ownership limits are enforced in C++.
-- Also repairs installations that missed updates/master/auth/2026_10_26_00.
INSERT INTO rbac_permissions (id, name) VALUES
    (1346, 'Command: playerbots bot')
    ON DUPLICATE KEY UPDATE name = VALUES(name);
INSERT INTO rbac_linked_permissions (id, linkedId) VALUES (199, 1346)
    ON DUPLICATE KEY UPDATE linkedId = VALUES(linkedId);
