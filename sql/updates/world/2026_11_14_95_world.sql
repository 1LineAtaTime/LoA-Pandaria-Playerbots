-- Eastern Plaguelands: four hidden turn-in spells are no longer shown as quest rewards.
--
-- Reported in game: Argent Call: The Trial of the Crypt showed "Trial of the Crypt: Set Argent Dawn Faction To Min Revered"
-- as a reward on the quest screen. It is a helper spell the turn-in casts, not a reward to show. The zone has three more of
-- the kind: Just Encased (Knockback 50 No Log), Soft Landing (Update Phase Shift (Player)) and Ix'lar the Underlord (Cancel
-- Summon Tarenar Sunstrike).
--
-- This core sends RewardSpell to the client as the spell to show, and never sends RewardDisplaySpell; at the turn-in it casts
-- RewardSpell, or RewardDisplaySpell when RewardSpell is 0 (Player::RewardQuest). A spell that sits only in
-- RewardDisplaySpell is therefore cast and not shown.
--
-- Sourced: the spells and that the turn-in casts them (both TDBs and the DBC names).
-- Ours: the column they sit in. Should the core one day send RewardDisplaySpell to the client, these four move back.

UPDATE `quest_template` SET `RewardDisplaySpell`=81778, `RewardSpell`=0 WHERE `ID`=27464 AND 81778 IN (`RewardSpell`,`RewardDisplaySpell`);
UPDATE `quest_template` SET `RewardDisplaySpell`=70966, `RewardSpell`=0 WHERE `ID`=27368 AND 70966 IN (`RewardSpell`,`RewardDisplaySpell`);
UPDATE `quest_template` SET `RewardDisplaySpell`=82238, `RewardSpell`=0 WHERE `ID`=27460 AND 82238 IN (`RewardSpell`,`RewardDisplaySpell`);
UPDATE `quest_template` SET `RewardDisplaySpell`=85807, `RewardSpell`=0 WHERE `ID`=27487 AND 85807 IN (`RewardSpell`,`RewardDisplaySpell`);
