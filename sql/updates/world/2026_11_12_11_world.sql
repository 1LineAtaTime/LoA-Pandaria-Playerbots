-- Second battle for Andorhal: the Forsaken side and the val'kyr layer hold the line against the Alliance's soldiers.
-- Sourced: Supporting the Troops 27198 "Our defenders are equally matched with their deathguards"; Wowhead comments on 27198
-- (2011-03-28) "the defenders kill the deathguards very fast. If they dps them under 50% u wont get credit" and on the
-- Andorhal Deathguard (2011-04-20) "they're fighting with AI guards. if I kill them I don't get credit". This core has the same
-- rule (Creature::IsDamageEnoughForLootingAndReward: no credit unless players or what they own dealt half the health).
-- Ours: the entries and the 85% value (the first battle's choice, 2026_11_10_95). TCPP's ported static flags carry no
-- NO_NPC_DAMAGE_BELOW_85PTC on any Andorhal mob (nor on the first battle's, where a comment describes the stop) and TDB 12.1.0's
-- creature_template_sparring has no Andorhal entry. Capped: the Deathguard 44447 (27198), the Lesser Val'kyr 45239 (27201), the
-- Forsaken Troopers 45240 and 45242 (27202), and Aradne 45235 (27204) as a precaution, in case she is pulled down from her tower
-- to where the Defenders fight (the nearest stands 61 yards away). Unit::DealDamage caps
-- damage only from creatures no player owns, so players, their pets and Thurman's minutemen kill them as before. The Andorhal
-- Defender 44448 is not capped here: 2026_11_12_15 caps it once the Horde's mobilized Forsaken Troopers (Ace in the Hole 27089)
-- count as their player's.
DELETE FROM `creature_sparring_template` WHERE `creature_id` IN (44447,45235,45239,45240,45242);
INSERT INTO `creature_sparring_template` (`creature_id`,`health_limit_pct`,`comment`) VALUES
(44447,85,'Andorhal Deathguard - second battle for Andorhal'),
(45235,85,'Aradne - second battle for Andorhal'),
(45239,85,'Lesser Val''kyr - second battle for Andorhal'),
(45240,85,'Forsaken Trooper - second battle for Andorhal'),
(45242,85,'Forsaken Trooper - second battle for Andorhal');
