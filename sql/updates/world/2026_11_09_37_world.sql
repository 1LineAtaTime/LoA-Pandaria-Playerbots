-- The Hinterlands: quest NPCs and spell targets that should stand still.
-- Sourced: TDB 4.4.2 and 1210 spawn the three Shadra'Alor altar bunnies (43067-43069, the targets of Summoning Shadra), Ongo'longo 42815
-- and Qiaga the Keeper 42919 standing on these spots. mop let them wander 3 yd.
-- Ours: Ongo'longo's two shackle bunnies (42816, 42817; no TDB spawn) stand with him.
UPDATE `creature` SET `MovementType`=0,`wander_distance`=0 WHERE `guid` IN (191291,191292,191293,192303,192304,192305,192024) AND `id` IN (43067,43068,43069,42815,42816,42817,42919);
