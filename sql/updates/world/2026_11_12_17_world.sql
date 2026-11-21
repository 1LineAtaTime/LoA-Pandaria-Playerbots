-- Second battle for Andorhal: all 75 Andorhal Defenders stand in the Horde-vs-Alliance layer again.
-- Reported in game (Horde, Supporting the Troops 27085) after 2026_11_12_16: "The distribution of defenders is definitely too low"
-- (21 of 75 were left in phase 261; the other 54 had gone to the val'kyr layer 272).
-- Ours: the 54 Defenders of 2026_11_12_16 go to phase 261 as well. A Defender in 261 only and a val'kyr-layer Forsaken (45235,
-- 45239, 45240, 45242, phase 272 only) share no phase, so they can neither see nor fight each other (WorldObject::CanNeverSee),
-- which keeps the fix for the original report ("fighting creatures Thanarok cannot see"). Cost: an Alliance player in 272 (after
-- The Depravity of the Forsaken 27205; every such player is also in 261) sees these Defenders standing beside the val'kyr layer's
-- troopers without fighting them. The troopers and val'kyr stay the Alliance quests' targets (27201, 27202, 27204).
UPDATE `creature` SET `phaseId`=261, `phaseGroup`=0 WHERE `id`=44448 AND `map`=0 AND `zoneId`=28 AND `phaseId`=272;
