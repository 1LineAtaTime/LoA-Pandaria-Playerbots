/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Shadowfang Keep (epic #373, issue #668). See CataSfkShared.h for sources and ids.

#ifndef PLAYERBOTS_CATASFKACTIONCONTEXT_H
#define PLAYERBOTS_CATASFKACTIONCONTEXT_H

#include "Action.h"
#include "CataSfkActions.h"
#include "NamedObjectContext.h"

class CataDungeonSfkActionContext : public NamedObjectContext<Action>
{
public:
    CataDungeonSfkActionContext()
    {
        creators["sfk interrupt pain and suffering"] = &CataDungeonSfkActionContext::sfk_interrupt_pain_and_suffering;
        creators["sfk kill worgen spirit"] = &CataDungeonSfkActionContext::sfk_kill_worgen_spirit;
        creators["sfk interrupt worgen"] = &CataDungeonSfkActionContext::sfk_interrupt_worgen;
        creators["sfk leave desecration"] = &CataDungeonSfkActionContext::sfk_leave_desecration;
        creators["sfk interrupt empowerment"] = &CataDungeonSfkActionContext::sfk_interrupt_empowerment;
        creators["sfk leave shield cone"] = &CataDungeonSfkActionContext::sfk_leave_shield_cone;
        creators["sfk leave poison mixture"] = &CataDungeonSfkActionContext::sfk_leave_poison_mixture;
        creators["sfk keep moving"] = &CataDungeonSfkActionContext::sfk_keep_moving;
        creators["sfk coagulant jump"] = &CataDungeonSfkActionContext::sfk_coagulant_jump;
        creators["sfk interrupt cursed bullets"] = &CataDungeonSfkActionContext::sfk_interrupt_cursed_bullets;
        creators["sfk leave barrage cone"] = &CataDungeonSfkActionContext::sfk_leave_barrage_cone;
        creators["sfk kill ghoul"] = &CataDungeonSfkActionContext::sfk_kill_ghoul;
    }

private:
    static Action* sfk_interrupt_pain_and_suffering(PlayerbotAI* botAI) { return new SfkInterruptPainAndSufferingAction(botAI); }
    static Action* sfk_kill_worgen_spirit(PlayerbotAI* botAI) { return new SfkKillWorgenSpiritAction(botAI); }
    static Action* sfk_interrupt_worgen(PlayerbotAI* botAI) { return new SfkInterruptWorgenAction(botAI); }
    static Action* sfk_leave_desecration(PlayerbotAI* botAI) { return new SfkLeaveDesecrationAction(botAI); }
    static Action* sfk_interrupt_empowerment(PlayerbotAI* botAI) { return new SfkInterruptEmpowermentAction(botAI); }
    static Action* sfk_leave_shield_cone(PlayerbotAI* botAI) { return new SfkLeaveShieldConeAction(botAI); }
    static Action* sfk_leave_poison_mixture(PlayerbotAI* botAI) { return new SfkLeavePoisonMixtureAction(botAI); }
    static Action* sfk_keep_moving(PlayerbotAI* botAI) { return new SfkKeepMovingAction(botAI); }
    static Action* sfk_coagulant_jump(PlayerbotAI* botAI) { return new SfkCoagulantJumpAction(botAI); }
    static Action* sfk_interrupt_cursed_bullets(PlayerbotAI* botAI) { return new SfkInterruptCursedBulletsAction(botAI); }
    static Action* sfk_leave_barrage_cone(PlayerbotAI* botAI) { return new SfkLeaveBarrageConeAction(botAI); }
    static Action* sfk_kill_ghoul(PlayerbotAI* botAI) { return new SfkKillGhoulAction(botAI); }
};

#endif
