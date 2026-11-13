-- Requires native05e94aabf6: signed next-link loading, reverse links and eligibility.
-- Project SkyFire signed-link semantics retained without narrowing existing
-- negative previous-quest "taken" behavior. Back up quest tables and EXE/PDB.
-- The paired deployer rejects positive values beyond the signed native range.
ALTER TABLE `quest_template_addon`
    MODIFY COLUMN `NextQuestID` INT NOT NULL DEFAULT 0;
