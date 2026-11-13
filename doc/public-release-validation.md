# Public contribution validation

The reconciled source merges upstream beta2f9aebde9f with the clean cumulative
contribution. Its native source is the9a24e3e579 merge. Later packaging commits
change documentation/templates, not the tested native implementation.

- Full Visual Studio2022 x64 RelWithDebInfo build: worldserver and authserver passed.
- Native CTest:24 passed,0 failed, including four daemon command-line checks.
- Python QA unit tests:37 passed; diagnostic tests:3 passed.
- Contribution migration checksums:146 verified; complete SQL inventory:1287 files.
- Public-tree credential/runtime-file scan: no findings after removing the old
  connection-patcher EXE and sanitizing legacy updater configuration defaults.
- Real-name/private-address checks: no matches in public files or candidate
  commit metadata. Private development history is not an ancestor of this branch.
- Original source copyright, GPL notices and imported-source attribution remain.

Compiler warning codes and daemon artifact hashes are in the paired JSON report.
Windows SDK resource-version macro redefinition warnings occurred; build success
is not a claim of zero compiler warnings. No runtime logger was disabled.
The compiled programs/symbols and private raw logs remain local, not in Git.

Earlier native protocol/feature results are documented in the paired feature
notes, including550-bot runtime, BattlePay flows, companion policies, Yak
services, Orb transactions, Dalaran errands and auction seller-cache coverage.
Those results do not substitute for a long gameplay soak of the final upstream
reconciliation. No fresh-database universal migration replay, Linux gameplay,
all-quest/all-spell coverage or rendered Auctioneer timing pass is claimed.
See CONTRIBUTION.md, UPGRADE.md and the detailed handoff for remaining work.
