# Gate of the Setting Sun crash and absent dungeon objects — November 12

## Root cause and repair

The13:14 local worldserver dump identifies access violation C0000005 in
npc_krikthik_bombarderAI::UpdateAI at gate_of_the_setting_sun.cpp:462.
The controller pointer is null: original mop_world.instance_template map 962
has an empty script binding, while its18 bombardiers execute dungeon AI.
The offending expression unconditionally dereferences pInstance->instance.
This is a dungeon scripting/data mismatch, not evidence of a Detour crash.

Restore the registered instance_gate_of_the_setting_sun binding. Native
bombardier AI now skips its optional bomb lookup when there is no controller
and resolves targets through its own map. Empty random bombardier/stalker
collections return an empty GUID instead of selecting from an empty container.
The rest of the dungeon continues to run; no logging levels are changed.

## Missing floor/lift investigation

The user confirms the ground disappeared after the first section. The original
map had just two portal objects. The matching LoA v1.0.4 reference has82 objects,
including lifts, doors, bombs and destructible corners. Migration08 restores
80 missing positioned objects, placeholder template mechanics and authored
addon flags; existing positioned objects are preserved. It does not regenerate
client maps or MMaps, fabricate bridge geometry, or assert the rendered route
has passed. The missing instance controller also prevented normal gate/event
management. Visual acceptance of the reported gap remains a separate check.

Reference: https://github.com/1LineAtaTime/LoA-Pandaria-Playerbots/releases/tag/v1.0.4
The locally preserved matching world SQL hash is in the private migration report.
This is a scoped reconciliation, not a claim that every dungeon is fully patched.

## Deployment and recovery

Visual Studio RelWithDebInfo/x64 compiled the two native changes with zero failed
projects. Matching worldserver EXE/PDB are staged in exports. Crash dump, report,
pre-change EXE/PDB, four scoped table backups and before/after rows are preserved
under out/backups/gate-setting-sun-crash-20261112. Source migration:
sql/updates/world/2026_11_12_08_world_setting_sun_controller_objects.sql.
Repeated application kept all 82 positioned object rows identical.

To roll back, stop worldserver gracefully, remove only the 80 inserted GUIDs
listed in migration-results.json (compare gameobjects-before.json), and restore
before-scoped-world.sql. Restore the saved EXE/PDB pair if reverting native code.
Do not overwrite account, character, shop or bot databases. The backup includes
exact old controller/template/addon rows; no live character position was changed.

## Validation

Focused native-protocol regression and final live results are appended below.
The regression creates one owned disposable account/character, loads the dungeon
in heroic mode, checks object delivery including the lift, samples server GPS
floor diagnostics and holds the bombardier area across multiple bomb timers.
It cleans its fixture through native character/account deletion. This is not a
rendered-client traversal, boss kill or full dungeon playthrough.

### Entrance-door follow-up

The restored entrance door was correctly locked, but original world data also
omitted all five authored area-trigger bindings. Migration09 restores8359
(first-door opening/intro),7332/7665 (corner events),7104 (pre-Gadok) and8395
(Gadok intro). No NPC dialogue or forced boss completion is required. A world
restart loads these bindings; the existing reload-all-area command does not
reload script bindings. All five previous rows were absent, saved as JSON;
rollback deletes only these five restored script/name pairs. The migration is
repeat-stable and preserves existing nonempty custom bindings.

### Completed native regression

The final fixture passed: actual entrance trigger8359 changed door212982 to
GO_STATE_ACTIVE0; restored bombs/doors were delivered, all 18 bombardiers loaded
and ran across several bomb timers without a crash. The lift211013 was observed
in object updates and a read-only native transport query confirmed map 962,
position1195.21/2304.52/430.22 and animation period23333 ms. GPS found valid
Map/VMap/MMap data at samples, floor388.098 in the first courtyard and381.235 on
the wall route; the saved fall position farther off that route had floor300.859.
The report and owned-fixture cleanup are in native-regression.json under the
private crash backup folder. This does not claim a rendered continuous traversal
or complete encounter playthrough; the reported visual gap should be retried.
