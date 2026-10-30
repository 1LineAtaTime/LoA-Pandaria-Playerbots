-- Issue #963: "ScriptName '0' / '1' exists in database, but no core script found!" (2 log lines).
-- WHAT: placeholder values '0'/'1' stored in ScriptName (bad import) reset to ''.
--   conditions: 6 rows ScriptName '0'; gameobject: 1 row (guid 20936, id 200004) ScriptName '1'.
-- EXPECTED AFTER: 0 lines for these. No behaviour change (neither is a script name).
-- SOURCE: self-evident; not in the reference DB as scripts.
-- Left alone (real missing core scripts, need C++): achievement_im_on_a_boat, at_well_of_eternity_skip_illidan_intro,
--   sat_icy_shadows, spell_howling_gale_howling_gale, spell_protection_of_elune, 21 battle_pay_* / wow_token_* item scripts.
UPDATE `conditions` SET `ScriptName` = '' WHERE `ScriptName` IN ('0', '1');
UPDATE `gameobject` SET `ScriptName` = '' WHERE `ScriptName` IN ('0', '1');
