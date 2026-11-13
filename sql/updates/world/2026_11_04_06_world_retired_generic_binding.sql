-- AddSC_generic_creature deliberately does not register generic_creature.
-- CreatureAISelector already falls through this missing script to AIName and
-- normal NPC/permit selection. Remove only that retired binding; do not enable
-- the old generic script over thousands of existing SmartAI/special NPCs.
UPDATE `creature_template` SET `ScriptName`='' WHERE `ScriptName`='generic_creature';
