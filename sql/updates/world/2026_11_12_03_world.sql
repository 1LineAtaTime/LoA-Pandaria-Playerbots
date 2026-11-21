-- Western Plaguelands, I Ain't Sayin' You a Gourd-Digger... (26956): the crop monsters come out of the plants the player clicks;
-- they don't wander Dalson's Farm beforehand.
--
--  * Sourced: Wowhead comments on 26956 ("Click on them and its turns into a mob to kill"; "I picked up an apple... THEN A SKELETON
--    POPPED OUT"); neither TDB (4.4.2, 12.1.0) spawns a Thrashing Pumpkin 44487, Pome Wraith 44488 or Corn Stalker 44489; the
--    sparkling plants 205052 / 205053 / 205054 (spellcasters, 1 charge, 300 s respawn) cast I Ain't Sayin': Summon Thrashing
--    Pumpkin / Pome Wraith / Corn Stalker 83409-83411 on the player. Those summons last 120 s (SpellDuration index 4,
--    SummonProperties 64 = wild, so a timed despawn), so one left alone goes by itself.
--  * mop had 28 static copies (random movement, 60 s respawn) wandering the fields next to the Menders' Stead. Nothing else refers
--    to these guids (no addon, formation, pool, event, linked respawn or per-guid script).

DELETE FROM `creature` WHERE `id` IN (44487, 44488, 44489) AND `guid` IN (311563,311564,311565,311566,311567,311568,311569,311570,311571,311572,311573,311574,311575,311576,311577,311578,311579,311580,311581,311582,311583,311584,311585,311586,311587,311588,311589,311592);
