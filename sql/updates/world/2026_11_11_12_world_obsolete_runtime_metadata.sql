-- Remove only wholly unreferenced empty pool definitions. These cannot spawn
-- an entity: preserve all creature/GO/quest/event and parent/child dependencies.
-- Back up pool_template and command/addon/details before applying.
DELETE p FROM pool_template p
WHERE NOT EXISTS(SELECT 1 FROM pool_creature c WHERE c.pool_entry=p.entry)
 AND NOT EXISTS(SELECT 1 FROM pool_gameobject g WHERE g.pool_entry=p.entry)
 AND NOT EXISTS(SELECT 1 FROM pool_pool x WHERE x.pool_id=p.entry OR x.mother_pool=p.entry)
 AND NOT EXISTS(SELECT 1 FROM pool_quest q WHERE q.pool_entry=p.entry)
 AND NOT EXISTS(SELECT 1 FROM game_event_pool e WHERE e.pool_entry=p.entry);
-- Obsolete overrides for commands absent from this native build. RBAC and
-- working commands are unchanged; removing these rows does not add features.
DELETE FROM command WHERE name IN ('guard delete','guard global','guard handle','guard player','isengard daily','itemlog','npc set spawndist','quest broken','quest broken add','quest broken reload','quest broken remove','reload locales_creature','reload locales_creature_text','reload locales_gameobject','reload locales_gossip_menu_option','reload locales_item','reload locales_npc_text','reload locales_page_text','reload locales_points_of_interest','reload locales_quest','spectate','spectate restoreviewpoint','spectate setviewpoint');
-- One-shot animations are invalid persistent addon states; the loader already
-- ignored these values. Clear the invalid state rather than invent an animation.
UPDATE creature_addon SET emote=0 WHERE emote IN (1,7,14,15,18,20,36,45,48,50,71,380);
-- These dialogue animation IDs do not exist in the loaded client DBC.
UPDATE quest_details SET Emote1=0 WHERE Emote1 IN (1024);
-- These dialogue animation IDs do not exist in the loaded client DBC.
UPDATE quest_details SET Emote2=0 WHERE Emote2 IN (1000,2000,3000);
-- These dialogue animation IDs do not exist in the loaded client DBC.
UPDATE quest_details SET Emote3=0 WHERE Emote3 IN (1000,2000,3000,5000);
-- These dialogue animation IDs do not exist in the loaded client DBC.
UPDATE quest_details SET Emote4=0 WHERE Emote4 IN (1000,1500,2000,3000);
