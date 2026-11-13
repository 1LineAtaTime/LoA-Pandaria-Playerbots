-- Verified against this core and the installed 5.4.8 spell data. Keep the
-- actual MoP implementations; do not apply the broad September deletion script.
-- Last Stand 12975 natively applies its 30% health aura (250), and the old
-- dummy script requires absent trigger 12976. Divine Storm's old heal script
-- requires absent 54171; its correctly bound glyph heal remains active.
DELETE FROM spell_script_names WHERE spell_id=12975 AND ScriptName='spell_warr_last_stand';
DELETE FROM spell_script_names WHERE spell_id=53385 AND ScriptName='spell_pal_divine_storm';
-- 18461 is the Vanish trigger, not the 11327/115193 stealth aura. Preserve
-- spell_rog_vanish_trigger on 18461 and the actual aura bindings.
DELETE FROM spell_script_names WHERE spell_id=18461 AND ScriptName='spell_rog_vanish';
-- The Lich King's Soul Reaper uses the boss script, not the player DK execute.
-- Preserve spell_the_lich_king_soul_reaper on 69409 and player class bindings
-- 114866/130735/130736 (whose source hook now uses PERIODIC_DUMMY).
DELETE FROM spell_script_names WHERE spell_id=69409 AND ScriptName='spell_dk_soul_reaper';
