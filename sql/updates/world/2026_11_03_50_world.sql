-- Hillsbrad Foothills, the Sludge Fields: Master Apothecary Lydon 48020 and Johnny Awesome 48021 go with the player from Freedom
-- for Lydon (28209) until A Blight Upon the Land (28237) is turned in, as on retail (warcraft.wiki.gg, Freedom for Lydon: "Upon
-- completion, Lydon and Johnny accompany the player"; Wowhead comments on Protocol 28230: "Johnny Awesome and Lydon will help with
-- this and all the following q leading up to A Blight Upon The Land"). mop had them as two static spawns in phase 188 instead
-- (2026_10_28_41), Lydon beside his cage and Johnny next to Blacksmith Verringtan; neither TDB has a static spawn of either.
-- 5.4.8 has the event's spells: Sludge Field Summon Controller 89292 (castable only in the Sludge Fields, area group 2831 = area
-- 286) and Summon Lydon/Summon Johnny 89295/89296 (SummonProperties 3045: ally guardians, not personal). Neither TDB has
-- spell_area, conditions or SmartAI for them; the quest bounds are ours, from the wiki and the comments. The summons are the
-- player's (spell_hillsbrad_sludge_field_summon_controller) and their AI follows and fights with the player
-- (npc_hillsbrad_sludge_fields_companion). Their quest giver and ender rows are kept: they start and end Protocol, Burnside Must
-- Fall and A Blight Upon the Land (Lydon), and Do it for Twinkles (Johnny), as in both TDBs and on Wowhead.

-- The controller in the Sludge Fields from Freedom for Lydon rewarded until A Blight Upon the Land is rewarded, and while Do it
-- for Twinkles (28231) is in the log (complete or not): it runs beside the chain, only Johnny ends it, and the farm's Johnny
-- 47869 still offers it after A Blight Upon the Land. The script then keeps only Johnny. No race mask: the quests gate race.
DELETE FROM `spell_area` WHERE `spell` IN (89292, 89295, 89296);
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `autocast`, `quest_start_status`, `quest_end_status`) VALUES
(89292, 286, 28209, 28237, 0, 0, 2, 1, 64, 11),
(89292, 286, 28231, 0, 0, 0, 2, 1, 10, 0);

DELETE FROM `spell_script_names` WHERE `spell_id` = 89292;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(89292, 'spell_hillsbrad_sludge_field_summon_controller');

UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_hillsbrad_sludge_fields_companion' WHERE `entry` IN (48020, 48021);
-- The caged Lydon: Johnny's plea as Freedom for Lydon is turned in
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_hillsbrad_caged_lydon' WHERE `entry` = 47900;

-- Johnny's lines (broadcast_text; the types are ours, the scream is 48283's own sound), personal: only his owner hears them
DELETE FROM `creature_text` WHERE `CreatureID` = 48021 AND `GroupID` IN (0, 1, 2);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(48021, 0, 0, 'You''re not leaving me to rot in this place! Take me with you!', 12, 0, 100, 0, 0, 0, 0, 48285, 5, 'Johnny Awesome - Freedom for Lydon turned in'),
(48021, 1, 0, 'TWINKLES! TWINKLES! NOOOOO!!', 14, 0, 100, 0, 0, 15131, 0, 48283, 5, 'Johnny Awesome - near Twinkles'),
(48021, 2, 0, 'I won''t let them get away with this, Twinkles. They will pay! They will all pay!', 12, 0, 100, 0, 0, 0, 0, 48284, 5, 'Johnny Awesome - near Twinkles');

-- The static stand-ins and the phase that only they used
DELETE FROM `creature_addon` WHERE `guid` IN (372480, 372482);
DELETE FROM `creature` WHERE `guid` IN (372480, 372482);
DELETE FROM `phase_area` WHERE `AreaId` = 267 AND `PhaseId` = 188;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 26 AND `SourceGroup` = 188 AND `SourceEntry` = 267;
