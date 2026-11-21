-- Eastern Plaguelands, Scourged Mass (27528): Kevin Frost's Crusader's Torch 61369 (Burn Corpse 85555) burns only a dead
-- Overstuffed Golem 45851 that is not burning yet, once per corpse, and the burned corpse goes 5 s later.
--
--  * Before: 85555 (eff0 dummy aura on the nearest 45851 within 7 yd, eff1 credit 45911 to the caster) had only an entry
--    condition and can target the dead and the living (AttributesEx2 0x1), so using the torch beside a living golem, or eight
--    times on one corpse (item cooldown 1 s), finished the quest.
--  * Sourced: "Kill Overstuffed Golems, then use the torch on their corpses" (objectives; 2011-02-13 "waiting to be killed and
--    burned"); "burn their bodies ... destroy the vile matter" (description): a body burns once.
--  * How (verified in code): the burn aura sticks on a corpse (85555 is DEATH_PERSISTENT, AttributesEx3 0x100000), so the
--    negative aura condition sees an earlier burn; SmartAI's SpellHit runs on a dead golem, so the corpse's spellhit row
--    despawns it; a despawned corpse loses its auras, so the respawned golem burns again.
--  * The credit (effect 1, KILL_CREDIT2 to the caster) has no target of its own, and when effect 0 finds no corpse this core
--    carries on with the cast (Spell::SelectImplicitNearbyTargets returns quietly), so the conditions alone still let eight
--    uses anywhere complete the quest. spell_epl_burn_corpse refuses the torch with no burnable corpse within 7 yd ("No valid
--    targets") and gives the credit only when effect 0 has its corpse.
--  * Group: KILL_CREDIT2 is the DBC's group form of the credit (Player::RewardPlayerAndGroupAtEvent), so one burned corpse
--    counts for the burner's group members nearby, as the spell has it; the comments say nothing either way.
--  * Ours: the corpse goes 5 s after the burn, with any loot left on it (loot first).

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceGroup` = 1 AND `SourceEntry` = 85555;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,1,85555,0,0,31,0,3,45851,0,0,0,0,'','Burn Corpse - target is an Overstuffed Golem'),
(13,1,85555,0,0,36,0,0,0,0,1,0,0,'','Burn Corpse - target is dead'),
(13,1,85555,0,0,1,0,85555,0,0,1,0,0,'','Burn Corpse - target is not burning yet');

DELETE FROM `smart_scripts` WHERE `entryorguid` = 45851 AND `source_type` = 0 AND `id` = 2;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`target_type`,`comment`) VALUES
(45851,0,2,0,8,0,100,0,85555,0,0,0,41,5000,0,1,'Overstuffed Golem - On Spellhit Burn Corpse - Despawn after 5 s');

DELETE FROM `spell_script_names` WHERE `spell_id` = 85555;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(85555,'spell_epl_burn_corpse');
