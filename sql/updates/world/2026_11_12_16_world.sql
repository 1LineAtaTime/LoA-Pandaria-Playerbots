-- Second battle for Andorhal: no soldier fights across the two layers any more.
-- Reported in game (Horde, Ashes to Ashes 27086 / Supporting the Troops 27085): "The Andorhal defenders are fighting creatures
-- Thanarok cannot see." The Alliance army (Andorhal Defender 44448, Force-Commander 44450, Provincial Minuteman 44944) stood in
-- phase group 519 = [261, 272], so it saw both the Forsaken army of 261 and the Alliance-only val'kyr layer of 272 (Lesser Val'kyr
-- 45239, Forsaken Troopers 45240/45242, Aradne 45235), and fought both. A player in 261 without 272 (every Horde player, and the
-- Alliance before The Depravity of the Forsaken 27205 is handed in) saw Defenders and minutemen swinging at nothing; with the 85%
-- sparring limits of 2026_11_12_11/_15 those fights never ended.
-- Sourced: phases 261 "Andorhal 03 - Horde vs. Alliance" and 272 "Andorhal 04A - Val'kyr Attack (A)" (Phase.dbc); the quest texts
-- 27085 ("slay the human soldiers that are fighting with our deathguards"), 27086 ("deep within Alliance-held Andorhal ... their
-- commanders ... do not fight"), 27198 ("Our defenders are equally matched with their deathguards"), 27202 ("you will need to
-- fight them too, along with the deathguards and val'kyr").
-- Ours: the split. No TDB spawns either army; group 519 was our first pass's choice (2026_11_10). Every player in 272 is also in
-- 261 (conditions 26/261 include 27205 and every later Alliance quest), so each soldier now stands in exactly one layer and only
-- meets opponents everyone who sees it also sees:
--  * 54 Defenders who stand among the val'kyr layer (a 272 opponent within 28 yards, 20 aggro + 2 x 3 wander + 2, and nearer than
--    any Deathguard or Behemoth; LoA's own trooper spawns 373593-373714 were placed 0.6-6 yards from them): phase 272. The Alliance
--    after 27205 sees them as before; the Horde and the Alliance before 27205 no longer see them.
--  * 21 Defenders, all 23 Force-Commanders (IMMUNE_TO_NPC: they never fight an NPC, and 27086 sends the Horde west for them) and
--    all 48 minutemen (the Horde's 27084 militia, fighting the Behemoths and Deathguards): phase 261. The six Lesser Val'kyr over
--    the minutemen (311828, 311830, 311841-311843, 311845) no longer fight them; Alliance players in 272 see them hover there.

-- 272 Andorhal 04A - Val'kyr Attack (A): the Defenders of the val'kyr layer
UPDATE `creature` SET `phaseId`=272, `phaseGroup`=0 WHERE `id`=44448 AND `guid` IN
(311847,311849,311852,311853,311854,311855,311858,311861,311862,311863,311866,311867,311868,311880,311889,311896,311903,311904,
 311907,311909,311910,311911,311915,311916,311917,311918,311919,311927,311943,311944,311946,311948,311950,311951,311981,311982,
 311984,311986,311987,311991,311992,311994,311996,311998,312000,312001,312002,312003,312004,312006,312007,312010,312014,312039);

-- 261 Andorhal 03 - Horde vs. Alliance: the rest of the army
UPDATE `creature` SET `phaseId`=261, `phaseGroup`=0 WHERE `id`=44448 AND `guid` IN
(311819,311835,311894,311908,311912,311913,311914,311920,311940,311941,311942,311945,311952,311975,311979,311980,311983,311989,
 311990,311993,311997);
UPDATE `creature` SET `phaseId`=261, `phaseGroup`=0 WHERE `map`=0 AND `zoneId`=28 AND `id` IN (44450,44944) AND `phaseGroup`=519;
