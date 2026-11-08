-- Eastpoint Tower: Tog'thar and Drull sit on Nils Beerot's couches.
-- Sourced: Eastpoint Tower 28620's reward text, "Now they sit upon my couch with their big, iron balls and cry"; their spawn z (66.65, 66.70) is on a
-- seat surface of the vmaps (66.58, 66.64), about 0.8 yd above the floor at 65.82 and 65.88, where Nils stands.
-- Ours: the sit state. Both TDBs have StandState 0 for them and no spawn addon, so no sniff shows them sitting. State 1 sits on the surface at the spawn's z.
UPDATE `creature_template_addon` SET `StandState`=1 WHERE `entry` IN (2238,2239);
