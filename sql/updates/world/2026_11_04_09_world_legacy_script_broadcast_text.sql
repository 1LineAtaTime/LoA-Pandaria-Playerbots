-- Legacy DB script TALK records use 2,000,000,000-range db_script_string IDs.
-- The current dispatcher resolves TALK through broadcast_text. Restore that
-- compatibility using the EXISTING authored text and translations; invent none.
-- Existing broadcast entries/locales win. Keep the legacy table unchanged.
INSERT IGNORE INTO `broadcast_text` (`ID`,`LanguageID`,`Text`,`Text1`)
SELECT entry,0,content_default,content_default FROM `db_script_string`
WHERE entry>=2000000000 AND entry<2147483647 AND COALESCE(content_default,'')<>'';
INSERT IGNORE INTO `broadcast_text_locale` (`ID`,`locale`,`Text`,`Text1`)
SELECT entry,locale,content,content FROM (
SELECT entry,'koKR' AS locale,content_loc1 AS content FROM `db_script_string` WHERE entry>=2000000000 AND entry<2147483647 AND COALESCE(content_default,'')<>''
UNION ALL
SELECT entry,'frFR' AS locale,content_loc2 AS content FROM `db_script_string` WHERE entry>=2000000000 AND entry<2147483647 AND COALESCE(content_default,'')<>''
UNION ALL
SELECT entry,'deDE' AS locale,content_loc3 AS content FROM `db_script_string` WHERE entry>=2000000000 AND entry<2147483647 AND COALESCE(content_default,'')<>''
UNION ALL
SELECT entry,'zhCN' AS locale,content_loc4 AS content FROM `db_script_string` WHERE entry>=2000000000 AND entry<2147483647 AND COALESCE(content_default,'')<>''
UNION ALL
SELECT entry,'zhTW' AS locale,content_loc5 AS content FROM `db_script_string` WHERE entry>=2000000000 AND entry<2147483647 AND COALESCE(content_default,'')<>''
UNION ALL
SELECT entry,'esES' AS locale,content_loc6 AS content FROM `db_script_string` WHERE entry>=2000000000 AND entry<2147483647 AND COALESCE(content_default,'')<>''
UNION ALL
SELECT entry,'esMX' AS locale,content_loc7 AS content FROM `db_script_string` WHERE entry>=2000000000 AND entry<2147483647 AND COALESCE(content_default,'')<>''
UNION ALL
SELECT entry,'ruRU' AS locale,content_loc8 AS content FROM `db_script_string` WHERE entry>=2000000000 AND entry<2147483647 AND COALESCE(content_default,'')<>''
UNION ALL
SELECT entry,'itIT' AS locale,content_loc9 AS content FROM `db_script_string` WHERE entry>=2000000000 AND entry<2147483647 AND COALESCE(content_default,'')<>''
UNION ALL
SELECT entry,'ptBR' AS locale,content_loc10 AS content FROM `db_script_string` WHERE entry>=2000000000 AND entry<2147483647 AND COALESCE(content_default,'')<>''
UNION ALL
SELECT entry,'ptPT' AS locale,content_loc11 AS content FROM `db_script_string` WHERE entry>=2000000000 AND entry<2147483647 AND COALESCE(content_default,'')<>''
) translated WHERE COALESCE(content,'')<>'';
