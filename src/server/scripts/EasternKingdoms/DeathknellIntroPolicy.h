#ifndef DEATHKNELL_INTRO_POLICY_H
#define DEATHKNELL_INTRO_POLICY_H
namespace DeathknellIntroPolicy
{
constexpr bool ShouldAwaken(bool undead, bool hasRigorMortis, bool introAccepted, bool introRewarded)
{
    return undead && hasRigorMortis && (introAccepted || introRewarded);
}
}
#endif
