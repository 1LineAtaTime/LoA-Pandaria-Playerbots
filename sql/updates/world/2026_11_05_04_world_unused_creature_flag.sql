-- 0x00800000 is CREATURE_FLAG_EXTRA_UNUSED_23, forbidden in DB_ALLOWED.
-- ObjectMgr already strips this unsupported bit at runtime. Normalize only
-- these reviewed custom templates, retaining 0x2 CIVILIAN and every other field.
UPDATE creature_template SET flags_extra=2
WHERE entry IN (190000,190001,190002,190003,190004,190005,190006,190007,190008,190009,190010,190011,190013)
  AND flags_extra=8388610;
