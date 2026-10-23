-- #635 Sethekk Halls heroic Anzu 1:1 (w105: bots' Rejuvenation on the bird spirits failed 407x with SPELL_FAILED_BAD_TARGETS).
-- Cause: Unit::_IsValidAssistTarget (Unit.cpp:12171-12179, PvC case) lets a player (UNIT_FLAG_PVP_ATTACKABLE set) assist a creature only if its
-- type_flags has CREATURE_TYPEFLAGS_PARTY_MEMBER (0x04000000) or CREATURE_TYPEFLAGS_AID_PLAYERS (0x1000). AC has no such rule, so AC's
-- type_flags 0 spirits are healable; the MoP rows have type_flags 0. Set AID_PLAYERS so the spirits are valid Rejuvenation targets.
-- Rollback: UPDATE `creature_template` SET `type_flags` = `type_flags` & ~0x1000 WHERE `entry` IN (23134, 23135, 23136);
UPDATE `creature_template` SET `type_flags` = `type_flags` | 0x1000 WHERE `entry` IN (23134, 23135, 23136);
