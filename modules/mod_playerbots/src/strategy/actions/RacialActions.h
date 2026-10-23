#ifndef _PLAYERBOT_RACIALACTIONS_H
#define _PLAYERBOT_RACIALACTIONS_H

#include "AiObject.h"
#include "GenericSpellActions.h"

class PlayerbotAI;

// #163 S8 (AC parity, Ai/Base/ActionContext.h; RacialsStrategy.cpp): one racial ability per class,
// resolved by name the same way every other spell action in this fork is - a Blood Elf paladin only
// ever knows "Arcane Torrent" spell 28730, a rogue only 25046, etc., so a single by-name action
// covers every id in DBC without hardcoding which race/class has which variant. Verified against the
// client Spell.dbc (5.4.8 build 18414): every AC racial name below still resolves to a real spell.
// Arcane Torrent / War Stomp / Berserking / Blood Fury / Gift of the Naaru are NOT declared here:
// they already exist in GenericSpellActions.h (with more specific base classes - e.g. War Stomp is
// CastMeleeSpellAction, Gift of the Naaru is CastHealingSpellAction) and are registered from there
// in ActionContext.h; a second BUFF_ACTION definition of the same class name was a build break
// (S8 build-fix pass, issue #194).
BUFF_ACTION(CastEveryManForHimselfAction, "every man for himself");
BUFF_ACTION(CastWillOfTheForsakenAction, "will of the forsaken");
BUFF_ACTION(CastStoneformAction, "stoneform");
BUFF_ACTION(CastEscapeArtistAction, "escape artist");
BUFF_ACTION(CastLifebloodAction, "lifeblood");

#endif
