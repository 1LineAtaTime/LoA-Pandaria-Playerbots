-- Quest 29791 "The Suffering of Shen-zin Su" (Wandering Isle, epic #440): Aysa Cloudsinger 56661 rides Shang Xi's
-- Hot Air Balloon (55649) as a vehicle accessory (vehicle_template_accessory 55649 seat 1) and owns the balloon
-- dialogue (creature_text 56661 groups 0-10). Her ScriptName 'npc_aysa_lake_escort' is the old Way of the Tushui lake
-- escort AI: 2.5 s after spawning it says its line 0 and jumps her to 1216,3499,91 (Fu's Pond). The lake escort is
-- now done by 59652 (SmartAI of 54567 on accepting 29414), so 56661 is only ever the balloon passenger. The MoP 5.4.8
-- data has 56661 with an empty ScriptName. Pairs with the npc_shang_xi_air_balloon fix in
-- zone_wandering_island_west.cpp (speakers looked up by entry, landing at Mandori instead of the sea).
--
-- Rollback: UPDATE `creature_template` SET `ScriptName` = 'npc_aysa_lake_escort' WHERE `entry` = 56661;

UPDATE `creature_template` SET `ScriptName` = '' WHERE `entry` = 56661;
