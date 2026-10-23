/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// End Time bot strategy "cata-et" - trigger context (issue #671, epic #373). Pattern: AC Ai/Dungeon/HoS/HoSTriggerContext.h.

#ifndef PLAYERBOTS_CATAETTRIGGERCONTEXT_H
#define PLAYERBOTS_CATAETTRIGGERCONTEXT_H

#include "CataEtTriggers.h"
#include "NamedObjectContext.h"

class CataDungeonEndTimeTriggerContext : public NamedObjectContext<Trigger>
{
    public:
        CataDungeonEndTimeTriggerContext()
        {
            creators["et sylvanas safe spot"] = &CataDungeonEndTimeTriggerContext::sylvanas_safe_spot;
            creators["et sylvanas stay inside"] = &CataDungeonEndTimeTriggerContext::sylvanas_stay_inside;
            creators["et blighted arrows"] = &CataDungeonEndTimeTriggerContext::blighted_arrows;
            creators["et sylvanas kill ghoul"] = &CataDungeonEndTimeTriggerContext::sylvanas_kill_ghoul;
            creators["et ranged spread"] = &CataDungeonEndTimeTriggerContext::ranged_spread;
            creators["et moonlance"] = &CataDungeonEndTimeTriggerContext::moonlance;
            creators["et eye of elune"] = &CataDungeonEndTimeTriggerContext::eye_of_elune;
            creators["et tyrande interrupt"] = &CataDungeonEndTimeTriggerContext::tyrande_interrupt;
            creators["et moonlight pool"] = &CataDungeonEndTimeTriggerContext::moonlight_pool;
            creators["et tyrande ranged band"] = &CataDungeonEndTimeTriggerContext::tyrande_ranged_band;
            creators["et frost blades"] = &CataDungeonEndTimeTriggerContext::frost_blades;
            creators["et flarecore"] = &CataDungeonEndTimeTriggerContext::flarecore;
            creators["et jaina interrupt"] = &CataDungeonEndTimeTriggerContext::jaina_interrupt;
            creators["et jaina fragments"] = &CataDungeonEndTimeTriggerContext::jaina_fragments;
            creators["et baine in lava"] = &CataDungeonEndTimeTriggerContext::baine_in_lava;
            creators["et baine totem"] = &CataDungeonEndTimeTriggerContext::baine_totem;
            creators["et distortion bomb"] = &CataDungeonEndTimeTriggerContext::distortion_bomb;
            creators["et infinite breath"] = &CataDungeonEndTimeTriggerContext::infinite_breath;
            creators["et hourglass"] = &CataDungeonEndTimeTriggerContext::hourglass;
        }
    private:
        static Trigger* sylvanas_safe_spot(PlayerbotAI* ai)
        {
            return new CataEtConditionTrigger(ai, "et sylvanas safe spot", &EndTime::NeedsSylvanasSafeSpot);
        }
        static Trigger* sylvanas_stay_inside(PlayerbotAI* ai)
        {
            return new CataEtConditionTrigger(ai, "et sylvanas stay inside", &EndTime::NeedsSylvanasInside);
        }
        static Trigger* blighted_arrows(PlayerbotAI* ai)
        {
            return new CataEtHazardTrigger(ai, "et blighted arrows", EndTime::HazardKind::BlightedArrows);
        }
        static Trigger* sylvanas_kill_ghoul(PlayerbotAI* ai)
        {
            return new CataEtConditionTrigger(ai, "et sylvanas kill ghoul", &EndTime::NeedsGhoulKill);
        }
        static Trigger* ranged_spread(PlayerbotAI* ai)
        {
            return new CataEtConditionTrigger(ai, "et ranged spread", &EndTime::NeedsRangedSpread);
        }
        static Trigger* moonlance(PlayerbotAI* ai)
        {
            return new CataEtHazardTrigger(ai, "et moonlance", EndTime::HazardKind::Moonlance);
        }
        static Trigger* eye_of_elune(PlayerbotAI* ai)
        {
            return new CataEtHazardTrigger(ai, "et eye of elune", EndTime::HazardKind::EyeOfElune);
        }
        static Trigger* tyrande_interrupt(PlayerbotAI* ai)
        {
            return new CataEtConditionTrigger(ai, "et tyrande interrupt", &EndTime::TyrandeNeedsInterrupt);
        }
        static Trigger* moonlight_pool(PlayerbotAI* ai)
        {
            return new CataEtConditionTrigger(ai, "et moonlight pool", &EndTime::NeedsMoonlightPool);
        }
        static Trigger* tyrande_ranged_band(PlayerbotAI* ai)
        {
            return new CataEtConditionTrigger(ai, "et tyrande ranged band", &EndTime::NeedsTyrandeRangedBand);
        }
        static Trigger* frost_blades(PlayerbotAI* ai)
        {
            return new CataEtConditionTrigger(ai, "et frost blades", &EndTime::InBladePath);
        }
        static Trigger* flarecore(PlayerbotAI* ai)
        {
            return new CataEtConditionTrigger(ai, "et flarecore", &EndTime::NeedsFlarecoreAction);
        }
        static Trigger* jaina_interrupt(PlayerbotAI* ai)
        {
            return new CataEtConditionTrigger(ai, "et jaina interrupt", &EndTime::JainaNeedsInterrupt);
        }
        static Trigger* jaina_fragments(PlayerbotAI* ai)
        {
            return new CataEtConditionTrigger(ai, "et jaina fragments", &EndTime::NeedsFragments);
        }
        static Trigger* baine_in_lava(PlayerbotAI* ai)
        {
            return new CataEtConditionTrigger(ai, "et baine in lava", &EndTime::NeedsLavaEscape);
        }
        static Trigger* baine_totem(PlayerbotAI* ai)
        {
            return new CataEtConditionTrigger(ai, "et baine totem", &EndTime::NeedsTotemThrow);
        }
        static Trigger* distortion_bomb(PlayerbotAI* ai)
        {
            return new CataEtHazardTrigger(ai, "et distortion bomb", EndTime::HazardKind::DistortionBomb);
        }
        static Trigger* infinite_breath(PlayerbotAI* ai)
        {
            return new CataEtConditionTrigger(ai, "et infinite breath", &EndTime::InInfiniteBreath);
        }
        static Trigger* hourglass(PlayerbotAI* ai)
        {
            return new CataEtConditionTrigger(ai, "et hourglass", &EndTime::NeedsHourglass);
        }
};

#endif
