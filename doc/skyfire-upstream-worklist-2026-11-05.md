# SkyFire upstream issue and fix review worklist — November5,2026

The user's first priority is the Pandaren progression blocker29409, The
Disciple's Challenge (Jaomin Ro54611). Existing starting-zone research is in
[starting-zone upstream audit](starting-zone-upstream-audit-2026-11-05.md).

This inventory captures all195 open issue reports returned by SkyFire's GitHub
API on this date (26 pull requests excluded), plus the release26 world-update
archive. An upstream report is a candidate to investigate, not proof that this
fork has the same defect; a closed/upstream repair is not proof of a local fix.
Do not bulk-import replacement world/auth/character databases. Preserve custom
shop/loot, accounts and characters. Compare each repair with the active local
script, schema and installed5.4.8 DBC; back up and verify affected behavior.

## Starting-zone implementation priorities

- [x] P0 Jaomin Ro29409 reported progression blocker: native repair/SQL24 deployed; ExampleCharacter confirmed close-range attackability and quest completion.
- [ ] Jaomin extended acceptance: group/pet credit, full hawk sequence, evade/death/disconnect recovery and repeat challengers; see jaomin-repair-2026-11-05.md.
- [ ] Gilneas14098 Merchant Square evacuation: review SkyFire4581bc6238df6d2edd55ee9b4f8dfebccd6609ec; validate door gating, citizen evacuation and credit.
- [ ] Gilneas14154 By the Skin of His Teeth: review42163b724df3528c5906e58d0d6c9c5fce8b699d; validate waves, survival and departure.
- [ ] Gilneas14159 The Rebel Lord's Arsenal: reviewdc61c594e41b8e7737e1c88d0159a9753d21c0a8; validate bite, phase and transformation.
- [ ] Review the archived starting-zone fixes for Coldridge Valley, Gnomeregan,
  Teldrassil, Echo Isles and Wandering Isle before full race/class playthroughs.
- [ ] Triage crash, unsafe-state and progression blockers ahead of cosmetic/text
  issues; maintain per-fix local evidence and test results.

## Local triage evidence

- SQL29 adds the reviewed nonzero training-chain subset to SQL27's numeric
  adaptations: nine corrected fields, including the Monk letter31141, with
  upstream Night Elf3106/3116 and warlock26916/26914 ID mistakes corrected.
  Four existing tutorial prerequisites are preserved. Full natural class/race
  progression remains unverified; see native-content-campaign-2026-11-05.md.

- All 296 release26 SQL files now have independent disposable-fixture results in
  [published SQL compatibility](published-sql-compatibility-2026-11-05.csv):
  175 changed, 70 no observed change, 51 failed. These are audit outcomes only;
  do not check off gameplay repairs from SQL execution. Existing local adaptations
  remain documented separately. See
  [automated coverage report](automated-content-testing-2026-11-05.md).

- SF#1553 (Hunter Disengage): unaffected by the reported obsolete-link defect.
  Local spell_linked_spell has only781→115927 (Glyph of Liberation), not the
  offending781→56446 link. Local EffectLeapBack explicitly sends781 backward.
  No database/native change made; this is configuration/source verification,
  not a new hunter client movement acceptance test.

- SF#1408 core null-target crash: local nearby-target selection had the unsafe
  post-hook dereference. Added the published guard; native build and17 CTests
  passed. Exact Rune quest workflow is not claimed playtested.
- Release26 2026_08_09_world_04/_05: SQL26 adapts the Stair of Destiny fix to
  reversed local option IDs and complete-or-incomplete active quest states.
  Backed-up application/fixture checks passed; player flight acceptance pending.
- Release26 2026_08_11_world_00 Tarindrella: local92420 destination already
  matches the published coordinates/orientation; no data change needed.

## All open upstream issue reports

Every item below starts unverified on this local fork. Check off only after
local reproduction is repaired/tested, or after recording evidence that the
local implementation is unaffected/already repaired. Titles are upstream report
metadata; follow their links for complete details and any proposed patches.

- [ ] [SF#1556](https://github.com/ProjectSkyfire/SkyFire_548/issues/1556) Title: \[DB\]\[Duskwood\] Chef Grual (272) has duplicate spawns in Darkshire
- [ ] [SF#1555](https://github.com/ProjectSkyfire/SkyFire_548/issues/1555) Title: \[Quest\]\[Redridge Mountains\] Several quests cannot be accepted due to missing quest givers and incorrect quest offer handling
- [ ] [SF#1554](https://github.com/ProjectSkyfire/SkyFire_548/issues/1554) \[Quest\]\[Redridge Mountains\] Marshal Marris (382) is invisible because the player is missing the detection aura
- [x] [SF#1553](https://github.com/ProjectSkyfire/SkyFire_548/issues/1553) \[Spell\]\[Hunter\] Disengage (781) jumps forward because of an obsolete linked spell
- [ ] [SF#1552](https://github.com/ProjectSkyfire/SkyFire_548/issues/1552) \[DB\]\[Spawns\] Torrention (41167) has two simultaneous nearby spawns
- [ ] [SF#1551](https://github.com/ProjectSkyfire/SkyFire_548/issues/1551) \[DB\]\[Spawns\] Cave Stalker (4040) has overlapping duplicate spawns
- [ ] [SF#1550](https://github.com/ProjectSkyfire/SkyFire_548/issues/1550) \[DB\]\[Loot\] Drungeld Glowerglare (41151) has only quest loot and no regular drops
- [ ] [SF#1549](https://github.com/ProjectSkyfire/SkyFire_548/issues/1549) \[DB\]\[Loot\] Dark Iron Holdout (41394) has no loot or money configured
- [ ] [SF#1547](https://github.com/ProjectSkyfire/SkyFire_548/issues/1547) \[Quest\]\[Redridge Mountains\] Detonation (26668) is missing the explosion sequence and quest NPC
- [ ] [SF#1546](https://github.com/ProjectSkyfire/SkyFire_548/issues/1546) \[Quest\]\[Redridge Mountains\] To Win a War, You Gotta Become War (26651) is missing charge planting logic and the completion NPC
- [ ] [SF#1545](https://github.com/ProjectSkyfire/SkyFire_548/issues/1545) \[Quest\]\[Redridge Mountains\] Prisoners of War (26646) is missing the prisoner rescue and completion logic
- [ ] [SF#1544](https://github.com/ProjectSkyfire/SkyFire_548/issues/1544) \[Quest\]\[Redridge Mountains\] Point of Contact: Brubaker (26639) cannot be completed because Brubaker is missing
- [ ] [SF#1543](https://github.com/ProjectSkyfire/SkyFire_548/issues/1543) \[Quest\]\[Redridge Mountains\] It's Never Over (26616) is missing the riverboat ride and quest completion NPC
- [ ] [SF#1542](https://github.com/ProjectSkyfire/SkyFire_548/issues/1542) \[Core\]\[Movement\] Amphibious murlocs stop chasing players in water in Redridge Mountains
- [ ] [SF#1541](https://github.com/ProjectSkyfire/SkyFire_548/issues/1541) \[Quest\]\[Redridge Mountains\] Breaking Out is Hard to Do (26587) is missing Messner's rescue script
- [ ] [SF#1540](https://github.com/ProjectSkyfire/SkyFire_548/issues/1540) \[Quest\]\[Redridge Mountains\] In Search of Bravo Company (26586) cannot be completed because Messner is invisible
- [ ] [SF#1539](https://github.com/ProjectSkyfire/SkyFire_548/issues/1539) \[Quest\]\[Redridge Mountains\] John J. Keeshan (26567) cannot be completed because the quest NPC is missing
- [ ] [SF#1538](https://github.com/ProjectSkyfire/SkyFire_548/issues/1538) \[Quest\] Still Assessing the Threat (26503) one POI
- [ ] [SF#1534](https://github.com/ProjectSkyfire/SkyFire_548/issues/1534) \[Loot\] Displaced Threshadon has no regular loot or skinning loot.
- [ ] [SF#1533](https://github.com/ProjectSkyfire/SkyFire_548/issues/1533) \[Loot\] Flood Elemental (41132) has no loot.
- [ ] [SF#1532](https://github.com/ProjectSkyfire/SkyFire_548/issues/1532) duplicate #include "SpellMgr.h" in SmartScriptMgr.cpp
- [ ] [SF#1531](https://github.com/ProjectSkyfire/SkyFire_548/issues/1531) Client crashes (STACK_OVERFLOW / ACCESS_VIOLATION) when a player is within ~500 yards of Dalaran (also Thrallmar, Silvermoon). Server sends normal traffic; crash needs a combination of Dalaran creature spawns
- [ ] [SF#1526](https://github.com/ProjectSkyfire/SkyFire_548/issues/1526) \[Script\] Anzu: encounter differs from 5.4.8 — skipped first banish, 66/33 vs 75/35, no early-banish-cancel, Flesh Rip/Dive/Brood Screech missing
- [ ] [SF#1524](https://github.com/ProjectSkyfire/SkyFire_548/issues/1524) \[Database\] Anzu (23035) never spawns in Heroic Sethekk Halls — still gated behind removed druid summon (permanent since 4.0.1)
- [ ] [SF#1522](https://github.com/ProjectSkyfire/SkyFire_548/issues/1522) Outland dungeons: death releases outside the instance — game_graveyard_zone missing in-instance graveyard links (all 13)
- [ ] [SF#1521](https://github.com/ProjectSkyfire/SkyFire_548/issues/1521) Raise Dead with Master of Ghouls (52150) summons nothing — references SummonProperties id 1, which does not exist in SummonProperties.dbc
- [ ] [SF#1515](https://github.com/ProjectSkyfire/SkyFire_548/issues/1515) \[Quest\] Axis of Awful (26868) is missing its items and scripted event
- [ ] [SF#1513](https://github.com/ProjectSkyfire/SkyFire_548/issues/1513) \[Data request\] Original MoP POI evidence for 12 empty blobs (NightElf export: 27316 / 13892)
- [ ] [SF#1512](https://github.com/ProjectSkyfire/SkyFire_548/issues/1512) \[Quest\] The Winds of Loch Modan (ID 27116) — Skystrider is not spawned
- [ ] [SF#1511](https://github.com/ProjectSkyfire/SkyFire_548/issues/1511) \[Loot\] Mo'grosh Darkmauler (44758) and Mo'grosh Earthbender (44760) have no loot
- [ ] [SF#1510](https://github.com/ProjectSkyfire/SkyFire_548/issues/1510) \[Loot\] Twilight Encroacher has no loot
- [ ] [SF#1509](https://github.com/ProjectSkyfire/SkyFire_548/issues/1509) \[Loot\] "Commander" Nazrim has no loot
- [ ] [SF#1508](https://github.com/ProjectSkyfire/SkyFire_548/issues/1508) Billibub Cogspinner's vendor window does not open
- [ ] [SF#1507](https://github.com/ProjectSkyfire/SkyFire_548/issues/1507) \[Quest 13650\] Keep Your Hands Off The Goods! cannot be completed
- [ ] [SF#1506](https://github.com/ProjectSkyfire/SkyFire_548/issues/1506) \[Loot\] Proto-Strider (44576) and Optimo (45399) have no loot
- [ ] [SF#1505](https://github.com/ProjectSkyfire/SkyFire_548/issues/1505) \[Loot\] Haywire Battle-Chicken (NPC 44616) has no loot
- [ ] [SF#1504](https://github.com/ProjectSkyfire/SkyFire_548/issues/1504) \[Loot\] Golden Eagle (NPC 44628) only has quest loot
- [ ] [SF#1503](https://github.com/ProjectSkyfire/SkyFire_548/issues/1503) \[Loot\] Hill Fox (44635) and Ashtail (45380) only have quest loot
- [ ] [SF#1502](https://github.com/ProjectSkyfire/SkyFire_548/issues/1502) \[Loot\] Hill Grizzly (NPC 44638) has no loot or skinning loot
- [ ] [SF#1501](https://github.com/ProjectSkyfire/SkyFire_548/issues/1501) \[Loot\] Bobcat (NPC 44188) has no loot
- [ ] [SF#1500](https://github.com/ProjectSkyfire/SkyFire_548/issues/1500) \[Loot\] Mudbelly Boar has no corpse or skinning loot
- [ ] [SF#1491](https://github.com/ProjectSkyfire/SkyFire_548/issues/1491) \[Loot\] Nix (45402) has no normal loot
- [ ] [SF#1490](https://github.com/ProjectSkyfire/SkyFire_548/issues/1490) \[Loot\] Bluegill Wanderer and Bluegill Mudskipper have no normal loot
- [ ] [SF#1489](https://github.com/ProjectSkyfire/SkyFire_548/issues/1489) \[Quest\] Protecting the Shipment (309) event does not start
- [ ] [SF#1488](https://github.com/ProjectSkyfire/SkyFire_548/issues/1488) \[Quest\] Resupplying the Excavation is offered twice and only one version can be completed
- [ ] [SF#1487](https://github.com/ProjectSkyfire/SkyFire_548/issues/1487) \[Loot\] Burning Invader (45336) has no loot
- [ ] [SF#1486](https://github.com/ProjectSkyfire/SkyFire_548/issues/1486) \[Loot\] Loch Buzzard (44184) has no loot
- [ ] [SF#1485](https://github.com/ProjectSkyfire/SkyFire_548/issues/1485) \[Quest\] Feeding the Hungry and the Hopeless (26271): homeless NPCs ignore placed food
- [ ] [SF#1484](https://github.com/ProjectSkyfire/SkyFire_548/issues/1484) \[Quest\] Secrets of the Tower (26290): missing NPCs and conversation event
- [ ] [SF#1483](https://github.com/ProjectSkyfire/SkyFire_548/issues/1483) \[NPC\] Nipsy's shop does not open due to incorrect gossip option settings
- [ ] [SF#1482](https://github.com/ProjectSkyfire/SkyFire_548/issues/1482) \[Spawns\] Duplicate Monty and Nipsy in Deeprun Tram
- [ ] [SF#1481](https://github.com/ProjectSkyfire/SkyFire_548/issues/1481) \[Loot\] Mosshide Scout and Mosshide Basher have no normal loot or money
- [ ] [SF#1480](https://github.com/ProjectSkyfire/SkyFire_548/issues/1480) \[Quest\] Grimaxe's Demise (26102): General Grimaxe cannot be attacked and is not mounted on his golem
- [ ] [SF#1479](https://github.com/ProjectSkyfire/SkyFire_548/issues/1479) \[Quest\] Striking Back (26094): Repaired Bomber cannot be used
- [ ] [SF#1478](https://github.com/ProjectSkyfire/SkyFire_548/issues/1478) Guild tabard design cannot be saved; background and border colors are swapped
- [ ] [SF#1477](https://github.com/ProjectSkyfire/SkyFire_548/issues/1477) \[Priest\] Wand Shoot cancels itself before firing due to its own global cooldown
- [ ] [SF#1476](https://github.com/ProjectSkyfire/SkyFire_548/issues/1476) Livin' the Life (26228): quest item fails with “Requires %s” in Jangolode Mine
- [ ] [SF#1475](https://github.com/ProjectSkyfire/SkyFire_548/issues/1475) Undiscovered flight masters do not display the green exclamation mark
- [ ] [SF#1474](https://github.com/ProjectSkyfire/SkyFire_548/issues/1474) Renew (139) can be learned at level 8, causing inconsistent low-level casting behavior
- [ ] [SF#1473](https://github.com/ProjectSkyfire/SkyFire_548/issues/1473) Title: The Collector (123) and Manhunt (147): turn-in markers point to Goldshire instead of Eastvale Logging Camp
- [ ] [SF#1430](https://github.com/ProjectSkyfire/SkyFire_548/issues/1430) Extinguish the Fires (26078): missing SmartAI prevents fires from reacting to firefighting equipment
- [ ] [SF#1429](https://github.com/ProjectSkyfire/SkyFire_548/issues/1429) Get to the Airfield (25998): clicking Mathel's Flying Machine does nothing
- [ ] [SF#1428](https://github.com/ProjectSkyfire/SkyFire_548/issues/1428) Entombed in Ice (25978): missing scripts and friendly Icy Tomb faction prevent completion
- [ ] [SF#1427](https://github.com/ProjectSkyfire/SkyFire_548/issues/1427) Dark Iron Scheming (25997): Captain Beld is missing
- [ ] [SF#1426](https://github.com/ProjectSkyfire/SkyFire_548/issues/1426) Helm's Bed Surger (41762) has no loot configured
- [ ] [SF#1425](https://github.com/ProjectSkyfire/SkyFire_548/issues/1425) The Public Servant (433): missing Trapped Miner SmartAI in the world database
- [ ] [SF#1423](https://github.com/ProjectSkyfire/SkyFire_548/issues/1423) How to submit a database fix?
- [ ] [SF#1421](https://github.com/ProjectSkyfire/SkyFire_548/issues/1421) Secrets of the Tower 26290  Kearnen not killing the goons
- [ ] [SF#1420](https://github.com/ProjectSkyfire/SkyFire_548/issues/1420) Quest Morbents Bane 26754 Immediately acceptable by characters
- [ ] [SF#1419](https://github.com/ProjectSkyfire/SkyFire_548/issues/1419) Furlbrows Deed 184 quest text incorrect.
- [ ] [SF#1417](https://github.com/ProjectSkyfire/SkyFire_548/issues/1417) Crafting ignores recipe-specific skill gains and awards only one skill point
- [ ] [SF#1416](https://github.com/ProjectSkyfire/SkyFire_548/issues/1416) Guild bank: items cannot be deposited from the first bag slot
- [ ] [SF#1415](https://github.com/ProjectSkyfire/SkyFire_548/issues/1415) It's Raid Night Every Night (25932): Amberstill Ranch defense event appears unimplemented
- [ ] [SF#1414](https://github.com/ProjectSkyfire/SkyFire_548/issues/1414) Rams on the Lam (25905): Stolen Rams do not respond to /whistle
- [ ] [SF#1413](https://github.com/ProjectSkyfire/SkyFire_548/issues/1413) Frostmane Raider (41544) has no loot or money configured
- [ ] [SF#1412](https://github.com/ProjectSkyfire/SkyFire_548/issues/1412) Strike From Above (25841): Signal Flare does not grant objective credit
- [ ] [SF#1411](https://github.com/ProjectSkyfire/SkyFire_548/issues/1411) Frostmane Warrior (41258) and Battok the Berserker (41284) have no loot configured
- [ ] [SF#1410](https://github.com/ProjectSkyfire/SkyFire_548/issues/1410) Ultrasafe Personnel Launcher does nothing after boarding (quest 25839)
- [ ] [SF#1409](https://github.com/ProjectSkyfire/SkyFire_548/issues/1409) Missing regular loot for Frostmane Scout (41175) and Frostmane Scavenger (41146)
- [x] [SF#1408](https://github.com/ProjectSkyfire/SkyFire_548/issues/1408) Rune of Fire can crash worldserver when no Constriction Totem is within range
- [ ] [SF#1407](https://github.com/ProjectSkyfire/SkyFire_548/issues/1407) Young Wendigo (40940) and Wendigo (40941) have no loot configured
- [ ] [SF#1406](https://github.com/ProjectSkyfire/SkyFire_548/issues/1406) Frostmane Seer (41121) and Frostmane Snowstrider (41122) have no loot configured
- [ ] [SF#1405](https://github.com/ProjectSkyfire/SkyFire_548/issues/1405) Fishing in Iceflow Lake yields no loot (missing area/zone fishing data)
- [ ] [SF#1404](https://github.com/ProjectSkyfire/SkyFire_548/issues/1404) Quest 14079 (Learn to Ride in Elwynn Forest) not able to complete.
- [ ] [SF#1403](https://github.com/ProjectSkyfire/SkyFire_548/issues/1403) Frostmane Blade (37507) drops no loot or money
- [ ] [SF#1401](https://github.com/ProjectSkyfire/SkyFire_548/issues/1401) Quest 26128 Battle of Thandol: Speech/enemy not spawning.
- [ ] [SF#1392](https://github.com/ProjectSkyfire/SkyFire_548/issues/1392) Desolace is Empty (truly it is now Desolate
- [ ] [SF#1391](https://github.com/ProjectSkyfire/SkyFire_548/issues/1391) Officer Jankie 36410 not where they should be.
- [ ] [SF#1390](https://github.com/ProjectSkyfire/SkyFire_548/issues/1390) NPC Singleton 38379 is not present.
- [ ] [SF#1389](https://github.com/ProjectSkyfire/SkyFire_548/issues/1389) Arcane Legacy  quest 25766 Multiple bugs.
- [ ] [SF#1388](https://github.com/ProjectSkyfire/SkyFire_548/issues/1388) Quest "Save the children!" 25889 cannot mount hippogryph
- [ ] [SF#1387](https://github.com/ProjectSkyfire/SkyFire_548/issues/1387) quest Grundig Darkcloud quest 25846 - Grundig is Hostile
- [ ] [SF#1385](https://github.com/ProjectSkyfire/SkyFire_548/issues/1385) Quest Three Friends of the Forest 13976 the friends are not there.
- [ ] [SF#1384](https://github.com/ProjectSkyfire/SkyFire_548/issues/1384) Quest Respect for the Fallen 13626 Corpses not lootable
- [ ] [SF#1383](https://github.com/ProjectSkyfire/SkyFire_548/issues/1383) Cannot kill these guys or even mouse over to see what they are
- [ ] [SF#1382](https://github.com/ProjectSkyfire/SkyFire_548/issues/1382) Quest Cleansing the Afflicted 13545
- [ ] [SF#1381](https://github.com/ProjectSkyfire/SkyFire_548/issues/1381) Aros 34446 is broken for quest 13895
- [ ] [SF#1380](https://github.com/ProjectSkyfire/SkyFire_548/issues/1380) Quest Leave no Tracks 13892 cannot accept quest
- [ ] [SF#1379](https://github.com/ProjectSkyfire/SkyFire_548/issues/1379) Quest the Emerald Dream 13586 Dragon is not there.
- [ ] [SF#1378](https://github.com/ProjectSkyfire/SkyFire_548/issues/1378) Quest Soothing the Elements 13580 gives %s
- [ ] [SF#1377](https://github.com/ProjectSkyfire/SkyFire_548/issues/1377) Instead of 'quest accepted' I am seeing 'Objective Complete.
- [ ] [SF#1376](https://github.com/ProjectSkyfire/SkyFire_548/issues/1376) Quest Three HAmmers to Break 13543
- [ ] [SF#1375](https://github.com/ProjectSkyfire/SkyFire_548/issues/1375) Quest The Last Refugee 13605 not triggering the walk to safety
- [ ] [SF#1374](https://github.com/ProjectSkyfire/SkyFire_548/issues/1374) Quest Call down the THunder error 13558 not triggering
- [ ] [SF#1373](https://github.com/ProjectSkyfire/SkyFire_548/issues/1373) Quest Coaxing the Spirits 13547 errro
- [ ] [SF#1371](https://github.com/ProjectSkyfire/SkyFire_548/issues/1371) Quest Ritual Materials issue 13566
- [ ] [SF#1369](https://github.com/ProjectSkyfire/SkyFire_548/issues/1369) Quest War Supplies 13509  cannot burn crates
- [ ] [SF#1362](https://github.com/ProjectSkyfire/SkyFire_548/issues/1362) Denalan freaks out after you complete quest The Sprouted Fronds
- [ ] [SF#1361](https://github.com/ProjectSkyfire/SkyFire_548/issues/1361) Spirit of Sathrah visible before you kill her.
- [ ] [SF#1360](https://github.com/ProjectSkyfire/SkyFire_548/issues/1360) Quest Druid of the claw 2561: Door does not open
- [ ] [SF#1359](https://github.com/ProjectSkyfire/SkyFire_548/issues/1359) Quest Natures Reprisal 2459 the seed is not doing anything.
- [ ] [SF#1357](https://github.com/ProjectSkyfire/SkyFire_548/issues/1357) Quest Seek Redemption! 489 Error
- [ ] [SF#1353](https://github.com/ProjectSkyfire/SkyFire_548/issues/1353) Quest Pack your Bags 24492 - When you leave the aircraft you parachute through to the underside of the world.
- [ ] [SF#1352](https://github.com/ProjectSkyfire/SkyFire_548/issues/1352) Quest Trolling for Information 24489 not triggering speech
- [ ] [SF#1351](https://github.com/ProjectSkyfire/SkyFire_548/issues/1351) Quest Dwarven Artifacts 24477 says 'that  is already being used.'
- [ ] [SF#1349](https://github.com/ProjectSkyfire/SkyFire_548/issues/1349) Twin Nipsies 13018
- [ ] [SF#1348](https://github.com/ProjectSkyfire/SkyFire_548/issues/1348) Deeprun Rat Roundup 6661 not working
- [ ] [SF#1347](https://github.com/ProjectSkyfire/SkyFire_548/issues/1347) Quest 26760 Cry for the Moon not completing
- [ ] [SF#1345](https://github.com/ProjectSkyfire/SkyFire_548/issues/1345) Quest The Embalmer's Revenge not triggering Stitches (26727)
- [ ] [SF#1343](https://github.com/ProjectSkyfire/SkyFire_548/issues/1343) Quest "Puting the Crunch in the Frog" 29509 not working
- [ ] [SF#1342](https://github.com/ProjectSkyfire/SkyFire_548/issues/1342) Quest Tan My Hide 29519 not working
- [ ] [SF#1341](https://github.com/ProjectSkyfire/SkyFire_548/issues/1341) Quest Injured Carnie 54518 not working
- [ ] [SF#1340](https://github.com/ProjectSkyfire/SkyFire_548/issues/1340) Impaled Blackrock Orc showing data
- [ ] [SF#1339](https://github.com/ProjectSkyfire/SkyFire_548/issues/1339) Chef Grual 272 has a twin!
- [ ] [SF#1338](https://github.com/ProjectSkyfire/SkyFire_548/issues/1338) Quest John J. Keeshan 26567 not working.
- [ ] [SF#1337](https://github.com/ProjectSkyfire/SkyFire_548/issues/1337) Quest Rise of the Brotherhood does not trigger 26322
- [ ] [SF#1336](https://github.com/ProjectSkyfire/SkyFire_548/issues/1336) Quest: A Vision of the Past (26320) not triggering
- [ ] [SF#1335](https://github.com/ProjectSkyfire/SkyFire_548/issues/1335) Quest "The Dawning of a new Day" 26297 quest not triggering event.
- [ ] [SF#1334](https://github.com/ProjectSkyfire/SkyFire_548/issues/1334) Secrets of the Tower quest 26290 potion not working.
- [ ] [SF#1333](https://github.com/ProjectSkyfire/SkyFire_548/issues/1333) Quest Feeding the homeless... not working 26271
- [ ] [SF#1332](https://github.com/ProjectSkyfire/SkyFire_548/issues/1332) Quest Lou's Parting Thoughts 26232 not triggering
- [ ] [SF#1331](https://github.com/ProjectSkyfire/SkyFire_548/issues/1331) Quest 26228 cannot use crate.
- [ ] [SF#1330](https://github.com/ProjectSkyfire/SkyFire_548/issues/1330) Fishing Trainers lacking information.
- [ ] [SF#1328](https://github.com/ProjectSkyfire/SkyFire_548/issues/1328) Quest Power of the Light 26918 not working
- [ ] [SF#1327](https://github.com/ProjectSkyfire/SkyFire_548/issues/1327) \[Quest ID 14294\] Due to incorrect phasing, King Genn Greymane and other NPCs do not spawn in the Greymane Court
- [ ] [SF#1326](https://github.com/ProjectSkyfire/SkyFire_548/issues/1326) After completing From the Shadows, phasing is incorrect
- [ ] [SF#1325](https://github.com/ProjectSkyfire/SkyFire_548/issues/1325) \[Quest ID 14293\] After accepting quest, no horse is spawned and quest cannot be completed
- [ ] [SF#1324](https://github.com/ProjectSkyfire/SkyFire_548/issues/1324) \[Quest ID 14159\] King Genn Greymane offers this quest right after meeting him
- [ ] [SF#1323](https://github.com/ProjectSkyfire/SkyFire_548/issues/1323) \[Quest ID 14099\] Getting close to Gwen Armstead causes her and the other NPCs to phase out
- [ ] [SF#1322](https://github.com/ProjectSkyfire/SkyFire_548/issues/1322) Cannot target mobs (sometimes) with spells on Worgen starting scenario
- [ ] [SF#1320](https://github.com/ProjectSkyfire/SkyFire_548/issues/1320) End of the Deserters - 27213 Gavis isnt there.
- [ ] [SF#1319](https://github.com/ProjectSkyfire/SkyFire_548/issues/1319) Discrediting the Deserters 27212 does not work
- [ ] [SF#1318](https://github.com/ProjectSkyfire/SkyFire_548/issues/1318) Marleys' Final Flight 25120 not working
- [ ] [SF#1316](https://github.com/ProjectSkyfire/SkyFire_548/issues/1316) Worgen starting scenario worgen are neutral and do not fight the soldiers
- [ ] [SF#1314](https://github.com/ProjectSkyfire/SkyFire_548/issues/1314) Quest Claim the Battlescar 25081 Error
- [ ] [SF#1313](https://github.com/ProjectSkyfire/SkyFire_548/issues/1313) Quest Working the Bugs Out 25082 Item not working.
- [ ] [SF#1312](https://github.com/ProjectSkyfire/SkyFire_548/issues/1312) Quest Clap Em in Irons 25057 doesnt work
- [ ] [SF#1311](https://github.com/ProjectSkyfire/SkyFire_548/issues/1311) Quest A Line in the Dirt 25045  Not working
- [ ] [SF#1309](https://github.com/ProjectSkyfire/SkyFire_548/issues/1309) Quest Diplomacy by another means not working
- [ ] [SF#1308](https://github.com/ProjectSkyfire/SkyFire_548/issues/1308) Adder Subtraction 24570 quest not working
- [ ] [SF#1307](https://github.com/ProjectSkyfire/SkyFire_548/issues/1307) Ongoing list of npcs that are not loot/skinnable.
- [ ] [SF#1306](https://github.com/ProjectSkyfire/SkyFire_548/issues/1306) Quest Sowing a Solution (24566) doesnt work
- [ ] [SF#1305](https://github.com/ProjectSkyfire/SkyFire_548/issues/1305) Mangled Body not starting quest
- [ ] [SF#1304](https://github.com/ProjectSkyfire/SkyFire_548/issues/1304) Quest 24934 - Repel boarders!  the Admiral does not help you fly to the ship.
- [ ] [SF#1303](https://github.com/ProjectSkyfire/SkyFire_548/issues/1303) Quest: run out the Guns.  Guns not there.
- [ ] [SF#1302](https://github.com/ProjectSkyfire/SkyFire_548/issues/1302) Keep watcher Kerry Not doing anything.
- [ ] [SF#1301](https://github.com/ProjectSkyfire/SkyFire_548/issues/1301) Quest 25027 Item does not work.
- [ ] [SF#1300](https://github.com/ProjectSkyfire/SkyFire_548/issues/1300) All quests available at once.
- [ ] [SF#1299](https://github.com/ProjectSkyfire/SkyFire_548/issues/1299) Quest 25022 Ambassador Gaines not there.
- [ ] [SF#1288](https://github.com/ProjectSkyfire/SkyFire_548/issues/1288) \[Spell\] Swiftmend incorrectly removes the active Rejuvenation or Regrowth aura
- [ ] [SF#1285](https://github.com/ProjectSkyfire/SkyFire_548/issues/1285) Ironforge Airport - Missing Spirit Healer
- [ ] [SF#1284](https://github.com/ProjectSkyfire/SkyFire_548/issues/1284) Missing spawns for 5 MoP dungeons in the database release (Mogu'shan Palace, Scarlet Halls, Scarlet Monastery, Scholomance, Siege of Niuzao Temple)
- [ ] [SF#1282](https://github.com/ProjectSkyfire/SkyFire_548/issues/1282) Tame effects call AddToMap() before SetMinion() — pet uncontrollable client-side until relog
- [ ] [SF#1281](https://github.com/ProjectSkyfire/SkyFire_548/issues/1281) Missing quest marks on npcs with feign death aura.
- [ ] [SF#1279](https://github.com/ProjectSkyfire/SkyFire_548/issues/1279) Quest: The way of the Tushui causes disconnect
- [ ] [SF#1272](https://github.com/ProjectSkyfire/SkyFire_548/issues/1272) Quest: Lifting the siege: Everything is already on fire.
- [ ] [SF#1269](https://github.com/ProjectSkyfire/SkyFire_548/issues/1269) Killing Spree can teleport the rogue into geometry — SpellScript has no hook to correct a destination point before a teleport effect resolves
- [ ] [SF#1267](https://github.com/ProjectSkyfire/SkyFire_548/issues/1267) Terrortooth Runner not dropping loot/not skinnable.
- [ ] [SF#1258](https://github.com/ProjectSkyfire/SkyFire_548/issues/1258) Lifebloom (Druid) double-heals on its final bloom, both on natural expiry and on dispel
- [ ] [SF#1248](https://github.com/ProjectSkyfire/SkyFire_548/issues/1248) Three Bristleback quilboar in Mulgore use a neutral faction instead of hostile (breaks two Tauren racial starting quests)
- [ ] [SF#1247](https://github.com/ProjectSkyfire/SkyFire_548/issues/1247) Blackrock Spy (49874) and Goblin Assassin (50039) in Northshire use a neutral faction instead of hostile
- [ ] [SF#1246](https://github.com/ProjectSkyfire/SkyFire_548/issues/1246) Four Valley of Trials NPCs (Vile Familiar, Felstalker, Yarrog Baneshadow, Sarkoth) use a neutral faction instead of hostile
- [ ] [SF#1238](https://github.com/ProjectSkyfire/SkyFire_548/issues/1238) Zanzil the outcast not in the ruins
- [ ] [SF#1232](https://github.com/ProjectSkyfire/SkyFire_548/issues/1232) Osborn Obnoticus 43884 not at Rebel Camp after rescuing him.
- [ ] [SF#1225](https://github.com/ProjectSkyfire/SkyFire_548/issues/1225) Quest: I think she's hungry target not working (Related quest issue inside)
- [ ] [SF#1212](https://github.com/ProjectSkyfire/SkyFire_548/issues/1212) \[Core\] - "DisableMgr.cpp" missing a disable flag in "case MAP_DUNGEON" for the "challenge" difficulty
- [ ] [SF#1206](https://github.com/ProjectSkyfire/SkyFire_548/issues/1206) \[Gameobject\] - Missing Instance Entrance and Exit Portals for "Heart of Fear" Raid
- [ ] [SF#1204](https://github.com/ProjectSkyfire/SkyFire_548/issues/1204) \[Gameobject\] - Jade Serpeant Doors (in and out of the instance) can be closed or opened
- [ ] [SF#1197](https://github.com/ProjectSkyfire/SkyFire_548/issues/1197) \[Zul'Aman\] Gong Event does not start
- [ ] [SF#1017](https://github.com/ProjectSkyfire/SkyFire_548/issues/1017) \[SPELL/WARRIOR/FURY\] Single-Minded Fury
- [ ] [SF#1003](https://github.com/ProjectSkyfire/SkyFire_548/issues/1003) \[SPELL/WARRIOR/TALENT\] Dragon Roar
- [ ] [SF#988](https://github.com/ProjectSkyfire/SkyFire_548/issues/988) \[SPELL/MAGE\] Ice Block
- [ ] [SF#981](https://github.com/ProjectSkyfire/SkyFire_548/issues/981) \[SPELL/MAGE/TALENT\] Flameglow
- [ ] [SF#980](https://github.com/ProjectSkyfire/SkyFire_548/issues/980) \[SPELL/MAGE/TALENT\] Temporal Shield
- [ ] [SF#979](https://github.com/ProjectSkyfire/SkyFire_548/issues/979) \[SPELL/MAGE/TALENT\] Ice Floes
- [ ] [SF#976](https://github.com/ProjectSkyfire/SkyFire_548/issues/976) \[SPELL/MAGE\] Arcane Brilliance
- [ ] [SF#972](https://github.com/ProjectSkyfire/SkyFire_548/issues/972) \[SPELL/MAGE\] Remove Curse
- [ ] [SF#971](https://github.com/ProjectSkyfire/SkyFire_548/issues/971) \[SPELL/MAGE\] Ice Lance
- [ ] [SF#970](https://github.com/ProjectSkyfire/SkyFire_548/issues/970) \[SPELL/MAGE\] Counterspell
- [ ] [SF#940](https://github.com/ProjectSkyfire/SkyFire_548/issues/940) \[SPELL/RACIAL/UNDEAD\] Touch of the Grave
- [ ] [SF#843](https://github.com/ProjectSkyfire/SkyFire_548/issues/843) Goblin Start Zone Quests
- [ ] [SF#842](https://github.com/ProjectSkyfire/SkyFire_548/issues/842) Mulgore Quests
- [ ] [SF#708](https://github.com/ProjectSkyfire/SkyFire_548/issues/708) Trying to fix some spells.
- [ ] [SF#704](https://github.com/ProjectSkyfire/SkyFire_548/issues/704) Generic spell Wrack Soul.
- [ ] [SF#683](https://github.com/ProjectSkyfire/SkyFire_548/issues/683) Fixing Problem of some quests in teldrassil

## Release26 archived world-fix candidates

All296 SQL updates from the pinned release25-to26 archive are indexed
below for compatibility review. Their presence does not authorize blind replay.

- [ ] [2026_07_19_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_19_world_00.sql)
- [ ] [2026_07_19_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_19_world_01.sql)
- [ ] [2026_07_19_world_02.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_19_world_02.sql)
- [ ] [2026_07_19_world_03.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_19_world_03.sql)
- [ ] [2026_07_19_world_04.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_19_world_04.sql)
- [ ] [2026_07_20_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_20_world_00.sql)
- [ ] [2026_07_20_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_20_world_01.sql)
- [ ] [2026_07_20_world_02.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_20_world_02.sql)
- [ ] [2026_07_20_world_03.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_20_world_03.sql)
- [ ] [2026_07_20_world_04.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_20_world_04.sql)
- [ ] [2026_07_20_world_05.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_20_world_05.sql)
- [ ] [2026_07_20_world_06.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_20_world_06.sql)
- [ ] [2026_07_21_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_21_world_00.sql)
- [ ] [2026_07_21_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_21_world_01.sql)
- [ ] [2026_07_22_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_22_world_00.sql)
- [ ] [2026_07_22_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_22_world_01.sql)
- [ ] [2026_07_22_world_02.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_22_world_02.sql)
- [ ] [2026_07_22_world_03.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_22_world_03.sql)
- [ ] [2026_07_23_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_23_world_00.sql)
- [ ] [2026_07_23_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_23_world_01.sql)
- [ ] [2026_07_24_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_24_world_00.sql)
- [ ] [2026_07_24_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_24_world_01.sql)
- [ ] [2026_07_24_world_02.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_24_world_02.sql)
- [ ] [2026_07_24_world_03.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_24_world_03.sql)
- [ ] [2026_07_24_world_04.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_24_world_04.sql)
- [ ] [2026_07_25_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_25_world_00.sql)
- [ ] [2026_07_25_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_25_world_01.sql)
- [ ] [2026_07_25_world_02.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_25_world_02.sql)
- [ ] [2026_07_25_world_03.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_25_world_03.sql)
- [ ] [2026_07_25_world_04.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_25_world_04.sql)
- [ ] [2026_07_25_world_05.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_25_world_05.sql)
- [ ] [2026_07_25_world_06.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_25_world_06.sql)
- [ ] [2026_07_25_world_07.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_25_world_07.sql)
- [ ] [2026_07_26_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_26_world_00.sql)
- [ ] [2026_07_26_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_26_world_01.sql)
- [ ] [2026_07_26_world_02.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_26_world_02.sql)
- [ ] [2026_07_26_world_03.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_26_world_03.sql)
- [ ] [2026_07_26_world_04.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_26_world_04.sql)
- [ ] [2026_07_26_world_05.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_26_world_05.sql)
- [ ] [2026_07_26_world_06.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_26_world_06.sql)
- [ ] [2026_07_26_world_07.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_26_world_07.sql)
- [ ] [2026_07_26_world_08.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_26_world_08.sql)
- [ ] [2026_07_26_world_09.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_26_world_09.sql)
- [ ] [2026_07_26_world_10.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_26_world_10.sql)
- [ ] [2026_07_27_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_00.sql)
- [ ] [2026_07_27_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_01.sql)
- [ ] [2026_07_27_world_02.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_02.sql)
- [ ] [2026_07_27_world_03.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_03.sql)
- [ ] [2026_07_27_world_04.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_04.sql)
- [ ] [2026_07_27_world_05.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_05.sql)
- [ ] [2026_07_27_world_06.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_06.sql)
- [ ] [2026_07_27_world_07.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_07.sql)
- [ ] [2026_07_27_world_08.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_08.sql)
- [ ] [2026_07_27_world_09.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_09.sql)
- [ ] [2026_07_27_world_10.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_10.sql)
- [ ] [2026_07_27_world_11.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_11.sql)
- [ ] [2026_07_27_world_12.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_12.sql)
- [ ] [2026_07_27_world_13.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_13.sql)
- [ ] [2026_07_27_world_14.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_14.sql)
- [ ] [2026_07_27_world_15.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_15.sql)
- [ ] [2026_07_27_world_16.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_16.sql)
- [ ] [2026_07_27_world_17.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_17.sql)
- [ ] [2026_07_27_world_18.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_18.sql)
- [ ] [2026_07_27_world_19.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_19.sql)
- [ ] [2026_07_27_world_20.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_20.sql)
- [ ] [2026_07_27_world_21.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_21.sql)
- [ ] [2026_07_27_world_22.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_22.sql)
- [ ] [2026_07_27_world_23.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_23.sql)
- [ ] [2026_07_27_world_24.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_24.sql)
- [ ] [2026_07_27_world_25.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_25.sql)
- [ ] [2026_07_27_world_26.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_26.sql)
- [ ] [2026_07_27_world_27.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_27.sql)
- [ ] [2026_07_27_world_28.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_28.sql)
- [ ] [2026_07_27_world_29.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_29.sql)
- [ ] [2026_07_27_world_30.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_30.sql)
- [ ] [2026_07_27_world_31.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_31.sql)
- [ ] [2026_07_27_world_32.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_32.sql)
- [ ] [2026_07_27_world_33.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_33.sql)
- [ ] [2026_07_27_world_34.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_34.sql)
- [ ] [2026_07_27_world_35.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_35.sql)
- [ ] [2026_07_27_world_36.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_36.sql)
- [ ] [2026_07_27_world_37.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_37.sql)
- [ ] [2026_07_27_world_38.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_38.sql)
- [ ] [2026_07_27_world_39.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_39.sql)
- [ ] [2026_07_27_world_40.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_40.sql)
- [ ] [2026_07_27_world_41.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_41.sql)
- [ ] [2026_07_27_world_42.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_42.sql)
- [ ] [2026_07_27_world_43.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_43.sql)
- [ ] [2026_07_27_world_44.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_27_world_44.sql)
- [ ] [2026_07_28_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_28_world_00.sql)
- [ ] [2026_07_28_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_28_world_01.sql)
- [ ] [2026_07_28_world_02.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_28_world_02.sql)
- [ ] [2026_07_28_world_03.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_28_world_03.sql)
- [ ] [2026_07_28_world_04.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_28_world_04.sql)
- [ ] [2026_07_28_world_05.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_28_world_05.sql)
- [ ] [2026_07_28_world_06.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_28_world_06.sql)
- [ ] [2026_07_28_world_07.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_28_world_07.sql)
- [ ] [2026_07_29_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_29_world_00.sql)
- [ ] [2026_07_29_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_29_world_01.sql)
- [ ] [2026_07_29_world_02.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_29_world_02.sql)
- [ ] [2026_07_29_world_03.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_29_world_03.sql)
- [ ] [2026_07_29_world_04.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_29_world_04.sql)
- [ ] [2026_07_29_world_05.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_29_world_05.sql)
- [ ] [2026_07_29_world_06.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_29_world_06.sql)
- [ ] [2026_07_29_world_07.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_29_world_07.sql)
- [ ] [2026_07_29_world_08.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_29_world_08.sql)
- [ ] [2026_07_30_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_30_world_00.sql)
- [ ] [2026_07_30_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_30_world_01.sql)
- [ ] [2026_07_30_world_02.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_30_world_02.sql)
- [ ] [2026_07_31_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_31_world_00.sql)
- [ ] [2026_07_31_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_31_world_01.sql)
- [ ] [2026_08_01_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_01_world_00.sql)
- [ ] [2026_08_01_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_01_world_01.sql)
- [ ] [2026_08_02_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_00.sql)
- [ ] [2026_08_02_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_01.sql)
- [ ] [2026_08_02_world_02.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_02.sql)
- [ ] [2026_08_02_world_03.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_03.sql)
- [ ] [2026_08_02_world_04.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_04.sql)
- [ ] [2026_08_02_world_05.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_05.sql)
- [ ] [2026_08_02_world_06.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_06.sql)
- [ ] [2026_08_02_world_07.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_07.sql)
- [ ] [2026_08_02_world_08.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_08.sql)
- [ ] [2026_08_02_world_09.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_09.sql)
- [ ] [2026_08_02_world_10.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_10.sql)
- [ ] [2026_08_02_world_11.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_11.sql)
- [ ] [2026_08_02_world_12.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_12.sql)
- [ ] [2026_08_02_world_13.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_13.sql)
- [ ] [2026_08_02_world_14.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_14.sql)
- [ ] [2026_08_02_world_15.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_15.sql)
- [ ] [2026_08_02_world_16.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_16.sql)
- [ ] [2026_08_02_world_17.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_17.sql)
- [ ] [2026_08_02_world_18.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_18.sql)
- [ ] [2026_08_02_world_19.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_19.sql)
- [ ] [2026_08_02_world_20.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_20.sql)
- [ ] [2026_08_02_world_21.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_21.sql)
- [ ] [2026_08_02_world_22.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_22.sql)
- [ ] [2026_08_02_world_23.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_23.sql)
- [ ] [2026_08_02_world_24.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_24.sql)
- [ ] [2026_08_02_world_25.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_25.sql)
- [ ] [2026_08_02_world_26.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_26.sql)
- [ ] [2026_08_02_world_27.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_27.sql)
- [ ] [2026_08_02_world_28.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_28.sql)
- [ ] [2026_08_02_world_29.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_29.sql)
- [ ] [2026_08_02_world_30.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_30.sql)
- [ ] [2026_08_02_world_31.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_31.sql)
- [ ] [2026_08_02_world_32.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_32.sql)
- [ ] [2026_08_02_world_33.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_33.sql)
- [ ] [2026_08_02_world_34.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_34.sql)
- [ ] [2026_08_02_world_35.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_35.sql)
- [ ] [2026_08_03_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_03_world_00.sql)
- [ ] [2026_08_03_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_03_world_01.sql)
- [ ] [2026_08_03_world_02.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_03_world_02.sql)
- [ ] [2026_08_03_world_03.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_03_world_03.sql)
- [ ] [2026_08_03_world_04.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_03_world_04.sql)
- [ ] [2026_08_03_world_05.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_03_world_05.sql)
- [ ] [2026_08_03_world_06.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_03_world_06.sql)
- [ ] [2026_08_03_world_07.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_03_world_07.sql)
- [ ] [2026_08_03_world_08.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_03_world_08.sql)
- [ ] [2026_08_03_world_09.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_03_world_09.sql)
- [ ] [2026_08_03_world_10.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_03_world_10.sql)
- [ ] [2026_08_03_world_11.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_03_world_11.sql)
- [ ] [2026_08_03_world_12.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_03_world_12.sql)
- [ ] [2026_08_03_world_13.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_03_world_13.sql)
- [ ] [2026_08_03_world_14.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_03_world_14.sql)
- [ ] [2026_08_04_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_00.sql)
- [ ] [2026_08_04_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_01.sql)
- [ ] [2026_08_04_world_02.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_02.sql)
- [ ] [2026_08_04_world_03.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_03.sql)
- [ ] [2026_08_04_world_04.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_04.sql)
- [ ] [2026_08_04_world_05.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_05.sql)
- [ ] [2026_08_04_world_06.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_06.sql)
- [ ] [2026_08_04_world_07.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_07.sql)
- [ ] [2026_08_04_world_08.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_08.sql)
- [ ] [2026_08_04_world_09.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_09.sql)
- [ ] [2026_08_04_world_10.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_10.sql)
- [ ] [2026_08_04_world_11.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_11.sql)
- [ ] [2026_08_04_world_12.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_12.sql)
- [ ] [2026_08_04_world_13.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_13.sql)
- [ ] [2026_08_04_world_14.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_14.sql)
- [ ] [2026_08_04_world_15.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_15.sql)
- [ ] [2026_08_04_world_16.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_16.sql)
- [ ] [2026_08_04_world_17.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_17.sql)
- [ ] [2026_08_04_world_18.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_18.sql)
- [ ] [2026_08_04_world_19.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_19.sql)
- [ ] [2026_08_04_world_20.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_20.sql)
- [ ] [2026_08_04_world_21.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_21.sql)
- [ ] [2026_08_05_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_00.sql)
- [ ] [2026_08_05_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_01.sql)
- [ ] [2026_08_05_world_02.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_02.sql)
- [ ] [2026_08_05_world_03.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_03.sql)
- [ ] [2026_08_05_world_04.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_04.sql)
- [ ] [2026_08_05_world_05.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_05.sql)
- [ ] [2026_08_05_world_06.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_06.sql)
- [ ] [2026_08_05_world_07.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_07.sql)
- [ ] [2026_08_05_world_08.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_08.sql)
- [ ] [2026_08_05_world_09.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_09.sql)
- [ ] [2026_08_05_world_10.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_10.sql)
- [ ] [2026_08_05_world_11.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_11.sql)
- [ ] [2026_08_05_world_12.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_12.sql)
- [ ] [2026_08_05_world_13.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_13.sql)
- [ ] [2026_08_05_world_14.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_14.sql)
- [ ] [2026_08_05_world_15.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_15.sql)
- [ ] [2026_08_05_world_16.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_16.sql)
- [ ] [2026_08_05_world_17.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_17.sql)
- [ ] [2026_08_05_world_18.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_18.sql)
- [ ] [2026_08_05_world_19.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_19.sql)
- [ ] [2026_08_05_world_20.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_20.sql)
- [ ] [2026_08_05_world_21.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_21.sql)
- [ ] [2026_08_05_world_22.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_22.sql)
- [ ] [2026_08_05_world_23.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_23.sql)
- [ ] [2026_08_05_world_24.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_24.sql)
- [ ] [2026_08_05_world_25.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_25.sql)
- [ ] [2026_08_06_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_00.sql)
- [ ] [2026_08_06_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_01.sql)
- [ ] [2026_08_06_world_02.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_02.sql)
- [ ] [2026_08_06_world_03.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_03.sql)
- [ ] [2026_08_06_world_04.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_04.sql)
- [ ] [2026_08_06_world_05.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_05.sql)
- [ ] [2026_08_06_world_06.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_06.sql)
- [ ] [2026_08_06_world_07.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_07.sql)
- [ ] [2026_08_06_world_08.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_08.sql)
- [ ] [2026_08_06_world_09.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_09.sql)
- [ ] [2026_08_06_world_10.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_10.sql)
- [ ] [2026_08_06_world_11.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_11.sql)
- [ ] [2026_08_06_world_12.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_12.sql)
- [ ] [2026_08_06_world_13.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_13.sql)
- [ ] [2026_08_07_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_00.sql)
- [ ] [2026_08_07_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_01.sql)
- [ ] [2026_08_07_world_02.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_02.sql)
- [ ] [2026_08_07_world_03.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_03.sql)
- [ ] [2026_08_07_world_04.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_04.sql)
- [ ] [2026_08_07_world_05.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_05.sql)
- [ ] [2026_08_07_world_06.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_06.sql)
- [ ] [2026_08_07_world_07.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_07.sql)
- [ ] [2026_08_07_world_08.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_08.sql)
- [ ] [2026_08_07_world_09.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_09.sql)
- [ ] [2026_08_07_world_10.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_10.sql)
- [ ] [2026_08_07_world_11.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_11.sql)
- [ ] [2026_08_07_world_12.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_12.sql)
- [ ] [2026_08_07_world_13.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_13.sql)
- [ ] [2026_08_07_world_14.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_14.sql)
- [ ] [2026_08_07_world_15.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_15.sql)
- [ ] [2026_08_07_world_16.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_16.sql)
- [ ] [2026_08_07_world_17.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_17.sql)
- [ ] [2026_08_08_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_08_world_00.sql)
- [ ] [2026_08_08_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_08_world_01.sql)
- [ ] [2026_08_08_world_02.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_08_world_02.sql)
- [ ] [2026_08_08_world_03.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_08_world_03.sql)
- [ ] [2026_08_08_world_04.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_08_world_04.sql)
- [ ] [2026_08_09_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_09_world_00.sql)
- [ ] [2026_08_09_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_09_world_01.sql)
- [ ] [2026_08_09_world_02.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_09_world_02.sql)
- [ ] [2026_08_09_world_03.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_09_world_03.sql)
- [x] [2026_08_09_world_04.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_09_world_04.sql)
- [x] [2026_08_09_world_05.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_09_world_05.sql)
- [ ] [2026_08_09_world_06.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_09_world_06.sql)
- [ ] [2026_08_10_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_10_world_00.sql)
- [ ] [2026_08_10_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_10_world_01.sql)
- [ ] [2026_08_10_world_02.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_10_world_02.sql)
- [ ] [2026_08_10_world_03.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_10_world_03.sql)
- [ ] [2026_08_10_world_04.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_10_world_04.sql)
- [x] [2026_08_11_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_11_world_00.sql)
- [ ] [2026_08_11_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_11_world_01.sql)
- [ ] [2026_08_11_world_02.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_11_world_02.sql)
- [ ] [2026_08_11_world_03.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_11_world_03.sql)
- [ ] [2026_08_11_world_04.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_11_world_04.sql)
- [ ] [2026_08_12_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_12_world_00.sql)
- [ ] [2026_08_12_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_12_world_01.sql)
- [ ] [2026_08_12_world_02.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_12_world_02.sql)
- [ ] [2026_08_12_world_03.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_12_world_03.sql)
- [ ] [2026_08_12_world_04.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_12_world_04.sql)
- [ ] [2026_08_12_world_05.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_12_world_05.sql)
- [ ] [2026_08_12_world_06.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_12_world_06.sql)
- [ ] [2026_08_12_world_07.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_12_world_07.sql)
- [ ] [2026_08_12_world_08.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_12_world_08.sql)
- [ ] [2026_08_12_world_09.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_12_world_09.sql)
- [ ] [2026_08_12_world_10.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_12_world_10.sql)
- [ ] [2026_08_12_world_11.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_12_world_11.sql)
- [ ] [2026_08_13_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_13_world_00.sql)
- [ ] [2026_08_13_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_13_world_01.sql)
- [ ] [2026_08_13_world_02.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_13_world_02.sql)
- [ ] [2026_08_13_world_03.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_13_world_03.sql)
- [ ] [2026_08_13_world_04.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_13_world_04.sql)
- [ ] [2026_08_13_world_05.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_13_world_05.sql)
- [ ] [2026_08_13_world_06.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_13_world_06.sql)
- [ ] [2026_08_13_world_07.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_13_world_07.sql)
- [ ] [2026_08_13_world_08.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_13_world_08.sql)
- [ ] [2026_08_13_world_09.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_13_world_09.sql)
- [ ] [2026_08_14_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_14_world_00.sql)
- [ ] [2026_08_14_world_01.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_14_world_01.sql)
- [ ] [2026_08_14_world_02.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_14_world_02.sql)
- [ ] [2026_08_14_world_03.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_14_world_03.sql)
- [ ] [2026_08_14_world_04.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_14_world_04.sql)
- [ ] [2026_08_14_world_05.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_14_world_05.sql)
- [ ] [2026_08_14_world_06.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_14_world_06.sql)
- [ ] [2026_08_15_world_00.sql](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_15_world_00.sql)

Primary source: [SkyFire releases](https://github.com/ProjectSkyfire/SkyFire_548/releases/tag/sf_db_26), [issue tracker](https://github.com/ProjectSkyfire/SkyFire_548/issues).

November 6 follow-up: SQL33/35 adapts 458 statements from79 release26 sources,
with377 active quest-field contracts and starter AI/credit/charter interactions.
44 source files are complete data selections,35 are subsets; the paired ledger
keeps every exclusion explicit. Bloodfang35118/quest14272 additionally receives a
local native missing-handler repair and three guarded duplicate-spawn removals.
See release26-quests-worgen-2026-11-06.md. Natural client acceptance remains open;
this is not a blanket all-SkyFire-fixes-complete claim.

November6 services/class continuation: SQL36–39 and four native ports implement
reviewed portions of77 more source files (nine overlap the prior quest/text batch).
Across those two batches147 distinct archive files have contributed changes or
retained matching data. Final2410 data checks pass; four native class ports are
built/loaded and all19 CTests pass. Natural gameplay and remaining native/complex
quest implementations stay open. See release26-services-class-repairs-2026-11-06.md.
