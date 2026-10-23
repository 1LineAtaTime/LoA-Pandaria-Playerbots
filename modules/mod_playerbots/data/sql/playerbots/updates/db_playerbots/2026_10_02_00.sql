-- Chat text for the ported quest-support layer (issue #322).
--
-- Two keys are looked up by the quest code being ported and have no row in `ai_playerbot_texts`
-- on this fork (verified: absent from data/sql/playerbots/base/ai_playerbot_texts.sql and
-- data/sql/playerbots/base/playerbots_database_full.sql):
--
--   * "%quest_link - %item_link %quest_obj_available/%quest_obj_required"
--       AC Ai/Base/Actions/QuestAction.cpp:370 - the per-item quest objective line printed by the
--       `QuestAction` "query" path.
--   * "Failed timer for %quest_link, abandoning"
--       AC Ai/Base/Actions/QuestAction.cpp:454 - printed when the bot abandons an expired quest.
--
-- Both are absent upstream too, so on AC `PlayerbotTextMgr::GetBotText()` falls through to returning
-- the key verbatim (after placeholder substitution). The rows added here keep that behaviour byte for
-- byte - `text` is the same string as `name` - and give the keys a translation slot (text_locN), which
-- is the only reason to have a row at all. Add real translations by updating text_loc1..text_loc8.
--
-- Note the keys are literal message text, so they are quoted with double quotes in C++; the single
-- quotes below are SQL string delimiters only.
--
-- `say_type`/`reply_type` are 0 (say / not a reply); text_loc1..text_loc8 default to '' as in the base
-- dump (data/sql/playerbots/base/ai_playerbot_texts.sql:19).

INSERT INTO `ai_playerbot_texts` (`name`, `text`, `say_type`, `reply_type`, `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`, `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`) VALUES
('%quest_link - %item_link %quest_obj_available/%quest_obj_required', '%quest_link - %item_link %quest_obj_available/%quest_obj_required', 0, 0, '', '', '', '', '', '', '', ''),
('Failed timer for %quest_link, abandoning', 'Failed timer for %quest_link, abandoning', 0, 0, '', '', '', '', '', '', '', '');