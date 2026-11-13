# Runtime diagnostics

## Notification clarification — 2026-11-04

The user confirmed two ExampleCharacter logins no longer spam or duplicate reward mail.
The remaining notifications have distinct observed triggers:

- Walking around Dalaran: yellow literal "Created." together with a level-up-like sound.
- Using `.tel`: the level-up-like sound, without "Created." text.

Nearby bot creation/visibility is the user's hypothesis for the first trigger,
not an established cause. Keep this distinction in packet/script tracing and do
not repeat the already completed addon-disable or developer-mode tests. Startup
error/content repairs remain the user's stated priority.

The console screenshot's NullGameObjectAI entries are DEBUG messages from the
scripts logger. They describe the fallback AI used by ordinary game objects;
they are not null-pointer failures. Actual invalid content and SQL errors remain
separate problems to investigate.

The November 4 audit found Server.log overwritten per boot but DBErrors.log
appending across boots (about 121 MB). Its cumulative counts must not be reported
as current-run failures. For example, the historical SoundType error is frequent,
but the current 18,479 creature_text rows all have valid SoundType 0.

`contrib/diagnostics/server_log_report.py --logs exports/logs` writes a grouped
plaintext report under logs/diagnostics. Add `--watch-seconds 60` to capture only
new data from Server.log and DBErrors.log during that interval, starting at EOF.
Reports contain private runtime data and stay in ignored runtime directories.

Use `--source all --scope current` for the complete current-session audit. It
combines Errors.txt (all ERROR/FATAL categories, including SQL) with WARN records
from Server.log, without double-counting core errors present in both files.
Server.log alone is insufficient because SQL errors have a separate destination.
Missing-script and creature names are grouped by error class while preserving
a concrete example. This affects reports only, never the raw diagnostics.

`contrib/windows/Configure-DiagnosticLogs.ps1 -ConfigPath exports/worldserver.conf`
backs up the existing configuration and prepares:

- Timestamp, severity and category prefixes in file logs.
- Separate Errors.txt collecting ERROR/FATAL from every configured active logger.
- Previous-session backups and rotation at approximately 50 MiB per file.
- Console shows WARN, ERROR and FATAL; routine INFO remains in Server.log.
- Root/server/scripts use INFO, eliminating trace/debug chatter. Existing stricter
  logger thresholds and database-error file destinations remain intact.
- Database settings and unrelated paths are preserved.

Changes take effect on the next restart or configuration reload. Existing logs
are not removed. Rotation preserves backups; retention cleanup is not automated.

The quiet profile was activated on November 4 by a normal console shutdown and
restart. Both Windows launchers now use `start` to open the server executable in
its own visible console and immediately exit the batch wrapper. Do not leave a
terminated old batch waiting at a Y/N prompt. The bot crowding shown at Stormwind's
bank and outdoor noticeboard is tracked separately; it is not marked resolved by
the existing city NPC target limit or these logging changes.

Startup error priorities are invalid cross-map pool membership, missing/invalid
loot references, incompatible achievement criteria and SmartAI/condition data.
Validate scoped content fixes against the matching client and database. Do not
bulk-delete content, remove diagnostics or mark the realm error-free to reduce counts.

## Other work status

User-validated: BattlePay balance, affordable purchase, insufficient funds, quote
cancellation; GM commands. Implemented and built, awaiting acceptance: companion
gold-only loot/automatic pass, silent greetings, initialization/status refresh,
city service visits and repeated auction actor activity. The latest aura crash
repair has started successfully; no subsequent dump was present during this audit,
but sustained summon/combat stability remains open. Party buff/healing coverage,
teleport persistence, flight catch-up, general quest-chat reduction, remaining
scripted flyers, complete dungeon/BG/duel validation and Linux packaging are pending.

## Verified current startup repair — 2026-11-04

A clean diagnostic boot still showed 16,466 invalid-SoundType messages despite valid stored values. The prepared loader used GetUInt32 on the one-byte TINYINT field, reading unrelated bytes. It now uses GetUInt8 and reports the actual SoundType value. The prepared query now reads the legacy entry/groupid/id key instead of the compatibility CreatureID field. The 33 sourced rows from the earlier text repair were reapplied with the correct primary key using INSERT IGNORE, after a temporary-table test confirmed 33 rows and unchanged count on a repeat run. Existing affected rows were backed up locally; referenced broadcast IDs were verified present. Existing text was preserved.

Visual Studio built revision 7b050e73 successfully; 9/9 CTest checks passed. The executable/PDB hashes match the staged exports pair. The visible local worldserver restarted gracefully and initialized in 19 seconds. Its new startup has zero incorrect-SoundType messages and zero missing-text messages for Orland. Cross-map pools (3,857 gameobject lines and 1,021 child-pool lines), empty pools (789), loot references and content conditions/scripts remain real open issues. This is not an error-free realm. The current report and raw Errors.txt stay under ignored exports/logs. The current creature count was not modified.

Logging retention and dedicated Errors.txt are active. Routine scripts DEBUG traces remain in Server.log and no longer flood the console. Root/server verbosity and ERROR/FATAL diagnostics are preserved.

## Priority update and population snapshot — 2026-11-04

The live config target is 190–210 random bots (user-selected approximately 200), with 80 random-bot accounts. The read-only database snapshot showed 208 rndbot characters online; five were recorded in Dalaran, three at exactly the same saved coordinates. Saved positions can lag live movement. Do not raise the total before investigating distribution, camp/teleport arrival clustering, activity budgets and player proximity. The relocation maps remain 0,1,530,571, omitting Pandaria 870. No bot-count or relocation-map setting was changed in this pass.

Startup errors remain the highest content priority. Current grouped counts include 3,857 cross-map gameobject pool messages, 1,021 child-pool map conflicts, 789 empty pools, 326 grouped-loot entries, 133 missing loot references, 314 unsupported criteria-data entries and 122 missing SmartAI talk groups. A read-only check confirmed pool 13135 really mixes spawns from maps 0,1,530. These need scoped, tested content/schema compatibility repairs; do not simply disable validation or delete global pools. The yellow Created text and teleport sound remain unresolved with addons/dev-mode checks already exhausted.

Auth logging now includes two concise lifecycle INFO messages through a separate AuthStatus console appender: loading and ready after a successful socket bind/realm load. Routine auth console output remains WARN and above; SQL tracing stays disabled. The new profile was activated with the native visible launcher.
