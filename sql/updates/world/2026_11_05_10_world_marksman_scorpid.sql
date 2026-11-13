-- Use the complete ordinary NPC Shoot rather than the later wrapper whose
-- force-cast companion88444 is absent from the 5.4.8 data. Both damage effects
-- are SCHOOL_DAMAGE, base23, explicit enemy target6; timers/flags stay intact.
UPDATE smart_scripts SET action_param1=6660
WHERE entryorguid=8338 AND source_type=0 AND id=0 AND action_type=11
  AND action_param1=88445 AND action_param2=64 AND target_type=2;
-- Envenom's warning belongs to the scorpid, directed at its victim. The old
-- target-as-speaker setting tried to find the scorpid text on the victim.
UPDATE smart_scripts SET action_param3=1
WHERE entryorguid=3125 AND source_type=0 AND id=2 AND event_type=61
  AND action_type=1 AND action_param1=0 AND action_param3=0 AND target_type=2;
