-- Quest drops of The Heart of the Matter 28484, Yetimus the Yeti Lord 28485 and Stagwiches 28489.
-- Sourced: TDB 4.4.2 and TDB 1210 have Chance 100, QuestRequired 1 for Yetimus' Twisted Horn, Juicy Stag Meat and the Ferocious Yeti's
-- Beating Yeti Heart. mop had -57.6, -89.2 and -46.5.
-- Wowhead MoP Classic: its 59.12% (horn) and 80.35% (meat) count all kills, with or without the quest, so they are only lower bounds.
-- It also marks all three "progressive loot" (the rate rises with quest progress), which this core does not have; a flat 100 is the TDBs'.
-- Cave Yeti's heart row (-63.7) stays: item 64380 lists both yetis as droppers (Cave Yeti 3148/60353 kills, Ferocious Yeti 1645/5607),
-- so the row the TDBs lack is real, but no source gives its rate.
UPDATE `creature_loot_template` SET `ChanceOrQuestChance`=-100 WHERE (`entry`=47484 AND `item`=64386) OR (`entry`=48184 AND `item`=64410) OR (`entry`=48628 AND `item`=64380);
