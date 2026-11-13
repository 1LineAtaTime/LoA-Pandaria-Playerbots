-- Runtime repair: assign the opening speech to the opening Liam variant that owns text groups 0..2.
UPDATE creature_template SET ScriptName='' WHERE entry=36451 AND ScriptName='npc_prince_liam_greymane';
UPDATE creature_template SET ScriptName='npc_prince_liam_greymane',AIName='' WHERE entry=34850 AND ScriptName='';
UPDATE creature_template SET AIName='SmartAI' WHERE entry=25220 AND AIName='' AND ScriptName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=36585 AND t.ScriptName='npc_pant_gob' AND t.AIName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=44592 AND t.ScriptName='npc_silverpine_fallen_human' AND t.AIName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=44593 AND t.ScriptName='npc_silverpine_fallen_human' AND t.AIName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=44615 AND t.ScriptName='npc_silverpine_grand_executor_mortuus' AND t.AIName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=44821 AND t.ScriptName='npc_silverpine_forsaken_bat' AND t.AIName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=44825 AND t.ScriptName='npc_silverpine_bat_handler_maggotbreath' AND t.AIName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=44882 AND t.ScriptName='npc_silverpine_deathstalker_rane_yorick' AND t.AIName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=44883 AND t.ScriptName='npc_silverpine_lord_darius_crowley_exsanguinate' AND t.AIName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=44884 AND t.ScriptName='npc_silverpine_packleader_ivar_bloodfang_exsanguinate' AND t.AIName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=44893 AND t.ScriptName='npc_silverpine_armoire' AND t.AIName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=44914 AND t.ScriptName='npc_silverpine_orc_sea_pup' AND t.AIName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=44915 AND t.ScriptName='npc_silverpine_orc_crate' AND t.AIName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=44916 AND t.ScriptName='npc_silverpine_admiral_hatchet' AND t.AIName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=44935 AND t.ScriptName='npc_silverpine_mutant_bush_chicken' AND t.AIName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=44941 AND t.ScriptName='npc_silverpine_webbed_victim' AND t.AIName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=44942 AND t.ScriptName='npc_silverpine_orc_sea_dog' AND t.AIName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=44951 AND t.ScriptName='npc_silverpine_agatha_fenris_isle' AND t.AIName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=44958 AND t.ScriptName='npc_silverpine_forsaken_trooper_fenris_isle' AND t.AIName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=45003 AND t.ScriptName='npc_silverpine_fenris_keep_camera' AND t.AIName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=45051 AND t.ScriptName='npc_silverpine_sylvanas_windrunner_lordaeron' AND t.AIName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=45057 AND t.ScriptName='npc_silverpine_forsaken_warhorse_lordaeron' AND t.AIName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=45228 AND t.ScriptName='npc_silverpine_master_forteski' AND t.AIName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=45318 AND t.ScriptName='npc_silverpine_arthura_forward_command' AND t.AIName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=45474 AND t.ScriptName='npc_silverpine_belmont_great_escape' AND t.AIName='';
UPDATE creature_template SET AIName='SmartAI' WHERE entry=46714 AND AIName='' AND ScriptName='';
UPDATE creature_template SET AIName='SmartAI' WHERE entry=46775 AND AIName='' AND ScriptName='';
DELETE s FROM smart_scripts s JOIN creature_template t ON t.entry=s.entryorguid WHERE s.source_type=0 AND t.entry=69993 AND t.ScriptName='npc_young_primal_devilsaur' AND t.AIName='';

-- Opening Liam's three legacy SmartAI timers duplicate the native speech sequence.
DELETE FROM smart_scripts WHERE entryorguid=34850 AND source_type=0 AND id IN (0,1,2) AND event_type=1 AND action_type=1 AND action_param1 IN (0,1,2) AND target_type=1;
