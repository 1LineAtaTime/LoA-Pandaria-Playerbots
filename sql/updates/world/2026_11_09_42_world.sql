-- The Hinterlands: remove the two level 85 Wildhammer Sentries 52084 at Stormfeather Outpost.
-- Sourced: neither the TDB 4.4.2 nor the TDB 1210 spawns entry 52084, and Wowhead MoP Classic has no level and no location for it.
-- Both mop spawns stand on the spots of level 40 Wildhammer Sentries 7865 that match the TDB (192428 2.1 yd from 191340, 192429
-- 4.6 yd from the pathing 191477), so the outpost keeps its guards.
-- Ours: the template and its SmartAI stay, unspawned. The other level 85 entry, 51923 at Aerie Peak, is left alone: Wowhead MoP
-- Classic places it there (16 pins) at level 85-90.
DELETE FROM `creature` WHERE `id`=52084 AND `guid` IN (192428,192429);
DELETE FROM `creature_addon` WHERE `guid` IN (192428,192429);
