-- ported from PR 15 2026_10_27_72_world.sql (#986)
-- Ruins of Gilneas: Forsaken Vanguard 307207 stands on the edge of the Forward Command phase box (2026_10_27_62) fighting two
-- Worgen Rebels of the Northern Headlands just outside it, who stay in the default phase; units in different phases do not see
-- each other, so it stood idle. It belongs to the Headlands fight, which goes on whatever the state of Forward Command (it has
-- no overrun twin); put it back in the default phase. It is the only holding-camp fighter whose nearest enemy is outside.
UPDATE `creature` SET `phaseId` = 0 WHERE `guid` = 307207 AND `id` = 45280;
