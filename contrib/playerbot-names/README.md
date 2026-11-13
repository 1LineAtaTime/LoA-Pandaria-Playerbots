# Curated playerbot naming and roster operations

This feature is opt-in. The existing realm is the only environment required.
The20,000-row CSV in modules/mod_playerbots/data/names is canonical. Native
selection validates candidates, draws weighted names without replacement,
uses optional race/class/gender hints and stops clearly on global exhaustion.
It never falls back to conlang or random letters while enabled.

## Deploy on an existing realm

1. Stop worldserver gracefully; keep one auth/world pair per realm.
2. From the repository root, use the existing MariaDB client and configuration:

```powershell
python contrib/playerbot-names/manage_roster.py backup --apply
python contrib/playerbot-names/manage_roster.py import --apply
python contrib/playerbot-names/manage_roster.py plan
python contrib/playerbot-names/manage_roster.py rename --apply
```

All commands accept --config, --client, --out and --prefix. Default schema prefix
is mop_. The offline manager supports Windows and Linux process/lock/dump paths;
the protocol/capacity probes currently use the existing Windows launcher tools.
Use a new --out for an independent deployment; keep its backup and durable plan.
No writes happen in backup/import/rename/rollback modes without --apply.

The import creates the character-side naming tables, imports candidate metadata,
and records existing bot accounts only when the bot-type registry, native RNDBOT
numbered prefix and zero privileged account access corroborate ownership.
A prefix by itself is insufficient. Other accounts/characters are excluded.
New native accounts record their own provisioning provenance.

Alternative clean SQL: apply
sql/updates/characters/2026_11_12_00_characters_curated_playerbot_names.sql,
then modules/mod_playerbots/data/sql/characters/curated_name_candidates.sql.
The generated SQL contains only public candidate metadata and no assignments,
accounts or credentials. On an existing roster, run the import manager as well
to establish ownership provenance before enabling the feature. A fresh realm
needs no preexisting ownership records: the native provisioner creates them.

## Configuration

AiPlayerbot.CuratedNames.Enable = 1
AiPlayerbot.RandomBotRosterTarget = 5000
AiPlayerbot.RandomBotAccountCount = 455
AiPlayerbot.MinRandomBots = 550
AiPlayerbot.MaxRandomBots = 550
AiPlayerbot.Provision.BatchSize = 100
AiPlayerbot.Provision.BatchPauseMs = 100
AiPlayerbot.Provision.MaxNewPerStartup = 0
AiPlayerbot.OnlineTargetOverride = 0

Keep DeleteRandomBotAccounts=0. RosterTarget is a stored-character target,
including existing bots, not an online target. Account sizing also accounts for
playable classes and the unchanged CharactersPerRealm cap. A final partial
account is allowed; no extra characters are created just to fill it.

MaxNewPerStartup optionally pauses after a bounded number of newly created
characters. Rebooting resumes missing classes and counts existing characters;
never delete/reset bots to resume. OnlineTargetOverride is an operational ramp
below the configured max; zero returns to normal min/max. Put the override in
worldserver.conf and use `reload config` to change it dynamically. Keep the
other settings in playerbots.conf; the existing configuration loader preserves
additional files on reload. This deployment uses50% account slack to avoid an
exactly-sized online-eligible pool, while still capping online bots at550.

## Verification and rollback

```powershell
python contrib/playerbot-names/manage_roster.py verify
python contrib/playerbot-names/Test-NativeNames.py
python contrib/playerbot-names/Measure-Capacity.py --target 550 --hold 90
# Offline, after reviewing conflicts:
python contrib/playerbot-names/manage_roster.py rollback --apply
```

The real native test uses one11-character matrix account plus two competing
creation clients; it verifies a single claimed name, relogin and full cleanup.
The capacity probe only observes the existing server; it starts no realm.
The manager's plan and GUID/account/old/new mapping remain private under out/.
Reruns reuse the plan and recognize previously assigned native names.

The tested name rollback changes only matching registered GUID/account pairs.
It refuses conflicting names instead of changing another player. Once retired
old names are reused, a full pre-change database/config/binary restore may be
needed to undo the entire rollout, including newly provisioned accounts/bots.
Do not restore databases while either server can write to them. The dump covers
auth, characters and playerbots. Any preexisting invalid views are saved as
separate definitions; none are deleted or repaired by the naming workflow.

Do not publish out/, private database dumps, old/new rename plans or working
configuration credentials. The project handoff carries clean schema/data,
source files, candidate CSV and documentation, not real account data.
