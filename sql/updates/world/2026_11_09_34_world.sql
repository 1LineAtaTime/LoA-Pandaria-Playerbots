-- Huntsman Markhor (14741): he can sell his pet supplies again.
-- Sourced: TDB 4.4.2 npcflag 4194435 (gossip, quest giver, vendor, stable master). mop had 4194307, without the vendor flag, so the
-- three items of his npc_vendor list (mop's own: Rope Pet Leash, Brazie's Sunflower Seeds, Horde Balloon) could not be bought.
UPDATE `creature_template` SET `npcflag`=4194435 WHERE `entry`=14741;
