-- Thundering Exile 2762: one spawn too many east of the Circle of Inner Binding. Sourced: TDB 442/1210 have 12 Thundering Exiles, mop 13 after 2026_11_08_12.
-- Each mop spawn maps onto its nearest TDB spawn one to one, except 73820 and 160770, which both map onto TDB 285783 (5.2 and 6.8 yd from it, 9.1 yd apart, both wandering).
-- 160770 goes (farther from the TDB spot). It has no addon, SmartAI, formation, pool, event, linked-respawn or movement rows.
DELETE FROM `creature` WHERE `guid` = 160770 AND `id` = 2762;
