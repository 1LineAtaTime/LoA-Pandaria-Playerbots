-- LoA world DB (the test DB `world`) - runtime DBErrors: "Entry 12530 not found in `trinity_string` table."
-- (70 per owner session) and "Entry 12528 ..." (4). MoP/LoA-only strings; AC has no equivalent.
--
-- Cause: Player::AddTotalBGCount (Player.cpp:31753-31789) formats LANG_BG_REWARD_BODY (12530) with
-- printf-style Format(fmt, totalBGCount, totalBGWins) on EVERY battleground finish (players and bots), even when
-- Battleground.Rewards is off; the reward mails use LANG_BG_REWARD_TITLE_POOR (12528) / _RICH (12529,
-- RatedPvp.cpp:165 too). Language.h:1555-1557 declares them; the world DB has 12520-12527 and 12531+ only.
-- Effect today: log spam only (the "<error>" text is used only when a reward mail is actually sent).
--
-- BACKUP: nothing is overwritten (rows do not exist). Pre-check (must return 0 rows):
SELECT entry FROM trinity_string WHERE entry IN (12528, 12529, 12530);

INSERT INTO `trinity_string` (`entry`, `content_default`) VALUES
(12528, 'Battleground reward'),
(12529, 'Battleground bonus reward'),
(12530, 'You have finished %u battlegrounds and won %u of them. Thank you for fighting!');

-- Takes effect after a worldserver restart.
-- ROLLBACK: DELETE FROM `trinity_string` WHERE `entry` IN (12528, 12529, 12530);
