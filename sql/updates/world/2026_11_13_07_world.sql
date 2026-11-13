-- Western Plaguelands, Andorhal: the second battle ends with each faction's chain, and the Forsaken hold the town afterwards.
-- Until now every player who had taken any second-battle quest stayed in 261 "Andorhal 03 - Horde vs. Alliance" (and in the
-- val'kyr layers 268/272) for good, so the battle never ended.
--
-- Sourced:
--   Horde end = Ace in the Hole 27089 handed in. Its reward text: "With Thassarian nowhere to be found, it would seem that the
--     Alliance forces have withdrawn"; Andorhal, Once and For All 27090: "Sylvanas will want to know about Thassarian's
--     departure from Andorhal. You resolve to return to the Forsaken-held side of town." warcraft.wiki.gg (Andorhal): the Horde
--     reach the Alliance camp to find Thassarian and his forces retreated and Alliance civilians dead; Andorhal Deathguards
--     patrol the town.
--   Alliance end = Alas, Andorhal 27206 handed in. Aradne 27204 reward: "The val'kyr and the Forsaken press on; soon, they will
--     overtake us. We have lost Andorhal." (so the battle goes on while 27206 is in the log). Wowhead comments: 27206
--     2010-11-28 [+45] "Completing this quest phases out the flight path for alliance", reply [+19] "once you're done with this
--     quest there's really nothing for you to do in Andorhal, and no friendly NPCs"; Thassarian 44453 2011-03-22 [+10] "after
--     completing all quests in western plaguelands as alliance, Andorhal is taken by the Forsaken. All NPCs are dead.
--     Thassarian is gone, only the corpse(!) of his minion Lurid remains"; Alexis Torry 47719 2011-04-01 [+7] "she doesn't make
--     it out after the Horde overrun Andorhal. Her body is on the path by the inn, next to that of Foreman Milos and Benjamin
--     Henderson"; Lurid 45129 2011-04-22 [+6] "after the final quests (27206/27090) in Andorhal, the Horde will take over the
--     town, and Lurid's corpse, among other Alliance corpses, will be found near the Inn". So both factions see a Forsaken town.
--   The Horde's living Alliance townsfolk (185) go when the Horde reach their camp (27089 taken), with Thassarian's camp (179),
--     when Lurid's corpse (180) appears: the wiki's order (camp reached, Thassarian gone, civilians dead).
-- Ours:
--   The keys. 261 now: while any second-battle quest is in the log, or between quests of the chain (The Battle Resumes!
--     27083/27197 handed in, the faction's last quest not yet). The in-log rows keep the battle for an optional quest taken
--     late (Supporting the Troops 27085 needs only 27083 and can still be taken after 27089). 268: 27087 handed in, 27089 not
--     yet. 272: 27205 handed in, 27206 not yet (or 27201/27202/27204 in the log).
--   Players keep 261 together with 268/272 as before; no spawn moves to Phase.dbc groups 518 [261,268] / 519 [261,272]. Those
--     groups suggest retail players in 268/272 no longer held 261, but the comments put the Defenders in Ace in the Hole's
--     fights and the Deathguards in Brother Against Brother 27202's ("fight them too, along with the deathguards and val'kyr"),
--     and one spawn can be in only one group.
--   191 = the town after the battle (Phase.dbc has no Andorhal phase after 272; 191 "Quest Zone-Specific 16" is used by Silverpine
--     Forest 130 through phase_area and granted by spells elsewhere, e.g. 74093 through spell_area in Gilneas; nothing grants it
--     in Western Plaguelands): Horde from 27089 handed in, unless 27085 is
--     in the log (the battle is back for it); Alliance from 27206 handed in. Its spawns are in 2026_11_13_08_world.sql.
DELETE FROM `phase_area` WHERE `AreaId`=28 AND `PhaseId` IN (191,261,268,272,185);
INSERT INTO `phase_area` (`AreaId`,`PhaseId`,`Comment`) VALUES
(28,185,'Western Plaguelands - the Alliance townsfolk of Andorhal until Ace in the Hole is taken / Alas, Andorhal is handed in'),
(28,191,'Western Plaguelands - Andorhal held by the Forsaken, after Ace in the Hole / Alas, Andorhal'),
(28,261,'Western Plaguelands - Andorhal 03: Horde vs. Alliance, from the first second-battle quest to the end of the faction''s chain'),
(28,268,'Western Plaguelands - Andorhal 04H: Val''kyr attack (Horde), from Lindsay Ravensun, Revealed until Ace in the Hole is handed in'),
(28,272,'Western Plaguelands - Andorhal 04A: Val''kyr attack (Alliance), from The Depravity of the Forsaken until Alas, Andorhal is handed in');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceEntry`=28 AND `SourceGroup` IN (191,261,268,272,185);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
-- 261, Horde: between the chain's quests, and each quest while in the log
(26,261,28,0,0,47,0,27083,64,0,0,0,0,'','Andorhal 03 - The Battle Resumes! (H) rewarded'),
(26,261,28,0,0,47,0,27089,64,0,1,0,0,'','Andorhal 03 - Ace in the Hole not rewarded'),
(26,261,28,0,1,47,0,27083,10,0,0,0,0,'','Andorhal 03 - The Battle Resumes! (H) in the log'),
(26,261,28,0,2,47,0,27084,10,0,0,0,0,'','Andorhal 03 - The Farmers'' Militia in the log'),
(26,261,28,0,3,47,0,27085,10,0,0,0,0,'','Andorhal 03 - Supporting the Troops (H) in the log'),
(26,261,28,0,4,47,0,27086,10,0,0,0,0,'','Andorhal 03 - Ashes to Ashes (H) in the log'),
(26,261,28,0,5,47,0,27087,10,0,0,0,0,'','Andorhal 03 - Lindsay Ravensun, Revealed in the log'),
(26,261,28,0,6,47,0,27089,10,0,0,0,0,'','Andorhal 03 - Ace in the Hole in the log'),
-- 261, Alliance
(26,261,28,0,7,47,0,27197,64,0,0,0,0,'','Andorhal 03 - The Battle Resumes! (A) rewarded'),
(26,261,28,0,7,47,0,27206,64,0,1,0,0,'','Andorhal 03 - Alas, Andorhal not rewarded'),
(26,261,28,0,8,47,0,27197,10,0,0,0,0,'','Andorhal 03 - The Battle Resumes! (A) in the log'),
(26,261,28,0,9,47,0,27198,10,0,0,0,0,'','Andorhal 03 - Supporting the Troops (A) in the log'),
(26,261,28,0,10,47,0,27199,10,0,0,0,0,'','Andorhal 03 - Ashes to Ashes (A) in the log'),
(26,261,28,0,11,47,0,27205,10,0,0,0,0,'','Andorhal 03 - The Depravity of the Forsaken in the log'),
(26,261,28,0,12,47,0,27201,10,0,0,0,0,'','Andorhal 03 - Val''kyr Incursion in the log'),
(26,261,28,0,13,47,0,27202,10,0,0,0,0,'','Andorhal 03 - Brother Against Brother in the log'),
(26,261,28,0,14,47,0,27204,10,0,0,0,0,'','Andorhal 03 - Aradne in the log'),
(26,261,28,0,15,47,0,27206,10,0,0,0,0,'','Andorhal 03 - Alas, Andorhal in the log'),
-- 268 (Horde val'kyr layer: the Frightened Provincial Minutemen and areatrigger 6235's phase)
(26,268,28,0,0,47,0,27087,64,0,0,0,0,'','Andorhal 04H - Lindsay Ravensun, Revealed rewarded'),
(26,268,28,0,0,47,0,27089,64,0,1,0,0,'','Andorhal 04H - Ace in the Hole not rewarded'),
-- 272 (Alliance val'kyr layer)
(26,272,28,0,0,47,0,27205,64,0,0,0,0,'','Andorhal 04A - The Depravity of the Forsaken rewarded'),
(26,272,28,0,0,47,0,27206,64,0,1,0,0,'','Andorhal 04A - Alas, Andorhal not rewarded'),
(26,272,28,0,1,47,0,27201,10,0,0,0,0,'','Andorhal 04A - Val''kyr Incursion in the log'),
(26,272,28,0,2,47,0,27202,10,0,0,0,0,'','Andorhal 04A - Brother Against Brother in the log'),
(26,272,28,0,3,47,0,27204,10,0,0,0,0,'','Andorhal 04A - Aradne in the log'),
-- 185 the living Alliance townsfolk
(26,185,28,0,0,47,0,27089,74,0,1,0,0,'','Alliance townsfolk - Ace in the Hole not taken'),
(26,185,28,0,0,47,0,27206,64,0,1,0,0,'','Alliance townsfolk - Alas, Andorhal not rewarded'),
-- 191 the town after the battle
(26,191,28,0,0,47,0,27089,64,0,0,0,0,'','Andorhal held by the Forsaken - Ace in the Hole rewarded'),
(26,191,28,0,0,47,0,27085,10,0,1,0,0,'','Andorhal held by the Forsaken - Supporting the Troops (H) not in the log'),
(26,191,28,0,1,47,0,27206,64,0,0,0,0,'','Andorhal held by the Forsaken - Alas, Andorhal rewarded');
