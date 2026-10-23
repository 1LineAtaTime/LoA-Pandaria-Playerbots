-- #667 Deadmines: Lumbering Oaf 47297 had exp=0 while every other Deadmines boss (Glubtok 47162, Helix 47296, Ripsnarl 47626,
-- Cookie 47739) has exp=3. With heroic level 86, basehp0 is 1, so the heroic Oaf had 40 HP (1 x health_mod 40) and died to the first
-- hit, skipping the Helix-on-Oaf phase. The MoP 5.4.8 data has the same exp=0. Undo: SET exp=0.
UPDATE `creature_template` SET `exp` = 3 WHERE `entry` = 47297;
