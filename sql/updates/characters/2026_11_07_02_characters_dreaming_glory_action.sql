-- Repair only the same invalid starter button on existing characters, and only
-- where the character does not actually know this spell. No spells are removed.
DELETE ca FROM `character_action` ca
WHERE ca.`button` = 7 AND ca.`action` = 28698 AND ca.`type` = 0
  AND NOT EXISTS (SELECT 1 FROM `character_spell` cs
                  WHERE cs.`guid` = ca.`guid` AND cs.`spell` = 28698);
