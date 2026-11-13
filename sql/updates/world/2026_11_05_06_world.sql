-- Hillsbrad Foothills: the turn-in of Heroes of the Horde! (28400) at High Warlord Cromush as both TDBs have it (Cataclysm
-- Classic 442 and retail 1210, VerifiedBuild 32305): emotes 66 (salute), 1, 1, and the text's opening "<High Warlord Cromush
-- salutes.>$B$B", which mop's copy had lost (it began with a space). The Road to Purgation (28375) loses a trailing space.
UPDATE `quest_offer_reward` SET `Emote1` = 66, `Emote2` = 1, `Emote3` = 1, `Emote4` = 0, `EmoteDelay1` = 0, `EmoteDelay2` = 0, `EmoteDelay3` = 0, `EmoteDelay4` = 0,
`RewardText` = '<High Warlord Cromush salutes.>$B$BHe will be honored as a hero of the Horde, just as he always hoped. What you and Orkus did will save hundreds of lives and ensure that our foothold in Hillsbrad is not lost.' WHERE `ID` = 28400;
UPDATE `quest_offer_reward` SET `RewardText` = 'This place is dark... foreboding. It smells of piss and fur. There are definitely worgen here and very likely dwarves.' WHERE `ID` = 28375;
