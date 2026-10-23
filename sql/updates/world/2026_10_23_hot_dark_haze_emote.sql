-- #696 Hour of Twilight, Path of the Titans: the Dark Haze 54628 wave emote (npc_frozen_servitor_stalker ACTION_SPAWN, Talk group 0)
-- carried BroadcastTextId 66157 ("Lilian's Soul fades into the spirit realm! It can no longer be harmed!", a Scholomance line), so
-- the client showed that text on every faceless wave (w106 headless trail: frm=Dark Haze msg="Lilian's Soul fades ...").
-- broadcast_text 53085 is the row's own text "Spawn of the Old Gods materialize nearby!" (Project-Neltharion-4.3.4
-- boss_archbishop_benedictus.cpp TALK_THRALL_3_WARNING_GROUP = -53085). The stage driver counts these emotes as the wave count.
-- Undo: UPDATE `creature_text` SET `BroadcastTextId` = 66157 WHERE `CreatureID` = 54628 AND `GroupID` = 0 AND `ID` = 0;
UPDATE `creature_text` SET `BroadcastTextId` = 53085 WHERE `CreatureID` = 54628 AND `GroupID` = 0 AND `ID` = 0;
