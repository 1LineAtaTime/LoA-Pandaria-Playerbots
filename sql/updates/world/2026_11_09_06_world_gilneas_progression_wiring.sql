-- Same allowed-status correction as05, covering the remaining Gilneas stages.
UPDATE spell_area SET quest_end_status=11 WHERE area IN(4714,4755,4786) AND quest_end<>0 AND quest_end_status=64 AND spell IN(68481,68483,69484,69485,69486,70695,70696,65141);
-- Historical imports left addon prerequisites zero while authored legacy links
-- remained nonzero. Preserve every explicit addon prerequisite; restore only gaps.
UPDATE quest_template_addon a JOIN quest_template q ON q.ID=a.ID SET a.PrevQuestID=q.PrevQuestId WHERE q.QuestSortID IN(4714,4755) AND q.PrevQuestId<>0 AND a.PrevQuestID=0;
-- Child clicks cast rescue spells. Gossip-only SmartAI never receives SpellHit.
UPDATE creature_template SET AIName='',ScriptName='npc_cynthia',npcflag=(npcflag | 16777216) & ~1 WHERE entry=36287;
UPDATE creature_template SET AIName='',ScriptName='npc_ashley',npcflag=(npcflag | 16777216) & ~1 WHERE entry=36288;
UPDATE creature_template SET AIName='',ScriptName='npc_james',npcflag=(npcflag | 16777216) & ~1 WHERE entry=36289;
DELETE FROM smart_scripts WHERE source_type=0 AND entryorguid IN(36287,36288,36289);
-- Native Gwen starts/retries the existing authored horse route and gate events.
UPDATE creature_template SET AIName='',ScriptName='npc_gwen_armstead' WHERE entry=36452;

-- Restore original15-point Manor route from LoA commit2d4b1928.
-- Base data replaced38765 with the76-point Hewell path;387650 remains separate.
DELETE FROM waypoints WHERE entry=38765;
INSERT INTO waypoints(entry,pointid,position_x,position_y,position_z,point_comment) VALUES
(38765, 1, -1870.36, 2282.73, 42.32, 'Gilneas14465 Manor route; upstream2d4b1928'),
(38765, 2, -1852.05, 2302.51, 41.83, 'Gilneas14465 Manor route; upstream2d4b1928'),
(38765, 3, -1829.29, 2323.74, 37.55, 'Gilneas14465 Manor route; upstream2d4b1928'),
(38765, 4, -1818.32, 2332.80, 36.37, 'Gilneas14465 Manor route; upstream2d4b1928'),
(38765, 5, -1801.58, 2344.71, 35.77, 'Gilneas14465 Manor route; upstream2d4b1928'),
(38765, 6, -1792.48, 2352.44, 36.52, 'Gilneas14465 Manor route; upstream2d4b1928'),
(38765, 7, -1787.74, 2359.60, 38.00, 'Gilneas14465 Manor route; upstream2d4b1928'),
(38765, 8, -1781.81, 2383.18, 43.27, 'Gilneas14465 Manor route; upstream2d4b1928'),
(38765, 9, -1777.55, 2423.56, 55.68, 'Gilneas14465 Manor route; upstream2d4b1928'),
(38765, 10, -1770.01, 2447.47, 62.25, 'Gilneas14465 Manor route; upstream2d4b1928'),
(38765, 11, -1754.47, 2464.13, 69.36, 'Gilneas14465 Manor route; upstream2d4b1928'),
(38765, 12, -1710.82, 2466.15, 82.81, 'Gilneas14465 Manor route; upstream2d4b1928'),
(38765, 13, -1691.46, 2493.66, 95.20, 'Gilneas14465 Manor route; upstream2d4b1928'),
(38765, 14, -1682.62, 2507.22, 97.79, 'Gilneas14465 Manor route; upstream2d4b1928'),
(38765, 15, -1669.61, 2519.56, 97.89, 'Gilneas14465 Manor route; upstream2d4b1928');
UPDATE gameobject SET phaseMask=16384 WHERE map=654 AND id IN(196863,196864);

-- The native watchman selects groups0..2, whereas the imported base grouped
-- its three alternatives as IDs0..2 of group0. Preserve all authored text.
UPDATE creature_text SET groupid=id,id=0 WHERE entry=36440 AND CreatureID=36440 AND groupid=0 AND id IN(1,2);
