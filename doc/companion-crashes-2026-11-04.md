# Companion testing crashes — November 4

Preserved the deployed df5768fc executable and matching symbols under ignored
out/crash-evidence-20261104-df5768fc before staging another build.

The local 12:15:17 and 13:30:43 reports both fault in Unit::SetPower at line
13587, called by ModifyPower, Aura::Update, Unit::_UpdateSpells and Player::Update
on a map worker. SetPower iterated the live proc-on-power aura list while invoking
triggered casts. Casts can remove the current/next effects, invalidating traversal.
The repair snapshots spell/effect/caster identities, resolves still-present effects
before use, and skips removed or replaced effects. This is a scoped source repair;
the dumps do not prove there are no other concurrent mutation defects. Repeated
summon/combat/power-regeneration testing and sustained runtime remain required.

The production 12:07:54 report is separate: MySQLConnection::_HandleMySQLErrno
aborted while a broad reload queried achievement reward locales. It is not the
same aura crash. Investigate the preceding database error separately; do not
disable database failure handling or run reload-all as a bot troubleshooting step.

## Command permissions

The connected mop_auth lacked permission 1346 and its role-199 link. Both are now
installed using the idempotent 2026_11_04_00_companion_command_permission.sql.
World::ReloadRBAC previously invalidated session sets without reloading the cached
definition graph. It now reloads AccountMgr's definitions too. A restricted exact
command also no longer falls through to matching longer command abbreviations.

## Reference executable

Inspected <local-workspace-path>
SHA256: 12175e4c1cc8ccb865f1027f7518502200d5ea8a265f6a0a8a63184edc6af336.
Command table at 0x1402245d0 registers a `bots` tree, including group role addition
at 0x140225b00 and PvP addition at 0x140225fa0. The original `.bot` is an abbreviation.
The new server's exact `.bot` adapter keeps access checks and avoids abbreviation
collisions. Binary configuration strings include distinct CityAmount, DuelingAmount,
CopyMount and login/idle delays. These reveal design features, not proof of their
correctness. Analysis files remain ignored under out/; no license checks were changed.

## Requested behavior after stability validation

- Currency-only corpse loot for companions; automatically pass all item rolls.
- Suppress greetings, quest chatter and routine whispers; rare useful notices in party chat.
- Party-wide eligible buff/healing support and appropriate hostile-target debuffs.
- Maintain companions across instances/teleports and keep up during mounted flight.
- Expose applicable controls in playerbots.conf, applying restrictions to companions
  without disabling autonomous bots' own world progression.

These behavior changes remain pending while crash validation takes priority.

## Staging and local startup

Visual Studio x64 RelWithDebInfo build succeeded for gameplay revision 0ed6ad64. CTest passed 9/9, including the production command-matching helper regression. worldserver.exe and worldserver.pdb staged in exports have SHA-256 hashes matching the build output. The permission migration is also packaged under exports/sql/auth. Authserver remained running; worldserver was opened through the visible launcher, initialized in 23 seconds and was verified listening on port 8085. Repeated summon/combat validation and a production soak remain pending. No claim of complete crash resolution is made.
