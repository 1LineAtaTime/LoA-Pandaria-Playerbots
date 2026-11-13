-- Eastern Plaguelands: the light above a tower shows once the caravan has reached it (Crown Guard, Eastwall, Northpass) or
-- The Plaguewood Tower (27532) has been handed in.
--
--  * Before: the four Northwatch Tower Stalkers 41178 (15 yd above the tops of Crown Guard, Eastwall, Northpass and the
--    Plaguewood Tower; both TDBs and mop) wear Tower Beam 77064 behind Generic Quest Invisibility 6 82358 (type 14), and
--    nothing in mop gives a type-14 detection, so no player ever saw a light.
--  * Sourced: "each time the caravan reaches a tower, a great orb of Light shines above it" (wiki, Fiona's Caravan); the
--    stalkers, their spots and template auras (both TDBs); the zone-wide See Quest Invis 6-9 spells 85817, 85097, 85315,
--    84889 (AreaGroup = zone 139) and their invisibility partners 82358, 85096, 78718, 84886 (types 14, 15, 16, 17).
--  * Ours: one type per tower so each light has its own window, and the windows: Crown Guard from Onward 27373 complete,
--    Eastwall from Boys Will Be Boys 27455 rewarded (the caravan's arrival there, 2026_11_14_00), Northpass from Nobody to
--    Blame but Myself 27489 complete, the Plaguewood Tower from The Plaguewood Tower 27532 rewarded; none ends (on the
--    user's question list). The per-spawn addon rows keep visibilityDistanceType 5 of the template (seen from afar).
--    No other creature in or near the zone wears type 14, 15, 16 or 17, and nothing else gives these detections.

DELETE FROM `creature_addon` WHERE `guid` IN (173051,175303,175314);
INSERT INTO `creature_addon` (`guid`,`path_id`,`mount`,`MountCreatureID`,`StandState`,`AnimTier`,`VisFlags`,`SheathState`,`PvPFlags`,`emote`,`aiAnimKit`,`movementAnimKit`,`meleeAnimKit`,`visibilityDistanceType`,`auras`) VALUES
(173051,0,0,0,0,0,1,1,0,0,0,0,0,5,'84886 77064'),  -- Crown Guard Tower, type 17
(175303,0,0,0,0,0,1,1,0,0,0,0,0,5,'85096 77064'),  -- Eastwall Tower, type 15
(175314,0,0,0,0,0,1,1,0,0,0,0,0,5,'78718 77064');  -- Northpass Tower, type 16
-- The Plaguewood Tower stalker 173036 keeps the template's 82358 (type 14).

DELETE FROM `spell_area` WHERE `spell` IN (84889,85097,85315,85817) AND `area`=139;
INSERT INTO `spell_area` (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`autocast`,`quest_start_status`,`quest_end_status`) VALUES
(84889,139,27373,0,0,0,2,1,66,0),   -- Crown Guard light (See Quest Invis 9, type 17)
(85097,139,27455,0,0,0,2,1,64,0),   -- Eastwall light (See Quest Invis 7, type 15)
(85315,139,27489,0,0,0,2,1,66,0),   -- Northpass light (See Quest Invis 8, type 16)
(85817,139,27532,0,0,0,2,1,64,0);   -- Plaguewood Tower light (See Quest Invis 6, type 14)
