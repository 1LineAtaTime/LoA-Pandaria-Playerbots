-- Durnholde: Zephyrus's Tornado 42895 does something (needs the C++ npc_durnholde_tornado).
-- Sourced: aura 80065 on 42895 (Wowhead npc_42895, Abilities tab), 80066 "Creates a Tornado under an enemy target" and its 5 s duration (DBC), "constant knockback" (Wowhead
-- 28643 comment, 2010). Ours: giving the aura and the summoner's level from C++. Howling Riftdweller 39844 also summons 42895: its tornado stays without the aura, but is now passive.
UPDATE `creature_template` SET `ScriptName`='npc_durnholde_tornado' WHERE `entry`=42895;
