# Published batch reconciliation — November 9, 2026

**Scope correction after the full-history recheck:** this latest-batch check did
not establish coverage of all older patches. A historical teleport correction
was subsequently found absent and deployed. See
[full-history reconciliation](full-history-reconciliation-2026-11-09.md) for the
10,738-file inventory and remaining open coverage. Do not use this earlier report
to claim that every older repair is installed.

The requested missing-rollup check found no additional **identified, compatible,
ready-to-deploy batch** after refreshing both public remotes. No server restart,
database replay, rebuild or configuration change was performed on this pass.
This is not a declaration that every published SkyFire C++ implementation or
historical SQL statement has been ported, or that all gameplay works.

## Source and deployment evidence

- Refreshed `skyfire/main`: `6f000f1878c547f04680cd7c269388a245132ff2`.
- Refreshed `upstream/playerbot-beta`: `68f80984313dc15a6144d4e91c325632810a488d`.
  This exact LoA tip is an ancestor of the current checkout.
- All150 older LoA backlog commits were included in the full beta integration.
  The subsequent50 commits and38 paired SQL inputs were deployed together;
  see [the batch inventory](published-loa-batch-68f8098431.json) and
  [deployment report](normal-gameplay-campaign-2026-11-09.md).
- SF release26-to27 and subsequent world updates through `a69eda918b` have111
  explicitly adapted/equivalent source dispositions. The later Anzu changes
  `1e21fc79ce`/`607dba2179` were ported in the November8 reconciliation.
  `6f000f1878` moves the pending Anzu SQL into `sql/updates/world`; its
  presence is not a second independent cumulative content release.
- Re-ran `audit_active_rollup.py` against the played `mop_loa_world` database:
  18,766 adapted historical checks,18,748 literal matches,18 differences.
  All18 difference identities exactly match the existing
  [documented dispositions](active-rollup-audit-2026-11-08.json); no unexpected
  differences or missing disposition identities. This recheck compares
  historical desired fields, not every table or full native behavior.
  The full private result is `out/quest-qa/rollup-current-audit.json`.
- `exports/worldserver.exe` SHA256 remains
  `bad4dad484e6b9475bf10e925075c2926db6c6aa6f018aba05a75a1f01f59617`.
  Native source is `2daf99f5f03dec8711fe0663a4fd86db3da995dd`;
  no tracked native source difference from that build was present.
  The played world/auth processes were still running from exports.

## Remaining work is not a missing one-click roll-up

The older native-gap inventory counted860 missing registration names,83 with
literal matches in the tagged SF source. Eight registrations were subsequently
deployed. That inventory is not a fresh count for the current binary and must
not be presented as852 confirmed current defects. Literal matches also include
retired pre-MoP bosses, commented-out events, examples and mechanisms already
owned by differently named local scripts. They are not safe aliases to activate
indiscriminately. Other implementations are genuinely absent or need native
API/encounter adaptation.

Next batches must classify active bindings and ownership, port missing supported
implementations with their paired data, and use the existing QA runner to expose
behavioral failures. Archived release26 subset exclusions also remain explicit
reconciliation work, not proof of a missing downloadable cumulative installer.
Keep these tasks open. Do not replace the customized world with an unrelated
full SF database or claim exhaustive quest coverage from this audit.

The stale unchecked LoA source-integration entry in `project-checklist.md` was
corrected; the gameplay and broader native integration entries remain open.
No warning/error logging was reduced or disabled on this pass.

Public sources:

- [LoA beta](https://github.com/1LineAtaTime/LoA-Pandaria-Playerbots/tree/68f80984313dc15a6144d4e91c325632810a488d)
- [SkyFire current pin](https://github.com/ProjectSkyfire/SkyFire_548/tree/6f000f1878c547f04680cd7c269388a245132ff2)
- [Anzu repair](https://github.com/ProjectSkyfire/SkyFire_548/commit/607dba2179cf7dee3bf0b1247c992330c94b9969)
