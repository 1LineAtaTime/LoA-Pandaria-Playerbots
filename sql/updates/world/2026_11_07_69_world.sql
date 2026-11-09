-- Syndicate Thief 24477 carried UNIT_FLAG_STUNNED in its template, the only creature of the zone that does; no stun aura goes with it, and
-- Spell::CheckCasterAuras fails its Backstab, Disarm and Poison SmartAI casts on that flag.
-- Sourced: TDB 442 and 1210 have no stun flag on it. Ours: 32768 as mop's other Syndicate entries (TDB: 0), keeping their CAN_SWIM.
UPDATE `creature_template` SET `unit_flags`=32768 WHERE `entry`=24477;
