-- Original LoA v1.0.4 uses explicit SmartAI with no C++ ScriptName for these
-- same entries/names. The installed core cannot resolve the stale C++ names
-- and already falls back to the explicit SmartAI. Preserve SmartAI and all
-- authored smart_scripts; do not erase unknown scripts on other creatures.
-- Source: 1LineAtaTime/LoA-Pandaria-Playerbots release v1.0.4 world.sql,
-- SHA256 of database ZIP db8dd845e3a5da4bc3a6f28afa68ed839aceb222101db949ae74840a5a2365a0.
UPDATE creature_template SET ScriptName='' WHERE entry=22953 AND name='Wrathbone Flayer' AND AIName='SmartAI' AND ScriptName='npc_wrathbone_flayer' AND EXISTS (SELECT 1 FROM smart_scripts WHERE entryorguid=22953 AND source_type=0);
UPDATE creature_template SET ScriptName='' WHERE entry=23216 AND name='Ashtongue Defender' AND AIName='SmartAI' AND ScriptName='npc_ashtongue_defender' AND EXISTS (SELECT 1 FROM smart_scripts WHERE entryorguid=23216 AND source_type=0);
UPDATE creature_template SET ScriptName='' WHERE entry=23318 AND name='Ashtongue Rogue' AND AIName='SmartAI' AND ScriptName='npc_ashtongue_rogue' AND EXISTS (SELECT 1 FROM smart_scripts WHERE entryorguid=23318 AND source_type=0);
UPDATE creature_template SET ScriptName='' WHERE entry=23523 AND name='Ashtongue Elementalist' AND AIName='SmartAI' AND ScriptName='npc_ashtongue_elementalist' AND EXISTS (SELECT 1 FROM smart_scripts WHERE entryorguid=23523 AND source_type=0);
UPDATE creature_template SET ScriptName='' WHERE entry=23524 AND name='Ashtongue Spiritbinder' AND AIName='SmartAI' AND ScriptName='npc_ashtongue_spiritbinder' AND EXISTS (SELECT 1 FROM smart_scripts WHERE entryorguid=23524 AND source_type=0);
UPDATE creature_template SET ScriptName='' WHERE entry=28033 AND name='Weslex Quickwrench' AND AIName='SmartAI' AND ScriptName='npc_the_etymidian' AND EXISTS (SELECT 1 FROM smart_scripts WHERE entryorguid=28033 AND source_type=0);
UPDATE creature_template SET ScriptName='' WHERE entry=30154 AND name='Agnetta Tyrsdottar' AND AIName='SmartAI' AND ScriptName='npc_agnetta_tyrsdottar' AND EXISTS (SELECT 1 FROM smart_scripts WHERE entryorguid=30154 AND source_type=0);
UPDATE creature_template SET ScriptName='' WHERE entry=35566 AND name='Lord Darius Crowley' AND AIName='SmartAI' AND ScriptName='npc_lord_darius_crowley_c3' AND EXISTS (SELECT 1 FROM smart_scripts WHERE entryorguid=35566 AND source_type=0);
UPDATE creature_template SET ScriptName='' WHERE entry=41027 AND name='Wormwing Screecher' AND AIName='SmartAI' AND ScriptName='0' AND EXISTS (SELECT 1 FROM smart_scripts WHERE entryorguid=41027 AND source_type=0);
UPDATE creature_template SET ScriptName='' WHERE entry=53565 AND name='Aspiring Trainee' AND AIName='SmartAI' AND ScriptName='npc_aspiring_trainee' AND EXISTS (SELECT 1 FROM smart_scripts WHERE entryorguid=53565 AND source_type=0);
UPDATE creature_template SET ScriptName='' WHERE entry=57748 AND name='Instructor Qun' AND AIName='SmartAI' AND ScriptName='npc_instructors' AND EXISTS (SELECT 1 FROM smart_scripts WHERE entryorguid=57748 AND source_type=0);
UPDATE creature_template SET ScriptName='' WHERE entry=58983 AND name='Riverblade Bloodletter' AND AIName='SmartAI' AND ScriptName='npc_bloodletter_ambush_b' AND EXISTS (SELECT 1 FROM smart_scripts WHERE entryorguid=58983 AND source_type=0);
UPDATE creature_template SET ScriptName='' WHERE entry=59073 AND name='Mayor Bramblestaff' AND AIName='SmartAI' AND ScriptName='npc_mayor_bramblestaff' AND EXISTS (SELECT 1 FROM smart_scripts WHERE entryorguid=59073 AND source_type=0);
UPDATE creature_template SET ScriptName='' WHERE entry=60031 AND name='Pao-kun the Pyromancer' AND AIName='SmartAI' AND ScriptName='npc_pao_kun_pyromancer' AND EXISTS (SELECT 1 FROM smart_scripts WHERE entryorguid=60031 AND source_type=0);
UPDATE creature_template SET ScriptName='' WHERE entry=60094 AND name='Ji-Lu''s Cart' AND AIName='SmartAI' AND ScriptName='npc_initial_cart' AND EXISTS (SELECT 1 FROM smart_scripts WHERE entryorguid=60094 AND source_type=0);
UPDATE creature_template SET ScriptName='' WHERE entry=60694 AND name='Sage Liao' AND AIName='SmartAI' AND ScriptName='npc_sage_liao_laying' AND EXISTS (SELECT 1 FROM smart_scripts WHERE entryorguid=60694 AND source_type=0);
UPDATE creature_template SET ScriptName='' WHERE entry=61303 AND name='Kobai' AND AIName='SmartAI' AND ScriptName='npc_kobai' AND EXISTS (SELECT 1 FROM smart_scripts WHERE entryorguid=61303 AND source_type=0);
UPDATE creature_template SET ScriptName='' WHERE entry=61518 AND name='Sra''thik Guard' AND AIName='SmartAI' AND ScriptName='npc_srathik_guard' AND EXISTS (SELECT 1 FROM smart_scripts WHERE entryorguid=61518 AND source_type=0);
UPDATE creature_template SET ScriptName='' WHERE entry=65469 AND name='Aspiring Trainee' AND AIName='SmartAI' AND ScriptName='npc_aspiring_trainee' AND EXISTS (SELECT 1 FROM smart_scripts WHERE entryorguid=65469 AND source_type=0);
