-- Silverpine Forest quest targets that others help kill: credit whoever tagged them, even with less than half of the damage
-- (CREATURE_FLAG_EXTRA_NO_PLAYER_DAMAGE_REQ)
-- 44367 Forest Ettin (It's Only Poisonous if You Ingest It): the player's Bush Chicken Explosion! takes most of its health
-- 44793 Worgen Renegade (The Gilneas Liberation Front): fights the Forsaken Troopers around Forsaken High Command
UPDATE `creature_template` SET `flags_extra` = 4194304 WHERE `entry` IN (44367, 44793);
