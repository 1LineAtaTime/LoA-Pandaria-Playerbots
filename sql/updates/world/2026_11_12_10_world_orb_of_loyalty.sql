-- Orb of Loyalty: single-use100 DP credit through the native item script.
-- Existing item instances are never deleted/clamped by this public migration.
UPDATE item_template SET Quality=5,bonding=1,stackable=20,SellPrice=0,
 description='Adds currency to your account of 100 Battle Shop Coins.',
 class=0,subclass=8,spellid_1=483,spelltrigger_1=0,spellcharges_1=0
 WHERE entry=35626;
INSERT INTO item_script_names(Id,ScriptName) VALUES(35626,'item_orb_of_loyalty')
 ON DUPLICATE KEY UPDATE ScriptName=VALUES(ScriptName);
UPDATE item_template_locale SET Description='Adds currency to your account of 100 Battle Shop Coins.' WHERE ID=35626;
-- Notify already-cached clients about both changed item data tables at login.
-- Replace only this item's two timestamps; do not accumulate duplicate versions.
DELETE FROM hotfix_data WHERE entry=35626 AND type IN(1344507586,2442913102);
INSERT INTO hotfix_data(entry,type,hotfixDate) VALUES
(35626,1344507586,CURRENT_TIMESTAMP),(35626,2442913102,CURRENT_TIMESTAMP);
