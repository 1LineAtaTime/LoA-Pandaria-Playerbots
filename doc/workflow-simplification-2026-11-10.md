# Workflow simplification — November 10, 2026

User authorized one test environment and removal of obsolete Git branches.

## Current entry points

- Start-Server.cmd: one authserver and one worldserver, visible and reused.
- Run-QA.cmd: reviewed protocol quest contracts on that same pair, one worker at a time, saved report, exit.

Neither launches a game client, observer, live viewer or autonomous roaming loop.
Duplicate/foreign server processes block startup. Existing databases, test history
and characters are retained. The existing owned QA realm is the sole runtime for
these launchers; other database sets and exports files remain untouched/offline.
Startup roles and native DLLs are unchanged. No new native executable was staged.

The observer source experiment is archived; its source files/launchers and owned
client addon were removed from active use. Client settings and other addons were
preserved. The un-deployed native observe/rpginspect prototype was removed from
active C++; native source matches the verified8e4b42d17c QA build again.
Do not stage the build-directory executable containing that retired prototype.

Autonomous questing remains quarantined behind an explicit developer opt-in
because it used stuck teleports and circled in invalid geometry. This cleanup
is not a fix or certification of that movement. The default QA uses independent
reviewed lifecycle contracts. Existing pool reset protections still exclude
persistent adventure characters.

## Executed validation

- Cold Run-QA start: auth26236/world48404, one pair, no WoW client/viewers.
- Forced Orc Warrior25152 case: PASS, exact rewards, duplicate rejection and reconnect; out/qa-reports/20261110T190001422956Z.
- Forced Undead Rogue24959 case: PASS, including authored2copper reward at1x money; out/qa-reports/20261110T190250667625Z.
- Running Start-Server again reused the exact26236/48404 processes.
-81 Python checks pass:4campaign,8pool,3journal/lease,7supervision,26existingpipeline,33headless protocol.
- Native source guard now includes modules. Reviewed contract money expectations
  were aligned to the explicitly checked1x profile (base rewards0/2/15/2/50),
  after comparing the authored amounts to quest data and GetRewOrReqMoney's rate
  rule. No server rates were altered to force a pass.

These are two bounded cases, not whole-quest/race/class/instance certification.
Run-QA's default is all reviewed contracts expanded across eligible classes;
uncertified content remains debt.

## Git cleanup

Main is the only active branch after the cleanup. Three divergent historical
branch tips and the discontinued observer state have annotated archive tags.
Other deleted branch names were already reachable from the active history.
See branch-cleanup-2026-11-10.md for exact original SHA mappings. Both secondary
SkyFire worktree directories are retained, detached at their original commits.
No force push, source-history rewrite or database deletion is involved.
