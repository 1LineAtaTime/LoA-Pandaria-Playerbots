-- Eastern Plaguelands, Just a Drop in the Bucket (27454): the Mereldar Plague Cauldron's explosions, and the disguise.
--
-- Before: "Throw in an entire flask" made the PLAYER cast 141954 "HUGE EXPLOSION" (a 5.x spell: knockback on everything within
-- 200 yd of the player) and "Throw in all 12 flasks" made the player cast 81639 "Render's Valley Explosion (NUKE)": knockback
-- plus 2,500 Fire damage within 10 yd of the player, about the whole health of a level-42 character, after which he fell. The
-- options showed without the Death Cultist Disguise, and page 2's first option read "Put in a drop of liquid" again.
--
-- What the player should see, sourced (comments 2010-11-27 [+191] with replies, 2010-11-30 [+42] [+25], 2010-11-29 [+8],
-- 2010-12-01 [+8], 2011-08-06 [+21]; the wiki; DBC):
--  * The options are there only in disguise ("Once you use the disguise, you can interact with the cauldron", wiki; objective
--    text). Without it, or once the quest is complete, the cauldron shows its text page only.
--  * One drop -> page 2 "Nothing seems to happen... What will you do?" with "Do nothing. Betina said only one drop was needed."
--    (broadcast 45727): quiet completion.
--  * An entire flask: a small blast at the cauldron; "it basically knocked me off my feet, as it did with all the cultists
--    around": 85172 "Cosmetic - Cauldron Explosion (AOE)", MOD_STUN 2 s on every unit within 20 yd of its caster.
--  * All 12 flasks: a big blast throws the player and the cultists about 100 yd away from the cauldron ("into the lake",
--    "nearly as far as Corin's Crossing"), no damage but the fall: 85173 "Cosmetic - BIG Cauldron Explosion (AOE)",
--    KNOCK_BACK 300 / 300 on every unit within 20 yd of its caster (about 93 yd here). Pets fly too.
--  Any of the three completes the quest (existing credit rows).
-- Ours:
--  * The caster: the Generic Bunny - PRK (Large AOI) 44775 standing in the cauldron, spawn 173064 (1.8 yd from the object).
--    Entry 44775 has 92 spawns in 14 zones, so it is addressed by its spawn only (SMART_TARGET_CREATURE_GUID 173064 as the
--    caster of a cross-cast from the cauldron's SmartAI) and keeps no AI of its own; its template is untouched. The spells
--    are centred on their caster (DEST_CASTER), so the blast comes from the cauldron and pushes outward from it; the bunny
--    casts them at itself, the explicit target is dropped (Spell::InitExplicitTargets).
--  * The bunny stands inside its own 20 yd area: a condition keeps that one spawn out (else it stunned and threw itself).
--  * Triggered casts. Nobody enters combat: the bunny is faction 35, both spells carry NO_INITIAL_AGGRO / NO_THREAT.
-- A thrown cultist (Vile Tutors, Dark Summoners: random movers) lands up to ~93 yd away and wanders around where it landed
-- (EffectMovementGenerator::Finalize re-initialises the random movement there) until it fights, evades home or respawns.

-- 1. The explosions: the bunny 173064 casts them (rows 3, 6, 9, 12 of the cauldron; credit and close-gossip rows unchanged)
DELETE FROM `smart_scripts` WHERE `entryorguid` = 205485 AND `source_type` = 1 AND `id` IN (3, 6, 9, 12);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(205485, 1, 3, 4, 61, 0, 100, 0, 0, 0, 0, 0, 0, 86, 85172, 2, 10, 173064, 44775, 0, 10, 173064, 44775, 0, 0, 0, 0, 0, 0, 'Mereldar Plague Cauldron - Linked - Bunny 173064 Casts Cosmetic - Cauldron Explosion (AOE)'),
(205485, 1, 6, 7, 61, 0, 100, 0, 0, 0, 0, 0, 0, 86, 85173, 2, 10, 173064, 44775, 0, 10, 173064, 44775, 0, 0, 0, 0, 0, 0, 'Mereldar Plague Cauldron - Linked - Bunny 173064 Casts Cosmetic - BIG Cauldron Explosion (AOE)'),
(205485, 1, 9, 10, 61, 0, 100, 0, 0, 0, 0, 0, 0, 86, 85172, 2, 10, 173064, 44775, 0, 10, 173064, 44775, 0, 0, 0, 0, 0, 0, 'Mereldar Plague Cauldron - Linked - Bunny 173064 Casts Cosmetic - Cauldron Explosion (AOE)'),
(205485, 1, 12, 13, 61, 0, 100, 0, 0, 0, 0, 0, 0, 86, 85173, 2, 10, 173064, 44775, 0, 10, 173064, 44775, 0, 0, 0, 0, 0, 0, 'Mereldar Plague Cauldron - Linked - Bunny 173064 Casts Cosmetic - BIG Cauldron Explosion (AOE)');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` IN (85172, 85173);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 85172, 0, 0, 31, 0, 3, 44775, 173064, 1, 0, 0, '', 'Cosmetic - Cauldron Explosion (AOE) - not the cauldron bunny that casts it'),
(13, 1, 85173, 0, 0, 31, 0, 3, 44775, 173064, 1, 0, 0, '', 'Cosmetic - BIG Cauldron Explosion (AOE) - not the cauldron bunny that casts it');

-- 2. The options need the disguise as well as the quest in progress (the existing condition 9)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 12043 AND `ConditionTypeOrReference` = 1;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 12043, 0, 0, 0, 1, 0, 85141, 0, 0, 0, 0, 0, '', 'Mereldar Plague Cauldron - needs the Death Cultist Disguise'),
(15, 12043, 1, 0, 0, 1, 0, 85141, 0, 0, 0, 0, 0, '', 'Mereldar Plague Cauldron - needs the Death Cultist Disguise'),
(15, 12043, 2, 0, 0, 1, 0, 85141, 0, 0, 0, 0, 0, '', 'Mereldar Plague Cauldron - needs the Death Cultist Disguise');
UPDATE `conditions` SET `Comment` = 'Mereldar Plague Cauldron - only while Just a Drop in the Bucket is incomplete'
  WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 12043 AND `ConditionTypeOrReference` = 9 AND `ConditionValue1` = 27454;

-- 3. Page 2's first option is "Do nothing" (broadcast 45727; wiki)
UPDATE `gossip_menu_option` SET `OptionText` = 'Do nothing.  Betina said only one drop was needed.', `OptionBroadcastTextID` = 45727 WHERE `MenuID` = 12044 AND `OptionID` = 0;

-- 4. The bunny stood with MovementType 1 / wander 3 yd, so the blast's centre drifted around the cauldron: this spawn only stands
--    still (the entry's other 91 spawns are untouched)
UPDATE `creature` SET `MovementType` = 0, `wander_distance` = 0 WHERE `guid` = 173064 AND `id` = 44775;
