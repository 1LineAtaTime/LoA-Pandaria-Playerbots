-- Idriana4155 is already a banker, but her deposit-box gossip has type0 and
-- no required NPC flag, so it cannot dispatch SendShowBank. Restore BANKER(9)
-- in both canonical and compatibility columns. Preserve text and other menus.
UPDATE gossip_menu_option
SET OptionType=9,OptionNpcFlag=131072,option_id=9,npc_option_npcflag=131072
WHERE MenuID=2404 AND OptionID=0 AND menu_id=2404 AND id=0
 AND OptionBroadcastTextID=3398;
