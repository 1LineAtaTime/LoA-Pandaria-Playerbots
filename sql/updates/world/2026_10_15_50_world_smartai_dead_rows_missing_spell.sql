-- Refs #183 (SmartAI data errors, part 3: rows that reference a spell absent from the 5.4.8 spell data)
-- ERROR CLASS: "SmartAIMgr: Entry N SourceType S Event E Action A uses non-existent Spell entry X, skipped." (SmartScriptMgr.cpp:455 and the cast/aura checks)
--   15 boot lines. The loader REJECTS each row (the whole smart_scripts row is not loaded), so deleting it changes nothing at runtime; it only removes the line.
-- WHY INVALID: the spell id is not in the build-18414 spell data (Cataclysm 4.x ids 90947-91039, Legion 6/7 ids 215377/223971, a removed Wrath id 30011/14252,
--   100101 = 4.2 Firelands). The rows are Cata/Legion sniff leftovers on entries whose real MoP behaviour is not data-driven here.
-- LEFT ALONE: (14822,0,14) Sayge cast 23770 - 12 other rows LINK to this id, deleting it would only move the error to "Link Event not found"; the 3 log lines stay
--   (Sayge needs a real spell; 23770 is the Darkmoon Faire buff id and is missing from the spell data).
-- ROWS: 14 deletes (exact PK entryorguid,source_type,id,link=0): 47403 id 2,4; 47404 id 2,4; 48278 id 14,15; 48417 id 14; 48418 id 14; 48419 id 1; 53619 id 10;
--   70021 id 1; 70034 id 0; 27842 id 2; 1707700 (source_type 9 actionlist) id 2.
-- VERIFY: SELECT COUNT(*) FROM smart_scripts WHERE (entryorguid,source_type,id) IN ((47403,0,2),(47403,0,4),(47404,0,2),(47404,0,4),(48278,0,14),(48278,0,15),(48417,0,14),(48418,0,14),(48419,0,1),(53619,0,10),(70021,0,1),(70034,0,0),(27842,0,2),(1707700,9,2)); -- 0
-- EXPECTED DBErrors.log DROP: 14 lines (the 15th, Sayge 14822, stays; its 3 'Aura condition ... 23770' lines stay too).
-- UNDO: take a backup before applying (mysqldump -t world smart_scripts --where="(source_type=0 AND entryorguid IN (47403,47404,48278,48417,48418,48419,53619,70021,70034,27842)) OR (source_type=9 AND entryorguid=1707700)" > smart_scripts-before-dead.sql) and re-import it; 11 of the 14 rows also exist in the MoP 5.4.8 smart_scripts data.
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `link`=0 AND (`entryorguid`,`id`) IN ((47403,2),(47403,4),(47404,2),(47404,4),(48278,14),(48278,15),(48417,14),(48418,14),(48419,1),(53619,10),(70021,1),(70034,0),(27842,2));
DELETE FROM `smart_scripts` WHERE `source_type`=9 AND `link`=0 AND `entryorguid`=1707700 AND `id`=2;
