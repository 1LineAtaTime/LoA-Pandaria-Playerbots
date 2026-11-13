Public delivery note: this is a full integrated source checkout, reconciled
through upstream beta2f9aebde9f. Original overlay instructions below are historical
review guidance; use root CONTRIBUTION.md / UPGRADE.md for the current layout.
Contributed by Nicalicious with imported-source authorship retained.

# Maintainer briefing

## What is being offered

The work extends LoA's existing Playerbots core with a transactional BattlePay
backend and repaired client flows, random-bot market identities and item filters,
controlled player companions and addon commands, city activity, database-client
compatibility, crash guards, quest/phase repairs and an unattended QA toolchain.
It also ports selected SkyFire content/native implementations into LoA's APIs.

The overlay is based on the **beta** branch pinned in the top-level README. The
upstream's stable `playerbot` branch is not the same base. These differences
should not be presented as one conflict-free patch against an arbitrary release.
The maintainer can choose separate PRs or a single integration branch.

All 45 upstream commits through the pinned beta tip were incorporated before
computing this handoff. Their 41 SQL updates already belong to that public base
and are not duplicated as new contributions. The handoff contains the remaining
custom differences, including older quest and compatibility migrations.

## Suggested review/merge groups

| Group | Code/data to review together | Why it is coupled |
|---|---|---|
| Database/build compatibility | Database connection/version/transaction code, CMake detection, probes | Connector behavior and SQL engine assumptions must agree |
| BattlePay | Manager, handlers, prepared statements, accounting/recovery migrations | UI state, wallet mutation and delivery receipts form one workflow |
| Auction market | Buyer/seller ownership, item filter, configuration, filter tests | Visible sellers differ from the market simulator's actual implementation |
| Companions | Access policy, addon dispatcher, group/login/loot code, RBAC migration, config | Every acquisition/control path needs the same ownership/access checks |
| Quest/spell content | Native handlers, paired bindings, phase/status/schema migrations | Adding script names without native registration produces broken content |
| Navigation/stability | Scoped engine guards and regression fixtures | Fallbacks are not proof that data/maps are correct |
| QA tooling | Headless protocol updates, workflows, cleanup, diagnostic reports | Data audits, gameplay evidence and missing coverage must remain distinct |

## Merge policy recommendations

Use the source overlay as the complete sanitized custom version for the listed
files. Use the patch for review or staged application, then inspect the four
overlay-only replacements separately. Keep the existing public copyright and
license notices. Imported SkyFire code retains its attribution; review that
provenance under the repository's normal contribution policy rather than
relabeling imported work as entirely new code.

The SQL inventory is not an instruction to replay every historical migration.
Some files repair assumptions of earlier database revisions; later upstream
content can supersede them. The owner should reconcile against the actual target
schema/data, keep paired code/data changes together and back up affected tables.

No private Git repository, token or existing account is required to use this
handoff. No Git history or author-email history is embedded in the patch.
Original public contributor notices inside source files remain intact.

## What should not be inferred

This is not complete retail-equivalent coverage. The source has real remaining
content/script diagnostics, incomplete natural starting chains and limited
authored gameplay coverage. The catalogue of quests/spells/items is a planning
and structural reference, not proof that every entry was exercised. Autonomous
bot circling/clipping was observed; watching/observer experiments are retired
from the normal QA workflow. See the validation notes before making release claims.
