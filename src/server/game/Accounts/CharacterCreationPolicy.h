#ifndef CHARACTER_CREATION_POLICY_H
#define CHARACTER_CREATION_POLICY_H
namespace CharacterCreationPolicy
{
constexpr bool AllowsBothFactions(bool configured, bool pvpRealm, bool hasBypass)
{
    return configured || !pvpRealm || hasBypass;
}
}
#endif
