-- Western Plaguelands, Zen'Kiki and the Cultists (26955): Zen'Kiki heals the Captive Plaguebears the player frees, and that heal
-- gives the credit (npc_wpl_captive_plaguebear, go_wpl_cultist_cage, spell_wpl_zenkiki_remove_curse).
--
--  * Sourced: the log text ("break 4 Captive Plaguebears out of their cages and allow Zen'Kiki to heal them"), the objective
--    "Captive Plaguebear freed and healed", Wowhead comments (the freed bear is healed by Zen'Kiki and turns into a Shaggy Brown
--    Bear 44903, whose coat reverts to the plagued hide when it despawns), Shaggy Brown Bear's level (38 in TDB 4.4.2
--    creature_template_difficulty, as the captive bear; mop had 1) and the bears' respawn (120 s in both TDBs; mop had 60 s).
--  * Ours: the chain (cage -> bear -> its opener's Zen'Kiki casts 83816 on it -> 44903 and the credit); the spell itself
--    (83816 Remove Curse, an unused spell in the quest's id block 83813-83819, nothing ties it to Zen'Kiki); that the cage alone no
--    longer credits; the condition limiting 83816 to Captive Plaguebears; and the bear's exit.
--  * Before: each cage's SmartAI gave KilledMonsterCredit 44902 to whoever opened it, Zen'Kiki or not; nothing healed or changed
--    the bear. Needs 2026_11_11_14 (Zen'Kiki can be asked back after a logout), or a relogged player could not finish.
--  * The cages themselves never despawn here (a door gets GO_FLAG_NODESPAWN at load), so their spawntimesecs is left alone.

-- The cages hand the opener to the bear instead of crediting him
DELETE FROM `smart_scripts` WHERE `source_type` = 1 AND `entryorguid` IN (205155,205156,205157,205158,205159,205160,207427,207428,207429,207430);
UPDATE `gameobject_template` SET `AIName` = '', `ScriptName` = 'go_wpl_cultist_cage' WHERE `entry` IN (205155,205156,205157,205158,205159,205160,207427,207428,207429,207430);

-- The bear waits for its opener's Zen'Kiki; healed, it is a level-38 Shaggy Brown Bear
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_wpl_captive_plaguebear' WHERE `entry` = 44902;
UPDATE `creature_template` SET `minlevel` = 38, `maxlevel` = 38 WHERE `entry` = 44903;
UPDATE `creature` SET `spawntimesecs` = 120 WHERE `id` = 44902;

-- Zen'Kiki's Remove Curse
DELETE FROM `spell_script_names` WHERE `spell_id` = 83816;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(83816, 'spell_wpl_zenkiki_remove_curse');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 83816;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,1,83816,0,0,31,0,3,44902,0,0,0,0,'','Remove Curse (Zen''Kiki and the Cultists) - only on Captive Plaguebears');
