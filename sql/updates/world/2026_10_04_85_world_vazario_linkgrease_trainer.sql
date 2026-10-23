-- Vazario Linkgrease (8738, Goblin Engineering Trainer, Ratchet, LoA guid 264993): empty trainer window
-- (epic #441, audit row SQ007).
--
-- LoA: npcflag & 16 and trainer_type 2 are set, but npc_trainer has 0 rows for 8738, so "Train me" is empty.
-- Intended design: Wowhead mop-classic tooltips give BOTH Vazario Linkgrease (8738, Ratchet) and Nixx
-- Sprocketspring (8126, Gadgetzan) the title "Goblin Engineering Trainer"; Wowpedia "Vazario Linkgrease"
-- gossip: "If you are here to access goblin engineering schemata, please know we only interface with
-- certified goblin engineers." LoA's Nixx teaches the 18 goblin-engineering schematics, and the shared list
-- 200433 is row-for-row identical to Nixx's list (verified on the LoA base world dump) and referenced by
-- nobody, so Vazario gets a reference to it (LoA ObjectMgr::LoadTrainerSpell expands negative `spell` values
-- as references to another npc_trainer.entry). Pattern from the profession trainer menus patch (0031_profession_trainer_menus.sql).
--
-- NOT changed (the other four SQ007 NPCs), per Wowpedia (MediaWiki API, fetched 2026-10-04):
--  * Borgus Steelhand 7232 (Stormwind): "retitled as a blacksmithing trainer but does not actually teach anything".
--  * Kurdram Stonehammer 6297, Peter Galen 7866, Thorkaf Dragoneye 7867: {{Classic only|patch=4.0.3a}} - removed
--    in Cataclysm; none of them is spawned in LoA.
--
-- Rollback: DELETE FROM `npc_trainer` WHERE `entry` = 8738;

DELETE FROM `npc_trainer` WHERE `entry` = 8738;
INSERT INTO `npc_trainer` (`entry`, `spell`, `spellcost`, `reqskill`, `reqskillvalue`, `reqlevel`) VALUES
(8738, -200433, 0, 0, 0, 0);
