-- Gilneas, quest 24920 "Slowing the Inevitable": the parked Captured Riding Bat (38615) starts the ride only through its
-- spellclick (72472, which summons the rider's own bat 38540). An earlier update in this series cleared its AIName, but a
-- later sweep gives AIName 'SmartAI' to every creature that still has smart_scripts rows, and 38615 kept its old rows
-- (on spellclick, cross-cast Ride Vehicle Hardcoded onto the nearest 38540 within 5 yd), so both paths ran. Its npcflag
-- (spellclick) is already in creature_template, so the old SmartAI rows are not needed.
DELETE FROM `smart_scripts` WHERE `entryorguid` = 38615 AND `source_type` = 0;
UPDATE `creature_template` SET `AIName` = '' WHERE `entry` = 38615 AND `AIName` = 'SmartAI';
