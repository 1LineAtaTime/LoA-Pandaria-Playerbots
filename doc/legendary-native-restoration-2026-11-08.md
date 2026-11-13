# Legendary native handler restoration — November 8

## Implemented

The Dragonwrath/Tarecgosa and Fangs of the Father modules were already in the
LoA source/CMake inputs but their initialization functions were absent from
ScriptLoader. Wire those functions into the normal world-script loader. Thirteen
handlers now register, covering Anachronos, Tarecgosa/Kalecgos/Hallegosa ceremony
actors, Mostrasz, Wrathion and the Charging Decoder Ring expiry hook. The exact
names and registration evidence are in the adjacent JSON report.

Entry 52835 now receives its entry-matched Tarecgosa native credit handler only
when no other script/AI is selected. All existing other database bindings stay
intact. The previously selected SmartAI combat fallbacks for Thyrinar 53472 and
Hiram Creed 57802 are retained; their native encounter variants are not registered
in this batch. No boss-parity claim is made.

Before enabling the modules, fix concrete defects in their dormant code:

- Anachronos checked storage against Lovely Charm Collector's Kit 49661 while
  granting Timeless Eye 69238. Check and store the same correct item instead.
- Require the relevant incomplete quest before gossip credit/item delivery;
  check owned/banked items for duplicate Timeless Eyes and handle null item storage.
- Award Tarecgosa proximity credit through the objective system without forcibly
  completing the whole quest.
- Gate the Hallegosa ceremony on the appropriate active quest and gossip state,
  reset stale events/gravity on reappearance, and despawn the reward stalker only
  for its two ceremony quests.

## Automated acceptance

The new reusable gossip_item QA mode declares offline level/location and optional
active-quest setup as fixture preconditions on disposable characters. The test
then uses normal client gossip and item delivery, logs out and verifies the exact
saved item count. Fixture setup is not counted as quest acceptance/completion.

Both targeted live cases passed:

1. Active quest 29193: Anachronos offers the Timeless Eye option; repeated requests
   result in exactly one saved item 69238.
2. No active quest: the option is absent and saved item count remains zero.

All 15 existing supported starter scenarios passed and the comparator found no
regressions. All 17 test accounts/characters were verified removed. Eight broad
capability gaps remain in the starter suite. QA exits nonzero because three
structural audits remain failed, not because these supported client cases failed.

All 22 native CTests and 13 framework tests passed. The declared-dependency audit
checked 23 spells against the loaded Spell.dbc and 9 NPC/2 item references against
the world database, with no missing declared dependencies. It does not establish
spell mechanics. Re-run it with:

```powershell
python contrib/upstream-tests/verify_legendary_dependencies.py
```

## Deployment and rollback

Visual Studio RelWithDebInfo x64 completed with zero failed projects. Both
executable/PDB pairs in exports match the Visual Studio outputs by SHA-256.
Affected world tables and the preceding EXE/PDB/config files are backed up under
ignored out/legendary-native-20261108. The small guarded SQL binding update is
idempotent. Both servers were relaunched through visible export launchers.
Warnings/errors remain enabled. The bounded log comparison recorded 14 absent
SQL signatures, zero new (1,247 archived versus 1,233 current).

## Remaining work

Full Dragonwrath and Fangs journeys, ring expiry, teleports and complete ceremonies
still need scenario-specific acceptance tests. Most of the broader missing-native
backlog has no matching implementation in the source inspected so far. The updated
inventory lists 864 missing native names across all types and 609 creature templates
using missing handlers (348 with spawns; 97 spawned quest-giver templates).
The separate 32 NPC SmartAI fallback inventory remains a native-parity backlog too.
This restoration improves real behavior but does not make every quest playable.
