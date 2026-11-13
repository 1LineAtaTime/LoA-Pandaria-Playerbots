# Starting-zone upstream repair audit — 2026-11-05

Follow-up implementation/deployment: [Jaomin repair](jaomin-repair-2026-11-05.md). Full research inventory: [upstream worklist](skyfire-upstream-worklist-2026-11-05.md). The read-only audit below describes the earlier state.

Research and local inspection only. No runtime database, native source,
configuration or executable was changed for this audit. The user identified
the Pandaren blocker as The Disciple's Challenge, Jaomin Ro. They also recalled
problems in the Worgen chain without identifying a specific current blocker.

## Local Jaomin findings

- Quest29409, main NPC54611; actor57750 is the encounter's hawk/vehicle form.
- Main template uses AIName=SmartAI, empty ScriptName, npcflag0/gossip_menu0.
  It has 29 SmartAI rows and a summoned bunny55708 that triggers its intro by
  storing the nearby invoker and sending SetData0/1 to Jaomin.
- creature_text already contains intro group0 and six defeat variants in group1.
  Missing dialogue alone is therefore not established as the cause. The trigger,
  attackability, phase and duel state need in-game tracing.
- A separate boss_jaomin_ro C++ implementation is compiled/registered in
  src/server/scripts/Pandaria/WanderingIsland/zone_wandering_island_north.cpp,
  but is not the main template's active binding. It differs from SmartAI and
  upstream's npc_jaomin_ro. Blindly changing ScriptName could also retain an
  incompatible AIName, flags or supplemental encounter scripts.

## Primary upstream candidates

SkyFire has actual 5.4.8 implementations, not merely reports of retail bugs.
Its default-branch source was inspected at commit
505f6e63a20c3ea75dfebb17f58e891f4e5fb44c; individual candidate changes below are
immutable references. These are adaptation candidates, not locally validated
fixes or evidence that all quests work.

| Area | Candidate | Relevant behavior |
| --- | --- | --- |
| Jaomin | [d50a59cb — encounter implementation](https://github.com/ProjectSkyfire/SkyFire_548/commit/d50a59cb5c3a5d36a5ca3e7c19cf1474f6ff3a15) | Native npc_jaomin_ro, database binding and dialogue; March17,2024. |
| Jaomin | [23eed671 — vehicle events](https://github.com/ProjectSkyfire/SkyFire_548/commit/23eed67128d0032c50d6b1ea19c6107e5c24ef8b) | Follow-up hawk/vehicle sequence. |
| Jaomin | [7af8f3df — jump correction](https://github.com/ProjectSkyfire/SkyFire_548/commit/7af8f3df528d3bf56907fbea91ab70df57374453) | Baby-elephant attack movement. |
| Jaomin | [e042e3be — knockback correction](https://github.com/ProjectSkyfire/SkyFire_548/commit/e042e3be54700650e0b80f64438dbdca90127d02) | Player knockback. |
| Jaomin | [d12963ad — double-jump lag repair](https://github.com/ProjectSkyfire/SkyFire_548/commit/d12963adc4528289cb29b4309ee1226e5495874b) | Later movement correction; March20,2024. |
| Gilneas14098 | [4581bc62 — Evacuate the Merchant Square](https://github.com/ProjectSkyfire/SkyFire_548/commit/4581bc6238df6d2edd55ee9b4f8dfebccd6609ec) | Door use, frightened citizen, chasing worgen, evacuation credit and repeat-use gate; August12,2026. |
| Gilneas14154 | [42163b72 — By the Skin of His Teeth](https://github.com/ProjectSkyfire/SkyFire_548/commit/42163b724df3528c5906e58d0d6c9c5fce8b699d) | Sean Dempsey waves, Crowley quest-accept/combat, rooftop fighters; August12,2026. |
| Gilneas14159 | [dc61c594 — The Rebel Lord's Arsenal](https://github.com/ProjectSkyfire/SkyFire_548/commit/dc61c594e41b8e7737e1c88d0159a9753d21c0a8) | Avery reward hook/bite scene, phase visibility, duplicate cellar door and trigger visibility; August13,2026. |

[SkyFire release26](https://github.com/ProjectSkyfire/SkyFire_548/releases/tag/sf_db_26)
also documents improvements in several racial starting zones and other quest
chains. That is a useful index of individual changes, not an instruction to
replace this project's custom world database. Its class/spell issues and newer
commits require separate compatibility checks; no universal all-races/classes
repair pack was verified in this research.

[TrinityCore's modern Wandering Isle source](https://trinitycore.net/d7/d43/zone__the__wandering__isle_8cpp_source)
has another complete Jaomin state machine. It can inform encounter semantics,
but its current APIs/data target a different client generation and are not a
drop-in 5.4.8 patch. The Cataclysm Preservation Project likewise targets4.3.4,
so it is a secondary reference for Gilneas, not a replacement core/database.

## Compatibility findings and required verification

The fetched SkyFire Jaomin SQL changes the binding to npc_jaomin_ro and deletes
the existing dialogue before inserting only two rows. That name is not the
local boss_jaomin_ro implementation; copying this SQL alone could introduce an
unregistered script and discard existing authored variants. Preserve the local
text groups and review the actual state machine, defeat/credit behavior,
lethal-hit handling, vehicle ownership and event cancellation before adapting.

The local Gilneas core already has npc_josiah_avery and its worgen-form script,
but template35369 currently selects SmartAI; trigger50415 has no native binding.
Merchant Square Door195327 selects SmartGameObjectAI rather than SkyFire's new
go_merchant_square_door. These differences establish that upstream patches need
review against the active local event chains. They do not establish which
Worgen quest currently fails, or justify replacing every existing event.

Validation should use a fresh ordinary character, preserving existing accounts:

1. Jaomin: correct quest phase, intro, ability to challenge/attack, attack cycle,
   nonlethal defeat, eligible-player credit, and recovery after victory, evade,
   player death/disconnect and a second challenger. Check pets/companions too.
2. Gilneas: door credit, wave survival, Avery/bite scene, transformation and
   phase transitions, then the entire departure chain without GM completion.
3. Expand a race/class acceptance matrix from those verified playthroughs.
   Include reload/relog and quest abandonment so repairs do not merely bypass
   a single step or leave the next NPC invisible.

The existing live runtime remains unchanged and these checklist entries stay
open. No starting-zone fix is claimed deployed or user-validated here.
