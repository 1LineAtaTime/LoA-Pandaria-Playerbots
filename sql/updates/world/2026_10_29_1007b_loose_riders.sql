-- #1007 / #1007b: second pass on loose vehicle riders. The first pass only caught rider spawns within 3 yd of their vehicle
-- (and the Stonard kodo set, 986b_01). match2.py found 51 more rider spawns 3-15 yd from a vehicle that already seats the same
-- entry through vehicle_template_accessory (unreferenced, static, same phase). Lane H (v20, headless client in view of each) checked
-- every one live: the vehicle spawn is present, a runtime-seated rider of the accessory entry sits on it (spawn id 0) AND the DB
-- spawn below stands on its own (spawn id = the guid below) -> a second, loose rider. Types: Wildhammer Stormtalon 49683/49684 (10),
-- Gnomish Bomber 42015/42017 (14), Wintergarde Gryphon 27661/27662 (2), Hyldsmeet Warbear 30174/30175 (8), Skybreaker Recon
-- Fighter 32189/32190 (2) = 36.
-- NOT touched (not confirmed): Arachnopod Destroyer 34183/34184 x6 (Ulduar) and Lord Rhyolith 52558/53087 (Firelands) - raid maps the
-- headless client cannot enter; KTC Waitress 48721/48342 x2 (Kezan phase 2, not visible); Niffelem Frost Giant 29351/29558 (rider
-- in another phase, not visible); Shang Xi's Hot Air Balloon 55918/56662 (no seated rider at all, the DB spawn is the only one).
-- Backup of the deleted rows: (not shipped)  (undo: run that file).
DELETE FROM `creature_addon` WHERE `guid` IN (113202,113205,201015,201056,201060,201064,201068,201072,201079,201094,201104,201111,308457,308459,308460,308461,308462,308463,308465,308468,309578,309657,309891,310018,310050,310087,310251,310256,310308,310412,310457,310557,310564,310588,371073,371085);
DELETE FROM `creature` WHERE `guid` IN (113202,113205,201015,201056,201060,201064,201068,201072,201079,201094,201104,201111,308457,308459,308460,308461,308462,308463,308465,308468,309578,309657,309891,310018,310050,310087,310251,310256,310308,310412,310457,310557,310564,310588,371073,371085) AND `id` IN (49684,42017,27662,30175,32190);
