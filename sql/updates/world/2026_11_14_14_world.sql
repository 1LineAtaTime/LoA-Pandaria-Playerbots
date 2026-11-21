-- Eastern Plaguelands, Fiona's Caravan: Beezil Linkspanner joins when asked, and the two Full Caravan criteria that need a
-- script are credited.
--
--  * Before: Beezil's offer (menu 12081 option 0, added by 2026_11_14_05) led to his "Sure, I'll join you" but nothing marked him
--    as joined, so Beezil's Cog 27562 (gated on his flag quest 27563) could never be taken and his greeting never changed;
--    nothing credited Fiona's or Beezil's criterion of Full Caravan 5442.
--  * Now npc_epl_beezil rewards 27563 when the option is chosen and credits his criterion (be the target of 85811); Fiona
--    (npc_epl_fiona, 2026_11_14_12) credits hers (94998) when Onward 27373 is handed in to her.
--  * Sourced: the option and reply (both TDBs), the flag quest, the criteria (Achievement_Criteria.dbc: types 28 for 94998 and
--    85811, both spells serverside). Ours: when Fiona's is credited (no source says), that the option is what sets the flag.

UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_epl_beezil' WHERE `entry`=46022;
