# Native persistent gameplay — November 9, 2026

## Acceptance and limits

M0 and M1 pass for one persistent Orc Warrior, GUID120 (ExampleCharacter).
The existing native Playerbots engine played; the thin controller only logged in,
enabled the existing self-bot/quest/grind strategies, saved observations, and
performed ordinary logout/reconnect. No character reset, teleport, quest credit,
XP, item, equipment, or gold grants were used for this pilot. Movement was 1x.
The pilot started at level1 with its original starter equipment and reached level4 in08.
M2 (natural level15) remains incomplete; dungeon and level90 acceptance have not run.

This is one representative natural pilot, not certification of all quests,
classes, races or dungeons. Earlier controlled Worgen fixtures remain a separate
evidence tier. The QA observer is a GM and is excluded from natural certification.
Normal graphical-client observation remains untested; the independent normal
protocol observer saw the pilot move (10 samples, 9 distinct positions).

## Actual gameplay evidence

Immutable local evidence is under ignored `out/quest-qa/adventure/`; credentials
and database snapshots are not committed. The following are actual run IDs:

| Run suffix | Observed behavior | Result |
| --- | --- | --- |
| native-orc-pilot-20261109-04 | Accept/reward25152; accept25126; combat earns level2 | PASS for these transitions; inherited loot rates3x/2x, XP1x |
| 05 | Reward2383; kill six3098 for25126; level3; then keeps grinding instead of returning | FAIL: native proactive attack priority starves quest travel |
| 06-quest-priority | Same persistent character returns within0.37yd of Gornek3143 | Travel repaired; full16-slot backpack blocks reward |
| 07-vendor-recovery | Reward25128; ordinary vendor sales; reward25126; equip53360; kill seven39317 for25172 | PASS; ordinary loot/money/XP rates1x |
| 07 normal reconnect | Exact rewarded quest set and equipment persist | PASS, graceful logout, no run error |
| 08-leveling | Naturally reward25172 and equip its53386 chest upgrade; accept25127 | Mainline25152 ->25126 ->25172 verified; level4 and normal reconnect pass |

Rewarded quests at the end of07 are2383,25126,25128,25152. The observer assigned
the legitimate completed25126 objective using the existing supported whisper
`rpg status do quest 25126` during07 to reproduce the full-bag failure. This
changes only the AI goal, not objectives, quest state, reward eligibility or
rewards. That intervention is recorded in07/objective-assignment.json.

Quest25126 offers three choices. The native item-usage/stat-weight evaluator
selected choice3, item53360 Bragger's Gauntlets (warrior-compatible mail hands),
and equipped it into previously empty slot9. Saved item GUID6827, exact rewarded
quest rows, and unchanged reconnect equipment are independently checked in
07/verified-reward-state.json and07/result.json. The upgrade was earned through
the normal server quest reward path, not inferred from a chat announcement.

## Confirmed repairs

Native commit8e4b42d17c contains three tightly scoped changes:

1. Existing GrindingStrategy suppresses only proactive `attack anything` while
   the existing new-rpg quest strategy travels to its selected POI or turns in a
   completed quest. Defense and combat actions remain available. At-objective
   grinding resumes normally. Failed05 and repaired06/07 supply live evidence.
2. Usage classification includes unusable vendor goods. Previously the usable
   item visitor excluded unlearnable recipes and wrong-class goods before the
   existing `can sell` check. Only the usage query changes; usable selectors keep
   their permission filter. Actual full bags contained eight copies of recipe3609.
3. A completed quest whose reward cannot fit uses existing vendor navigation and
   SellAction, then the normal quest reward path. No deletion, storage-check
   bypass or free bags. The pilot actually sold ordinary goods, returned, earned
   the reward and equipped it. No new gameplay AI was introduced.

The prior sparse creature-equipment fix is commitf5d8fd3d51 (46 actual sparse
sets). The historical loot-reference candidate remains unported: active QA
creature loot contains zero matching rows; a targeted loader fixture is needed.

## Deployment and checks

Only the existing isolated QA runtime was restarted/staged after the directive.
Native source8e4b42d17c, matched EXE/PDB, executable SHA256:
`fb319652eb7983bc422c39b43e3d836202aa0b2136e897d51d000b52bd6ca4ee`.
Its final change versus the live07 binary is reduced vendor debug logging only.
All24 native CTests passed on the material gameplay build; the final Visual
Studio build succeeded. Eight pool/lease/reset tests pass, including protection
of persistent bodies from deterministic reset and selection. Runtime metadata
records source commit, binary hash, controller hash and prior dirty-patch hashes.

The existing QA auth binary/DLLs remain in place for modern MariaDB compatibility.
QA realm1 uses loopback auth3725/world8086 and owned `mop_loa_qa_*` schemas.
No played exports, production configuration or production database was modified
after the master directive. Startup still reports unresolved native script/content
issues (844 missing names); no zero-warning or full-content claim is made.

## Observation and continuation

Use the dedicated QA observer credentials in ignored
`out/quest-qa-runtime/observer-access.json`, the existing QA endpoint, then
`.gm on` and `.appear ExampleCharacter`. Do not publish these credentials or change the
user's installed client configuration without a reason. Existing RPG status
commands expose the pilot's selected quest/activity.

Continue120 without resetting it. The bounded adventure controller resumes its
saved state. Finish level15 naturally, including spell learning and recovery,
before attempting the first legitimate dungeon party. Targeted goal assignments
must be recorded and must never substitute quest completion/rewards.

Known setup side effect: RandomBotAccountCount0 means auto-sizing rather than
zero creation. Initial enabling created264 extra offline bodies outside the
original125-body pool. Autologin, min/max population and extra class pools are
now explicitly zero. These bodies remain excluded from certification; do not
delete original test history. Any cleanup needs exact ownership proof, backups
and a scoped reversible plan.
