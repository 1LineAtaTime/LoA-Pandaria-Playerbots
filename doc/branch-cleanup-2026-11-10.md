# Branch cleanup — November 10, 2026

User explicitly requested removing obsolete branches, superseding the earlier request to retain their names.
Main is the sole active branch. No commit history is rewritten. Unique historical work is preserved by annotated archive tags.

| Old branch | Original tip | Disposition / archive |
| --- | --- | --- |
| codex/autonomous-gameplay-qa | 90e14eea164a487d532c2752caff34d66acdc2bc | Merge into main, then delete redundant branch name |
| codex/city-companions | 00f9dd446f3b118806ee68969a586dae60c07a45 | Already preserved in active history; delete redundant branch name |
| codex/clean-sf-core | 6c9f625e89dc275c63677e07c8e097e4dfff92be | archive/2026-11-10/codex-clean-sf-core |
| codex/loa-fresh-restore | 0ae7b749408efca3ece20e928a721bd731380df6 | Already preserved in active history; delete redundant branch name |
| codex/loa-starter-repairs | 04ba874ecef7d954d937d2edc66d5656a6d5c28f | Already preserved in active history; delete redundant branch name |
| codex/pre-full-loa-beta-20261107 | 00312addff5b0066e29f0ee6e650481192634eca | Already preserved in active history; delete redundant branch name |
| codex/runtime-dependency-repair | aa03f2ce0386b668898b6d32322c6e4c9a01dde2 | archive/2026-11-10/codex-runtime-dependency-repair |
| codex/skyfire-integration | 836be5b749f39265691c8874d384e7aa78276ce4 | Already preserved in active history; delete redundant branch name |
| codex/skyfire-upstream-history | a69eda918b5bd416078fd8d832560d0fd4fb9023 | archive/2026-11-10/codex-skyfire-upstream-history |
| develop | 1a7ce5686e624f7b4b6e6d305afd809fd0edb083 | Already preserved in active history; delete redundant branch name |
| main | 04ba874ecef7d954d937d2edc66d5656a6d5c28f | Keep as only active branch; fast-forward after validation |
| playerbot | e135742e1891d3e653d27966da2dc70e793f203e | Already preserved in active history; delete redundant branch name |

The discontinued observer working state is retained at `archive/2026-11-10/observer-experiment`.
The clean-SF and SF-integration worktree directories are retained with detached HEADs at their original commits. No worktree files or databases are deleted.

Recover an archived line with `git switch -c recovery/<name> <archive-tag>`.
Forgejo remains authoritative. Archive tags are pushed before branch deletion; the GitHub mirror is checked afterward.


## Completion verification

Eleven obsolete branch names were removed from Forgejo, ten redundant local
branches were removed, and the corresponding eleven stale GitHub branch refs
were deleted after checking their tips against the reviewed plan. Native push
mirroring updated main/tags but did not prune those stale backup refs, so the
GitHub API deletion was limited to reproducing the authoritative source's branch
set. No commit was authored or rewritten on GitHub.

Only main remains locally and on both project repositories. Both default branches
are main. The archive tags were verified on GitHub before any branch deletion.
Original upstream fetch remotes are retained as reference sources.
