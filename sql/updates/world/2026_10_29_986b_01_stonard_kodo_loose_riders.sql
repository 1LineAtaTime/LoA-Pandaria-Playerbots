-- #1007 / #986b: the first #1007 pass only caught rider spawns within 3 yd of their vehicle. Stonard Kodo Beast 46748 (Swamp of
-- Sorrows) carries its Stonard Wardrummer 46749 as a vehicle_template_accessory (summoned seated at runtime); the 12 DB spawns of
-- 46749 are the sniffed copies of those same riders, standing 3-13 yd from the kodo (z +0.6..+4.3 = seat height), so the client sees
-- a seated rider plus a loose one (live check v19: kodo 190989 + accessory + loose spawn 190990 at 9.5 yd). None is referenced by
-- smart_scripts / formations / pools / game events / paths. Backup: (not shipped)
DELETE FROM `creature_addon` WHERE `guid` IN (190513,190980,190990,191119,191122,191125,191129,191131,191134,191140,191142,191146);
DELETE FROM `creature` WHERE `guid` IN (190513,190980,190990,191119,191122,191125,191129,191131,191134,191140,191142,191146) AND `id` = 46749;
