#include "CastTimeStrategy.h"

#include "GenericSpellActions.h"
#include "Playerbots.h"
#include "SpellDefines.h"

float CastTimeMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    if (!action->GetTarget() || action->GetTarget() != AI_VALUE(Unit*, "current target"))
        return 1.0f;

    CastSpellAction* spellAction = dynamic_cast<CastSpellAction*>(action);
    if (!spellAction)
        return 1.0f;

    uint32 spellId = AI_VALUE2(uint32, "spell id", spellAction->getSpell());
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return 1.0f;

    // 5.4.8: MoP's SpellInfo has no "Targets" bitfield left over from WotLK; the explicit target
    // mask is the equivalent (SpellDefines.h TARGET_FLAG_SOURCE_LOCATION/TARGET_FLAG_DEST_LOCATION).
    uint32 targetMask = spellInfo->GetExplicitTargetMask();
    if ((targetMask & TARGET_FLAG_DEST_LOCATION) != 0 || (targetMask & TARGET_FLAG_SOURCE_LOCATION) != 0)
        return 1.0f;

    uint32 castTime = spellInfo->CalcCastTime(bot->GetLevel());

    if (spellInfo->IsChanneled())
    {
        int32 duration = spellInfo->GetDuration();
        duration = std::min(duration, 3000);
        if (duration > 0)
            castTime += duration;
    }

    Unit* target = action->GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
        return 1.0f;

    float groupDps = AI_VALUE(float, "estimated group dps");
    if (groupDps > 0.0f && castTime > (1000 * target->GetHealth() / groupDps))
        return 0.1f;

    return 1.0f;
}

void CastTimeStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new CastTimeMultiplier(botAI));
}
