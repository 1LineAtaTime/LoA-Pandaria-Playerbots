-- An Ancient Enemy (24814): the event's actors come back quickly, so the next player can start it again
-- Zar'jira (38306) dies at the end of the event, and Zuni (38423) and Vanira (38437) despawn after it (their SmartAI
-- action lists 3842302 / 3843703). Their spawns kept a one-hour respawn timer, so after one player (or bot) finished
-- the event, Vol'jin's "I am ready" gossip started an event without Zar'jira, Zuni or Vanira for up to an hour. quietpatcher's fork found the
-- same dead end ("she stayed dead for an hour", zone_azshara.cpp in commit 2e3a6bd4) and respawns them from Vol'jin's
-- gossip in C++; here the SmartAI version of the event runs (its C++ is no longer bound), so the spawns get a one-minute
-- respawn timer instead. Vol'jin (38225) never dies and keeps his timer.
-- Idempotent: only the three spawns, and only while they still have the old timer.
UPDATE `creature` SET `spawntimesecs` = 60
WHERE `guid` IN (252718, 252719, 252720) AND `id` IN (38306, 38423, 38437) AND `spawntimesecs` = 3600;
