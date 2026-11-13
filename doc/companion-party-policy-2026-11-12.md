# Quiet companions and human-led dungeon parties — November 12

## Behavior

Routine TellMaster/TellMasterNoFacing narration is opt-in, including quest
acceptance/completion and lifecycle greetings. Immediate replies while executing
an explicit bot chat command and addon-language protocol responses remain
available; TellError retains its existing error path. The quest-accept distance
sound is also opt-in. Direct diagnostic RPG/status commands remain explicit
requests. This does not change ordinary human chat or hide server errors.

The central gate covers greeting/quest call sites that bypassed the old limited
SilentGreetings checks. Both worldserver.conf and playerbots.conf now contain
the settings, because these native policy reads use the main config manager.

New LFG groups prefer a connected human leader over a bot. A leader can remove
bots immediately from an otherwise bot-only dungeon group, including stale
offline members. In mixed groups the offline-bot recovery path still requires
the human leader, a positively identified bot and a stable dungeon state.
Humans are never classified as bots based on their name or level. Online bot
sessions identify themselves; offline identities require curated provisioning
provenance. The original native group-removal path updates membership/LFG state.

In mixed groups, pending bot votes copy the real leader's explicit yes/no vote;
they never cast a vote for another real player or the intended victim. The
initiator's already-recorded yes is evaluated immediately, and passed-vote
checks accept a majority above the required count as well as exactly equal.
Outsider and repeat/changed answers are rejected.

Companions in a group with a human retain coin-only loot policy. Existing roll
creation automatically records their passes. A late AI/client need/greed reply
cannot overwrite that pass or count a vote twice; group-removal's internal roll
sentinel is preserved. No item rewards are deleted from players.

## Loot controls

The unmodified5.4.8 client disables its loot dropdown for Dungeon Finder groups.
The server still identifies the group as LFG so teleport, role and queue controls
keep working. Use `.partyloot free`, `.partyloot roundrobin`, `.partyloot group`,
`.partyloot need` or `.partyloot master` in a companion-only party. Only its
real-player leader can use this command; master loot designates that player.
It uses the existing playerbot-command permission and needs no new RBAC grant.
Normal loot-method requests are validated and cannot change mixed-human LFG loot.
The dropdown may remain grey; the command is the supported server-side control.

## Config and deployment

AiPlayerbot.Companion.RoutineMessages =0
AiPlayerbot.Companion.SilentGreetings =1
AiPlayerbot.Companion.GoldOnlyLoot =1
AiPlayerbot.Companion.FollowLeaderVotes =1
AiPlayerbot.Companion.RemoveOfflineBots =1
AiPlayerbot.Companion.LeaderLootControl =1

The RemoveOfflineBots switch also enables immediate bot-only-party removal.
Native/default/runtime configs and source overlay are updated. A worldserver
rebuild/restart is required; no account passwords or player quests are changed.
Rollback uses the previous EXE/PDB and config copies under the ignored backup
folders. The data-only Gate of the Setting Sun migrations are separate.

## Verification

The real-protocol companion_party_regression.py creates one disposable human
fixture, joins four companions, shares a quest, checks silence/loot control,
uses normal LFG join/proposal packets and tests leader bot/offline removal.
Owned fixture cleanup and exact result/limitations are recorded under ignored
out/companion-party-20261112/report.json. Final execution results follow.

### Completed runtime checks

The owned fixture passed four-companion invitation/quest-share silence, human
leadership, .partyloot change, real LFG join/proposal formation, retained human
leadership and both online-bot and offline-bot removal. Its account/character
and dependencies were cleaned. The status-reply follow-up accounts for queued
chat-trigger execution: explicit requests have a bounded reply window, while
hello/goodbye and quest accepted/completed notifications remain muted even in
that window. Mixed-human yes/no vote mirroring and actual loot-roll outcomes are
compiled/code-reviewed; this fixture tests bot-only removal and loot controls.

The follow-up used actual roster companions rather than a second GM-account
identity: four companions joined, the selected bot answered a deferred position
query, and quest sharing immediately afterward still produced no routine
whispers. See out/companion-reply-20261112/report.json for private evidence.
