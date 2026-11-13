# Rut’theran departure, bot lifetime and emote repair

## Diagnosis

After Endgame26706, the intended departure is Rut’theran Village14434 from
Admiral Nightwind36616. Its reward casts78107. The local18414 SpellEffect.dbc
contains bind(effect0) and teleport(effect1), both requesting TARGET_DEST_DB.
Neither destination row existed, so the spell fell back to the caster’s current
location. The quest could be rewarded while leaving the player in Gilneas.
Nightwind had a questgiver relation but no AI for his old ready-to-sail option.
This explains a completed departure with an empty quest log. The earlier Lorna
turn-in failure was separately measured as stale movement ownership, with the
server still seeing the rider48 yards away; a phase mismatch was not observed.

Source for the published destination:
https://github.com/TrinityCore/TrinityCore/issues/22248 . The coordinates are
adapted to this core’s column names and both18414 effect indices; the source’s
newer-schema SQL is not imported wholesale. Quest progression references:
https://warcraft.wiki.gg/wiki/Rut%27theran_Village_%28quest%29 .

## Repairs

SQL05 restores78107’s bind/teleport destination to map1,8343.86/1165.28/4.40044,
requires rewarded26706 before14434, and binds Nightwind’s departure script.
Uncompleted14434 remains a normal quest with its normal reward spell. Players
who already rewarded both quests receive a separate ready-to-sail gossip retry;
it casts the same spell without awarding money, XP or quests again. Sender,
action, race, reward state, alive/combat and vehicle checks guard the selection.
The following chain is The Howling Oak28517 from Krennan Aranas42968 at the docks.

The ten addon emote diagnostics were genuine data errors: oneshot animations
cannot be persistent addon states. Attack/ready poses now use persistent333;
spell-precast uses193. Beg/rude lack matching persistent forms and are removed
from the addon so normal scripted/combat animation owns the pose. The emote
validator and warning/error levels remain enabled. SQL05 also restores the
missing LINK type on Shadowsworn Cultist6004’s melee-weapon action9, and removes
an inappropriate normal-dungeon-only flag from Skeletal Servant8477’s despawn.

## Overnight crash

The02:24 crash in source470f9a4b occurred at PlayerbotAI::UpdateAI line1166 on a
map worker. Disassembly confirms an invalid MoveSpline pointer read, not a null
pointer: the address register was8000013200000000. Adding a null guard would
not repair it. The small dump does not include the Player/AI heap, so the exact
corruption origin and character GUID cannot be proven from that dump.

A concrete lifetime defect was found in the surrounding subsystem:
DisablePlayerBot erased the holder entry and deleted the AI but retained the
same pointer in the global AI registry. Later lookups could return freed memory.
It now unregisters before deletion. Lookup additionally requires the AI’s bound
Player pointer to match the requesting Player, preventing reuse across sessions
with the same character GUID. This repairs a real use-after-free path; it is not
proof that all memory-corruption causes or all bot-thread races are eliminated.

## Deployment and rollback

Visual Studio RelWithDebInfo/x64 build succeeded. Scoped pre-change static rows,
a runnable SQL rollback and the prior matching EXE/PDB are private under
out/backups/departure-emotes-ai-20261112. No account or password changes were
made. At the user’s explicit request, only the affected character’s14434 reward
record was backed up and removed while offline;26706 remains rewarded. This
individual testing reset is not part of the distributable migration.

Regressions: contrib/headless-client/nightwind_departure_regression.py and
contrib/headless-client/companion_lifetime_regression.py. Results will be recorded
below after running against the rebuilt local server. They use disposable owned
identities; prerequisite/level fixtures do not certify a natural full starter run.

## Authentication crash found by regression

A fresh disposable-account login exposed a second, independently confirmed crash
in authserver: HexStrToByteArray asserted after SetVSFields generated a verifier
with high zero bytes. BigNumber::AsHexStr produces a variable-width number while
SRP requires fixed32-byte values. This was reproduced by the native dump at
11:25:52 and repaired by serializing fixed32-byte little-endian arrays to the
existing big-endian hex database notation. Valid short legacy numeric values
are left-padded before conversion, preserving the same verifier and salt.
Malformed SRP fields can be rebuilt from a valid legacy SHA1 hash; when neither
representation is valid, only that login is rejected. No passwords are reset.

The rebuilt authserver passed auth_srp_regression.py: deliberately short salt
and verifier authenticate, fresh generation persists64-character fields,
non-hex SRP data can be rederived with a valid hash, and an unrecoverable account
is rejected while authserver remains available. The negative case intentionally
emits one error for the owned corrupt test account; the fixture is then removed.
Private report: out/auth-srp-regression.json.

The first departure regression passed normal quest acceptance/reward, map1
arrival, hearth binding, the already-rewarded retry and28517 acceptance. A follow-up
run also requires a normal questgiver-hello dialogue response at Krennan.

## Companion lifecycle hang and time-sync fix

Five ordinary companion add/remove cycles passed. A subsequent login/logout of
the same character as a normal player hung the world thread in
std::deque<unsigned int>::_Tidy from Player::~Player, WorldSession::LogoutPlayer.
All four map workers were idle. A read-only live thread/context/stack capture is
private in out/live-world-lifetime-hang.txt. Normal shutdown could not run, so
only the hung worldserver was force-terminated after the capture.

The only unsigned-int queue on Player is m_timeSyncQueue. HandleTimeSyncResp
unconditionally called front()/pop() even with no pending request, and consumed
a pending request even when its counter did not match. Empty pop is undefined
behavior and can underflow the queue; its destructor then spins through an
invalid element count. The handler now ignores unsolicited/duplicate responses
and preserves pending requests when a counter mismatches. This also prevents
invalid responses from contaminating movement clock deltas. The companion
regression now explicitly replays four consumed/unsolicited responses before
normal logout. This bug is independent of the bot AI unregister fix.

## Final deployed regression results

The final time-sync build passed five companion add/remove cycles, normal login
of the former companion, four unsolicited/duplicate time-sync responses, and
acknowledged logout. The world thread remained responsive and native cleanup
removed both characters and their account. The interrupted pre-fix fixture was
also removed after restart. Private report: out/companion-lifetime-regression.json.
The final auth build reran and passed all five SRP checks with its fixture removed.
Credential derivation now uses stack-owned byte arrays rather than allocating
BigNumber byte/hex buffers and leaking them.

The original overnight invalid-spline read remains a monitoring item: repairing
the demonstrated stale-AI and time-sync lifetime defects does not establish which
one caused that older heap pointer corruption. Other legacy world-data errors
remain tracked; neither the error logger nor the validators were suppressed.

The final departure rerun passed as well, including an actual questgiver-hello
response from Krennan before normal28517 acceptance. Both the test account and
character were removed; no live-player quest was completed by these runners.
