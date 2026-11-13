-- Restore37 absent whole gameobject loot groups from original LoA v1.0.4.
-- Preserve every existing group, including partial custom overrides. Reject a
-- source group if an item is absent/quarantined. Counts/chances/modes stay sourced.
CREATE TEMPORARY TABLE codex_missing_go_loot_seed LIKE gameobject_loot_template;
INSERT INTO codex_missing_go_loot_seed (entry,item,ChanceOrQuestChance,lootmode,groupid,mincountOrRef,maxcount) VALUES
(25106,45196,0.5,'',0,1,1),
(25666,45196,0.5,'',0,1,1),
(25672,45196,0.5,'',0,1,1),
(28567,2835,0.1,'',0,1,1),
(35544,45196,100,'',0,1,1),
(40442,74295,50,'',0,1,1),
(40483,75217,30,'',0,1,1),
(40862,73208,-100,'',0,1,1),
(40862,73212,-100,'',0,1,1),
(40864,73210,-100,'',0,1,1),
(42524,74860,15,'',0,1,2),
(42524,87724,0.5,'',0,1,1),
(42733,40652,0.5,'',0,1,1),
(42832,89112,5,'',0,1,1),
(43122,87282,100,'',0,1,1),
(43354,46691,100,'',0,1,1),
(43362,45196,0.5,'',0,1,1),
(43477,45196,0.5,'',0,1,1),
(43571,29150,100,'',0,1,1),
(43905,75208,100,'',0,1,1),
(44078,45196,100,'',0,1,1),
(45180,45196,20,'',0,1,1),
(45540,91846,100,'',0,1,1),
(45553,91907,100,'',0,1,4),
(45601,92550,100,'',0,1,1),
(45845,3820,20,'',0,1,1),
(45845,6358,15,'',0,1,1),
(45845,6359,25,'',0,1,1),
(45845,21150,35,'',0,1,1),
(45845,21151,1,'',0,1,1),
(45853,3820,1,'',0,1,1),
(45853,21071,25,'',0,1,1),
(45853,21113,15,'',0,1,1),
(45853,21151,20,'',0,1,1),
(46970,74859,85,'',0,1,1),
(46970,86542,1,'',0,1,1),
(46970,88496,20,'',0,1,1),
(46970,94934,0.2,'',0,1,1),
(46971,74861,15,'',0,1,2),
(46971,86542,0.5,'',0,1,1),
(46971,88496,0.8,'',0,1,1),
(46971,94935,10,'',0,1,2),
(46971,97981,0.5,'',0,1,1),
(46973,83064,80,'',0,1,1),
(46973,86544,1,'',0,1,1),
(46973,88496,80,'',0,1,1),
(46973,94932,1,'',0,1,1),
(46974,74865,0.8,'',0,1,1),
(46975,74856,80,'',0,1,1),
(46975,86542,1,'',0,1,1),
(46975,88496,80,'',0,1,1),
(46975,94934,1,'',0,1,1),
(46976,74863,80,'',0,1,1),
(46976,86542,0.5,'',0,1,1),
(46976,88496,80,'',0,1,1),
(46976,94933,0.5,'',0,1,1),
(47771,97543,100,'',0,1,2),
(47772,97544,100,'',0,1,2),
(47773,97545,100,'',0,1,5),
(49457,74863,60,'',0,1,1),
(49457,86542,1,'',0,1,1),
(49457,94933,1,'',0,1,1),
(49530,34822,0.5,'',0,1,1),
(50764,45196,0.5,'',0,1,1);
CREATE TEMPORARY TABLE codex_existing_go_loot AS SELECT DISTINCT entry FROM gameobject_loot_template;
CREATE TEMPORARY TABLE codex_blocked_go_loot AS
 SELECT DISTINCT s.entry FROM codex_missing_go_loot_seed s LEFT JOIN item_template i ON i.entry=s.item
 WHERE i.entry IS NULL OR i.RequiredLevel>=100;
INSERT INTO gameobject_loot_template (entry,item,ChanceOrQuestChance,lootmode,groupid,mincountOrRef,maxcount)
 SELECT s.entry,s.item,s.ChanceOrQuestChance,s.lootmode,s.groupid,s.mincountOrRef,s.maxcount
 FROM codex_missing_go_loot_seed s
 LEFT JOIN codex_existing_go_loot e ON e.entry=s.entry
 LEFT JOIN codex_blocked_go_loot b ON b.entry=s.entry
 WHERE e.entry IS NULL AND b.entry IS NULL;
DROP TEMPORARY TABLE codex_missing_go_loot_seed;
DROP TEMPORARY TABLE codex_existing_go_loot;
DROP TEMPORARY TABLE codex_blocked_go_loot;
