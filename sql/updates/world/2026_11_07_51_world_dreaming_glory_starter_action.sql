-- Spell.dbc 28698 is Dreaming Glory (herbalism effect), not a starter ability.
-- An invalid spell button 7 was copied into 110 race/class starting bars and
-- recreated on every new character/bot randomization. Preserve all other slots.
-- Reload starting player information by a normal worldserver restart afterward.
DELETE FROM `playercreateinfo_action`
WHERE `button` = 7 AND `action` = 28698 AND `type` = 0;
