# SF27 complete spell-module integration, November 6

All eighteen active upstream spell modules at df19329b209fa1d1ae80444b43f6201d5dc124f0 are integrated: eleven classes plus generic, quest, item, pet scaling, holiday, mastery and archaeology. Supporting mage and priest summon AI is paired with their data. Local-only registrations remain; shared Polymorph has one owner. Upstream attribution and its license are retained. No auction-house, playerbot or BattlePay implementation file was replaced.

SQL44 pairs the native modules with 1,773 canonical bindings (1,793 individual rank bindings before equivalent negative-first-rank encoding), scoped proc/bonus/link/group/learning rules, area triggers and NPC model/addon data. Seventeen pinned source files are recorded in sf27-native-spell-data.json. Unmentioned custom fields and twelve non-client spell variants remain. The public database reference is never copied over the local world.

Compatibility translations include GUID types, floating aura callbacks, exact effect-index bonus APIs, relative-millisecond cooldowns, pet load/dismiss modes, local SmartAI phase actions and active NPC model/addon storage. Archaeology advances only after successful find creation, because this fork does not advance on loot. Delayed mushroom events use their owning creature's event lifetime rather than an unsupported global creature store.

## Actual failures and repairs

The first startup aborted on duplicate Polymorph registration from spell_common.cpp. The prior executable and fourteen tables were restored, and ownership was corrected. The next startup completed but bot login exposed a recursive pet-load stack overflow. Visual Studio captured Pet::LoadPetFromDB -> spell6962 -> imported spell_gen_pet_summoned -> Pet::LoadPetFromDB repeatedly. This fork emits6962 after attaching the pet; the script now ignores that notification when a pet is already present, retaining genuine missing-pet resummoning. The native fix is30cd0296b.

Four missing supporting summon-AI names were implemented with the published mage/priest pet modules. Second Wind/Sudden Death hooks now match this fork's existing SpellMgr corrections. Clone and corpse-soothing scripts attach handlers to actual supported effects; they do not claim absent effect slots will run. The final build has no new spell-hook or missing supporting-NPC startup errors. No logger suppression was added.

## Verification and runtime

Visual Studio full worldserver build: zero failed projects. Twenty native CTests passed; final executable config tests passed after each native correction. The public SQL44 fixture passes468 value/custom-variant checks, verifies every canonical binding and obsolete-key removal, and is repeat-stable across fourteen tables. Of9,314 older data assertions,9,312 remain identical; two are intentionally superseded: Symbiosis112997 now uses spell_dru_shattering_blow, and Shooting Stars93399 uses the published100% handler-controlled proc chance. Their replacements are covered by SQL44 checks. This is not two uninvestigated failures.

The repaired build logged in193/193 random bots under Visual Studio and remained active beyond the earlier crash point. This observation is limited; it is not an exhaustive class/quest playthrough or a long crash soak. Existing unrelated content errors remain visible.

Private backups and failed/runtime EXE/PDB pairs are under ignored out. The fourteen-table pre-native backup is out/sf27-native-before-20261106-184148.sql,21,500,497bytes, SHA25691b1476ab50854350ba15733a2d793998c61c43473edd940a5c7c10c348d8ebb. Prior runtime is out/runtime-before-sf27-native-20261106-184148. Failed pet-recursion evidence is retained separately; the first standalone recursion dump was empty, so the actual diagnosis comes from the live debugger stack.

Final native30cd0296b exported pair:
- worldserver.exe:71,533,568bytes, SHA25610feaaf93a7a2269a2939a6b13c9dbc4425ea85b93aec435b3535a062ed6d19d
- worldserver.pdb:691,744,768bytes, SHA2566f51c961b4d531dd2d3d2cacda90e03c5699cd4f770673421733ad6ba7b45b71

Remaining SF27 dungeon, world-spawn, quest-event and core differences are tracked in the integration inventory. Completing spell modules does not mean all remaining SF fixes have deployed.
