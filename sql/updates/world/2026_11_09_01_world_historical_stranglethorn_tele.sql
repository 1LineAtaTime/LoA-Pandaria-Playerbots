-- SkyFire contributors, GPL-3.0: historical release26 correction.
-- https://github.com/ProjectSkyfire/SkyFire_548/blob/6f000f1878c547f04680cd7c269388a245132ff2/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_20_world_06.sql
-- Also present in the official SFDB27 full dump. LoA retained z=10.1021.
-- Guard the complete old destination identity; preserve custom/renumbered teles.
UPDATE `game_tele` SET `position_z`=18.2021
WHERE `id`=956 AND `name`='StranglethornVale' AND `map`=0
  AND ABS(`position_x`-(-12644.3))<0.01
  AND ABS(`position_y`-(-377.411))<0.01
  AND ABS(`position_z`-10.1021)<0.001;
