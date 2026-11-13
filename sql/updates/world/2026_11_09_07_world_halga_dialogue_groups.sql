-- Halga Bloodeye's caravan scripts use Talk groups 0, 1 and 2. The imported
-- texts were all alternatives in group 0, leaving groups 1/2 unresolved and
-- causing the SmartAI action for group 1 to be rejected at startup.
-- Preserve the original text, broadcast IDs, sound, locale and attribution.
-- Idempotent on the LoA baseline; no invented dialogue or quest completion.
UPDATE creature_text SET GroupID=2, ID=0 WHERE CreatureID=34258 AND GroupID=0 AND ID=2;
UPDATE creature_text SET GroupID=1, ID=0 WHERE CreatureID=34258 AND GroupID=0 AND ID=1;
