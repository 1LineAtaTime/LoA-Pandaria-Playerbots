-- Matching native encounter from SF release26/27; do not run the alternative
-- periodic-dummy spawner at the same time as Sean Dempsey's wave controller.
UPDATE creature_template SET AIName='',ScriptName='npc_sean_dempsey' WHERE entry=35081;
UPDATE creature_template SET AIName='',ScriptName='npc_lord_darius_crowley_c1' WHERE entry=35077;
UPDATE creature_template SET AIName='',ScriptName='npc_worgen_alpha_c1' WHERE entry=35170;
UPDATE creature_template SET AIName='',ScriptName='npc_worgen_alpha_c2' WHERE entry=35167;
UPDATE creature_template SET AIName='',ScriptName='npc_worgen_runt_c1' WHERE entry=35188;
UPDATE creature_template SET AIName='',ScriptName='npc_worgen_runt_c2' WHERE entry=35456;
UPDATE creature_template SET faction=2179,faction_A=2179,faction_H=2179 WHERE entry IN (35170,35167,35188,35456);
DELETE FROM spell_script_names WHERE ScriptName='spell_gen_gilneas_prison_periodic_dummy';
