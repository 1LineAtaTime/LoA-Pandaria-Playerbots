-- Shadow Prison 46558 (Scholomancer): always the invisible model, as TDB 4.4.2 and 12.1.0.
-- Sourced: both TDBs give display 11686 (invisible trigger) probability 1 and display 1126 probability 0, both at scale 1.
-- mop had 1126 at probability 1 too, so about half the prisons that lift the player were visible.
UPDATE `creature_template_model` SET `Probability`=1, `DisplayScale`=1 WHERE `CreatureID`=46558 AND `Idx`=0;
UPDATE `creature_template_model` SET `Probability`=0, `DisplayScale`=1 WHERE `CreatureID`=46558 AND `Idx`=1;
