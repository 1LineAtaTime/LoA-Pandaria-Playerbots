# Historical quest-data reconciliation — November 9, 2026

## Batch scope

Scanned1,398 current/archived SF5.4.8 world SQL files at
`6f000f1878c547f04680cd7c269388a245132ff2`, not just release26 or the newest
update directory. For explicitly keyed quest-field updates, use the **final
SFDB27 full-database value**, rather than replaying each older assignment.
The reference contains17,934 quests. Its actual byte SHA256 is
`d4a2284c9a4f659ab025a8b5dde499bd7a88fa452ff4ba32850eea3689b0b53a`.
Archive/source provenance is in `out/skyfire27-reference/reference.json` and
the earlier release acquisition report.

Final migration contains6,888 field corrections across5,831 existing quests.
The adjacent [machine-readable ledger](historical-quest-field-batch-2026-11-09.json)
records every old/new field value and the historical source statements, or the
canonical prerequisite dependency that required it. Previously only selected
quest/service batches were reconciled. Case-insensitive legacy column spelling
(`PrevQuestID` versus `PrevQuestId`) exposed many additional archived updates.

The batch reconciles prerequisite/follow-up links,exclusive groups,reward and
request text,some reward items,quest labels and a skill requirement. It maps
split modern tables explicitly. It never executes the original historical DDL,
whole-table replacement,GUID-based spawn updates or raw archived SQL.

34,831 mapped fields already matched;323 newer-override fields were preserved;
11 absent quest dependencies and4 signed-next-link compatibility cases remain
recorded and unapplied. No additional
missing linked quest/reward-item dependency was introduced. These counts are
field contracts,not counts of independently proven gameplay bugs.

## Preservation and chain consistency

Newer SF post-dump updates and the latest38 merged LoA SQL inputs protect their
quest identities. Script/phase flags,race/class masks,source/reward spells,
account/character data and custom BattlePay/AH/playerbot tables are outside this
migration. Existing unsupported conditional predicates and unmapped fields are
counted explicitly; they are not silently marked deployed.

Canonical prerequisite ancestors are included when needed. A partial import
initially would have made1365/1366 circular: existing1365 pointed at1366 while
the published1366 correction points at1365. The final batch also restores the
SF parent prerequisite for1365. The complete projected positive-prerequisite
graph changes from10 cycles to4,with **no newly introduced cycle**. The deployer
enforces that check before mutation. The four surviving pre-existing loops are
not claimed fixed and require separate investigation.

## Deployment and rollback

Migration: `sql/updates/world/2026_11_09_02_world_historical_quest_fields.sql`.
SHA256:`d5f7750f8965d1663c15b37afd9212ad228e1c061bfb94128c8469a52bd91cdb`.

The deployer requires a stopped named local realm,strict target schema,exact SQL
hash and expected per-field old/target values. It read-locks and dumps all four
affected public quest tables,applies the migration,checks every target field,
then checks whole-table checksum stability after a second apply. Incomplete
mutations are never retried automatically. Backups/manifests live under
`out/full-history-20261109/quest-fields-expanded/{qa,played}-compatible`.

The earlier738-field QA-only preparation has its original SQL and pre-change
backup retained under `out/full-history-20261109/quest-fields/qa`. Its fields are
a subset of the final migration. The final QA backup therefore includes that
earlier subset; to restore QA fully to the original state,use the earlier
four-table backup. Played rollback uses the final played backup. Stop the world,
restore only those four quest tables,and restart. No auth/characters restore.

There is no native implementation change in this batch. The existing matched
worldserver EXE/PDB and MariaDB DLLs are retained; recompilation is unnecessary
for these SQL corrections. Keep warning/error logging enabled.

## Validation boundaries and remaining work

Seven assembler/deployment logic tests cover field spelling,string parsing,
guarded identity,excluded sensitive fields and prerequisite cycles. Existing
starter protocol checks provide limited acceptance evidence; they do not test
all5,831 affected quests. Startup checks compare exact diagnostic signatures
against the previous QA logs. Detailed run outcomes and deployed state are in
the adjacent ledger and handoff.

This closes a large historical **quest-field** gap. It does not claim every SF
C++ implementation,conditional SQL predicate,objective combination,phase,
encounter,profession or archived schema transition is integrated. Source-presence
inventories and full-game QA coverage remain separate from deployment coverage.

The proposed Westfall native transplant also needs ownership reconciliation:
the current Transient entries already use SmartAI,including quest26209 timed
lists. Empty native ScriptName is not by itself a missing behavior. Do not
overwrite those handlers with the old faction-resetting native script solely
to eliminate a name difference.

## Completed deployment evidence

Both databases have all6,888 target fields verified and identical final checksums
for all four affected tables. The played table backup is22,201,783 bytes;
SHA256:`c91b8dad172e1a45edea80a8686a913e57f47503825f020be6f05fd3dfeda4b1`.
Visible launchers restarted played world43640 on8085 and QA world55116 on8086.
Unchanged auth44944/61780 remain on3724/3725. Verify PIDs before future commands.

The initial738-field QA subset passed22 natural starter cases,with1 unchanged
cached case,batch20261109-174603-3c419a. After final deployment,a forced fresh
Undead rogue accept/reward/reconnect/follow-up case passed in
20261109-175655-d8076e. These are bounded regressions,not acceptance of5,831 quests.
QA startup introduced0 new exact diagnostic signatures. Played startup produced
two new exact messages from existing ContinueTaxiFlight recovery for bot GUIDs
28/46;the taxi code/state tables were not changed. Keep that recovery issue open.
No warning/error levels were reduced.

One expanded QA attempt rejected NextQuestID=-619 because both the active SQL
column and native Quest getter are unsigned. Four source cases are now explicitly
blocked:quests618,8554,12222,12223. The partial QA changes were restored from the
verified table backup,then the revised batch applied cleanly. Failed-attempt
evidence remains in `quest-fields-expanded/qa`;do not replay its obsolete SQL.
An initial text-mode restore encountered Windows cp1252 encoding limitations;
the completed restore used the original UTF8 dump bytes with a completion marker.
Restore these dumps through a byte stream or UTF8-configured client,not the old
Database.q text helper's locale-dependent dump transport.

Historical audit differences for fields in this ledger are superseded by these
final SFDB27 values;do not replay an older desired-value ledger to undo them.
