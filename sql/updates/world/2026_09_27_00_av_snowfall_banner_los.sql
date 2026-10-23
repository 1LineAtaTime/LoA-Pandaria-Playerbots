-- #167: AV Snowfall Banner (GO 180418) needs losOK (data16) = 1, as AzerothCore; with 0 the core never
-- treats a click as an assault (SpellEffects.cpp EffectOpenLock losOK check) and Snowfall can never be captured.
UPDATE `gameobject_template` SET `data16` = 1 WHERE `entry` = 180418;
