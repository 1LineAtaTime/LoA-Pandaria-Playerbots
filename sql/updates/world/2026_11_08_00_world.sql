-- Gilneas, 26706 "Endgame": the static Orc Gunship 800100 crashed the client as soon as it came into view.
-- Its display 8253 (ND_HordeGunship) is a WMO. The client draws a WMO only for the GameObject types made for it
-- (transport, map object, moving transport, destructible building); for every other type it loads the display as an
-- M2 model, and a WMO display then crashes it. 800100 was type 5 (generic), so it crashed. Any other ship WMO display
-- would crash the same way; the type is what has to change.
-- Type 14 (map object) is the static WMO type. Server-side collision depends on the display only, so the deck stays
-- walkable for the boarding sequence.
UPDATE `gameobject_template` SET `type`=14 WHERE `entry`=800100;
