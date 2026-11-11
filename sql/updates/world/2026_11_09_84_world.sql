-- Lilian Voss 59200 (Scholomance): creature_template_movement marks her Rooted, as does the 12.1 TDB (StaticFlags1 SESSILE,
-- with no script there). Ours: this core's boss_lilian_voss walks her to her defeat point and to Gandling's door, makes
-- her wander in the soul phase and jumps her for Shadow Shiv, so once Rooted is applied she must not be rooted.
UPDATE `creature_template_movement` SET `Rooted`=0 WHERE `CreatureId`=59200;
