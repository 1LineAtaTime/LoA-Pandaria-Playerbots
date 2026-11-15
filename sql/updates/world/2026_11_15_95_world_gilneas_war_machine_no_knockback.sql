-- They Have Allies, But So Do We (24681): the Orcish War Machines (37921) are siege engines the player
-- destroys from a Glaive Thrower. Launch Glaive / Glaive Barrage knock their target back from the caster,
-- so every hit pushed a machine 4-10 yd further up the hill and out of the thrower's reach before it died.
-- Make the machines immune to knockback, like other siege units.
UPDATE `creature_template` SET `flags_extra` = `flags_extra` | 0x00200000 WHERE `entry` = 37921;
