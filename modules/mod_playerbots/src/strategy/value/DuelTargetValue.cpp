/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Value/DuelTargetValue.cpp) into
 * the Legends of Azeroth Pandaria fork (issue #149).
 *
 * AC's DuelInfo member is `Opponent`; this fork's TrinityCore-based DuelInfo (Player.h) uses the
 * lowercase `opponent`.
 */

#include "DuelTargetValue.h"

#include "Playerbots.h"

Unit* DuelTargetValue::Calculate() { return bot->duel ? bot->duel->opponent : nullptr; }
