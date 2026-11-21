-- Eastern Plaguelands, Argent Call: The Trial of the Crypt 27464: its reward spell 81778 "Trial of the Crypt: Set Argent Dawn
-- Faction To Min Revered" (the quest's RewardSpell, cast on the player at the turn-in) was a bare dummy, so the Argent Dawn
-- standing did not change.
--  * Sourced: comments 2010-11-24 and 2010-11-27 with their replies, 2012-03-08 and 2013-06-02: completing the quest puts the
--    Argent Dawn at Revered 0/21000, gives nothing to a character already Revered or above, and never lowers it; players with
--    reputation bonuses ended above 0/21000 (22880, 24960: comments 2011-03-18, 2011-05-13, 2012-03-10).
--  * Ours: the difference up to 21000 is added as a spell reputation gain (Player::CalculateReputationGain, source SPELL), so
--    the reputation-gain auras apply as the comments show.
DELETE FROM `spell_script_names` WHERE `spell_id`=81778;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES (81778,'spell_epl_trial_of_the_crypt_reputation');
