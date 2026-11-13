# Features and improvements

**MoP Playerbots — cumulative handoff, November 12, 2026**

This is the plain-language companion to `changelog.md`. It explains what the
work adds for players and server owners, without listing C++ changes or SQL
commands. The full changed files, installation notes and database updates are
included in this handoff.

This is a substantial set of improvements to the LoA Pandaria playerbot server.
It is still a project in development. “Implemented” means the change is in the
delivered files; it does not mean every possible situation has been tested.
Known limits and remaining work are listed at the end.

## Latest follow-up: less noise and less repetitive chat

- Fixed69 incorrectly configured NPC animation commands, including Hilda's repeated emote error.
- Repaired linked actions that caused repeated NPC script errors.
- Restored missing authored dialogue for affected NPCs while preserving custom text.
- Repaired an old quest reward that requested a missing spell.
- Prevented empty learning requests from trying to add spell zero.
- Restored four missing Gilneas graveyard links.
- Public bot chat now shares a30–60-second channel gap and a five-minute per-bot gap.
- Repeated opening sentences are blocked across bots for30minutes.
- General and dungeon party conversation now uses115 standalone lines instead of adding generic endings to the same sentence.
- Public quest play-by-play and low-quality loot narration are switched off.
- Human chat, ordinary NPC dialogue and error logging are unaffected by those chat limits.

The large earlier chat library remains part of the delivered work. These newer
settings and social hooks supersede its original general-chat presentation.
Remaining unrelated content errors stay visible and documented.

## At a glance

- A working BattlePay store with account balances, purchases and clearer failure handling.
- A redeemable Orb of Loyalty that adds 100 Battle Shop Coins to the account.
- Auction listings and purchases spread across many bot names.
- A curated name library and a persistent roster of 5,000 bots in the current environment.
- A current target of 550 bots online, with controls to tune the population.
- Player-requested party companions and support for the Playersbots addon.
- Quiet party companions that do not whisper every routine action.
- A much larger public chat library, including 10,000 additional conversational variants.
- More controls for human-led dungeon groups, including replacing unavailable bots.
- Coin-only companion looting and automatic passing on item rolls.
- Numerous starting-zone, Worgen, quest, vehicle and interaction repairs.
- Gate of the Setting Sun crash and missing-object repairs.
- Working Grand Expedition Yak merchant and reforger services.
- Better crash investigation, database compatibility, packaging and repeatable QA.
- A documented handoff with complete changed files and clean database updates.

## BattlePay: account currency and the shop

### What players gain

- The shop reads the balance belonging to the logged-in account.
- Stored donation points and displayed shop coins use the same understandable units.
- The balance tile no longer shows the confusing crossed-out zero beside the balance.
- Purchases that a player can afford deduct the appropriate amount and use the delivery system.
- Insufficient funds produce the client's native failure result instead of leaving the shop stuck connecting.
- Closing an affordable purchase confirmation no longer locks the shop for roughly five minutes.
- Players can start another purchase after canceling a confirmation.
- Existing shop products and customized catalogue work are preserved.

### What server owners gain

- Better protection against charging twice or delivering twice after a repeated request.
- Saved purchase and delivery records to support recovery and investigation.
- Checks that prevent negative balances, invalid prices and currency overflow.
- A clearer distinction between an account's coins and the shop's internal price formatting.
- An optional clean copy of the static shop catalogue for maintainer review.
- Database updates that do not reset existing player balances.

The shop's central purchase, balance, insufficient-funds and cancellation flows
were exercised during development. This does not certify every catalogue
product, service or unusual delivery situation.

## Orb of Loyalty: redeemable Battle Shop Coins

The existing Orb of Loyalty is now a currency voucher.

- Each right-click consumes one orb and adds **100 Battle Shop Coins** to the account.
- The coins belong to the account, so they are available to its characters through the shop.
- The item is **Legendary** quality.
- It is **soulbound**, preventing normal trading, mailing or auctioning.
- It has no vendor sale value.
- It stacks to **20** per stack.
- Its description reads: **“Adds currency to your account of 100 Battle Shop Coins.”**
- The shop balance refreshes after a successful redemption.
- Replaying a request for an already-consumed orb does not produce another credit.
- If the account cannot receive more coins, the orb is retained.
- The currency credit and item consumption succeed together rather than risking one without the other.
- A database failure does not lead to a guessed extra item or duplicate credit.
- Old cached item details are marked for refresh by the client.

Automated checks covered real item use, stack splitting, the final orb in a
stack, two characters sharing one wallet, repeated packets, overflow and a
deliberately forced database failure. The server's item-information replies
also carried the new quality, stack size and description.

The requested cleanup removed old orb items while preserving the characters.
That owner-specific cleanup is **not** included as a general-purpose SQL script
for another server to run.

### How a GM gives a soulbound Orb of Loyalty

Select the intended online player, confirm their name is your selected target,
then use `.additem 35626 1`. Use `.additem 35626 20` for a full stack. It goes
directly into that player's bags, so no trade is needed. Each orb adds100coins
when redeemed. The recipient needs bag space; your GM account needs permission
for .additem. Without a suitable player selected, the command can give the item
to the GM instead. Do not append the recipient's name to this command.

## Auction houses that look busier

- New simulated auctions can appear under many existing playerbot names.
- Bots can appear as both buyers and sellers.
- The same bot identity can participate in more than one auction.
- Faction auction houses use appropriate faction characters.
- Neutral auction houses can draw from both sides.
- The auction engine can bid on or buy eligible player listings using its configured pricing rules.
- Bot-owned stock is recognized so the simulation does not simply buy its own listings as player goods.
- Old single-owner Auctionbot listings can be retired gradually.
- Listings with player bids are protected during that cleanup.
- Broken-item name filters exclude TEST, DEPRECATED, DEBUG, UNUSED and similar unsuitable entries.
- Items requiring level 100 or above are excluded under the chosen broken-item convention.
- Legitimate high item-level Pandaria gear is not excluded merely because its item level is high.
- Existing buyer, seller, price and stock controls remain available.

The auction engine creates and manages the simulated market activity. A bot
name on an auction does **not** mean that character physically traveled to an
auctioneer and listed an item from its bags. City visits and auction accounting
are related atmosphere features, but they are separate systems.

## A larger, more consistent bot population

- A supplied library of **20,000 candidate names** is included with validation tools.
- The current environment has a persistent roster of **5,000 managed bots**.
- The roster uses **455 accounts**, respecting the 11-character-per-account limit.
- The earlier managed bots were renamed without replacing their character identities.
- Additional bots were created to reach the new roster size.
- Existing ordinary-player identities were protected during the migration.
- Names are reserved so two simultaneous requests cannot claim the same candidate.
- The system handles a depleted name pool explicitly rather than quietly inventing unwanted names.
- Population creation can resume after interruption.
- Restarting does not recreate the completed roster or change its assigned names.
- Race, class and gender hints can guide naming where supplied.
- The current configuration targets **550 online random bots**.
- Online population can be tuned independently of the total stored roster.

A 550 target is not a promise that exactly 550 bots are online every second.
Login scheduling, availability and server activity affect the live count.
Population ramps and short observations were completed; a long combat-heavy
soak is still needed.

## Player-requested companions and the addon

- The server understands the Playersbots addon's ordinary party-command requests.
- Players can request role-appropriate tank, healer and damage companions.
- Party and raid requests use the existing group system and its limits.
- Companions can be initialized for the requesting player's level and role.
- Attack, follow, hold and related controls are supported through the command paths.
- The companion summon button can use the supported bot summon command without giving a normal player GM summon rights.
- A small addon patching tool is included for the relevant summon-button change.
- Server owners can restrict companion access by player level.
- Access can also depend on account permission level.
- GM exceptions and maximum companion counts are configurable.
- Pending companion requests count toward the limit, helping prevent repeated-click overfilling.
- Ownership checks prevent players from freely commanding another player's characters.
- Companion creation and teleport status handling were improved to address the reported 1-health/offline appearance.

The default access policy described in the feature notes starts ordinary
companion use at level 60, with separate administrator controls. Review the
actual supplied configuration when choosing the policy for a new server.
PvP request buttons currently use role-appropriate companions; they do not add
a separate fully tuned PvP equipment system.

## Less annoying party communication

- Party companions no longer need to whisper hello and goodbye.
- Accepting a quest does not need to produce four separate companion whispers.
- Routine quest-completion and AI status narration is quiet by default.
- Explicit commands can still receive useful replies.
- The addon can still exchange its required command responses.
- Quiet companion behavior is scoped to companions grouped with a real player.
- Public ambient bot conversation stays enabled.
- Optional dungeon party flavor uses party chat instead of routine whispers.
- That flavor is shared across the companions: it is not a separate chatter timer for every party member.
- Its default interval is five minutes, with a one-minute minimum setting.
- Flavor can be disabled independently of the ordinary companion controls.

A real four-companion test confirmed quiet invitations and quest sharing.
A selected roster bot also answered an explicit position request while quest
sharing immediately afterward remained quiet.

## More believable public conversation

- **10,000 additional distinct chat variants** are supplied.
- General conversation includes ordinary Azeroth banter, bag space, mounts, professions, auction browsing and other everyday topics.
- Gathering lines discuss materials and farming routes.
- Sale offers use the actual item and price supplied by the bot's trade action.
- Sale wording avoids pretending that an inventory offer is already a posted auction.
- Dungeon lines support group finding and conversation about runs.
- Quest lines use the actual quest link supplied by the activity.
- Reputation lines discuss the faction relevant to that action.
- Loot lines use the item the activity supplies.
- Dungeon party lines include pacing, patrols, repair bills, getting lost and other light group banter.
- The larger library does not raise the existing public-chat event frequencies.
- The pack retains the original and customized chat entries.
- Reapplying the update does not duplicate all the lines.

The 10,000 variants combine contextual openers with conversational follow-up
phrases. They are not 10,000 individually handwritten stories, and they do not
form a conversation-aware chatbot. Actual tone and repetition should still be
reviewed during longer play sessions.

### Broken punctuation repaired

- The strange characters shown in the screenshots came from damaged stored punctuation.
- Known damaged apostrophes, quotes and similar punctuation are repaired.
- The new English pack uses plain punctuation to avoid that import problem.
- The loader also repairs those specific known sequences in future imports.
- Genuine foreign-language letters are preserved rather than stripped out.
- Localized text columns remain available.

The repeated import check finished with **11,707 total chat entries**, including
the 10,000 new variants, without duplicate growth.

## Human-led dungeon groups and loot

- A real player retains leadership in a party filled with companions.
- Unavailable bots can be removed so the player can replace them.
- Online companion removal and offline companion removal were exercised.
- Bot-only companion groups can avoid an unnecessary vote deadlock when replacing a bot.
- Companion vote behavior is aligned with the human-initiated vote where that path applies.
- Mixed-human groups retain their own voting rules rather than becoming a one-player override.
- Human-grouped companions are configured to collect only ordinary coin money from corpses.
- They skip item and currency-token loot slots.
- They automatically pass on item rolls.
- They are excluded from ordinary rotating item ownership under the companion policy.
- Bot-only autonomous groups retain their separate progression behavior.
- A `.partyloot` command gives the eligible human-led companion group a server-side loot control.

The retail Dungeon Finder loot dropdown can still remain grey. The added
server command is the supported control; this work does not rewrite that client
interface. Broader testing of real loot-roll outcomes and mixed-human votes
remains on the list.

## City activity and world atmosphere

- Added city-service visit behavior for eligible ungrouped bots.
- Bots can choose auctioneers, banks, vendors and other service stops.
- Stops can vary rather than immediately repeating the same destination.
- Pauses vary to reduce mechanical-looking timing.
- Destination reservations limit how many bots choose the same service target.
- Unreachable or stale targets can be abandoned instead of held forever.
- The larger named roster supplies more identities for world and auction activity.
- Public conversation and market activity remain independently configurable.

This is not a guarantee that every city is populated or that crowding is solved.
Bot distribution, navigation and local NPC data still affect what players see.
Overlapping bots at banks, flying-city NPC routes and long-distance following
remain areas that need further attention.

## Starting zones and quest progression

The project includes many matched code-and-data repairs, rather than only
moving a stuck character or marking a quest complete. Examples include:

- **The Disciple's Challenge:** Jaomin Ro's attackability transition and fight credit.
- **Master Shang Xi:** bridge visibility and quest handoff behavior.
- **Undead starting area:** inappropriate recurring Rigor Mortis application.
- **Early Undead progression:** missing starter interaction and questgiver data.
- **Practice Eviscerate:** Worgen rogue training progression.
- **Worgen introductory events:** wave, actor and quest-support repairs.
- **Gilneas horse quests:** boarding and quest action support.
- **Bite/cage progression:** control-state, terrain and animation-related repairs.
- **Save the Children!:** rescued-child interaction work.
- **To Greymane Manor:** horse travel and retry support.
- **The Battle for Gilneas City:** event start/retry, supporting actors and progression work.
- **Gorerot:** encounter location, respawn and combat-related repairs.
- **Slowing the Inevitable:** captured bat phasing, boarding, bombing and return.
- **Knee-Deep:** Half Burnt Torch interaction repairs.
- **They Have Allies, But So Do We:** supporting vehicle and encounter data.
- **Endgame:** hippogryph, gunship, rope and return-trip repairs.
- **Gilneas departure:** Admiral Nightwind continuation and travel handoff.
- **The Howling Oak:** quest turn-in and nearby service interactions.

These are specific repair areas. They are **not** a claim that every quest in
every race/class storyline now works. The detailed documents record which
steps were accepted in the client, which were checked automatically and which
still need follow-up.

## Vehicles, travel and client crashes

- Repaired a client crash associated with invalid animation-tier information.
- Repaired the Endgame gunship model/type handling implicated in the reported crash.
- Removed the overlapping duplicate gunship in that encounter.
- Repaired the rope landing so it places the player on the ship instead of outside the map.
- Improved camera/control restoration after the return flight.
- Repaired vehicle service and encounter information used by the Gilneas finale.
- Added safer handling of missing controllers and empty target lists in relevant dungeon logic.
- Kept matching crash reports, executable versions and symbols for investigation.
- Applied a narrowly scoped navigation crash guard with its limits documented.
- Repaired reviewed movement-map coverage problems and retained the remaining movement issues on the checklist.

These fixes target known faults. They do not establish that every vehicle,
cinematic, flying NPC or movement path is now free of problems.

## Gate of the Setting Sun

- Restored the missing dungeon controller binding.
- Restored 80 missing authored dungeon objects and their relevant supporting data.
- Restored five missing area-trigger connections.
- Added protection against the missing-controller crash in bombardier behavior.
- Added protection against selecting from an empty target list.
- Verified the first door's trigger-driven state change.
- Verified lift information through the running server.
- Checked the relevant map/navigation data at the inspected locations.

A continuous rendered playthrough of the missing-ground route is still needed.
The restored objects and crash guard are not a substitute for that visual check.

## Vendors, bank and the Grand Expedition Yak

- Restored vendor data for the relevant Keel Harbor service NPC.
- Repaired Tarien Silverdew's interaction path.
- Repaired Bank Teller Idriana's banking interaction.
- Restored **14 merchant entries** for Cousin Slowhands on the Yak.
- Restored Mystic Birdhat's **5.4.8 reforging service**.
- Tested the actual mounted Yak passengers, rather than spawning unrelated stand-in vendors.
- Received the expected merchant list and a successful reforging reply.

Birdhat is a reforger in this version of the game. This change does not add a
later-expansion transmogrification service.

## Mail, account state and authentication

- Restore saved account achievement state before normal player login.
- Apply the corresponding account-state loading to bot sessions.
- Prevent the known repeated-login achievement reward replay that caused repeated mail.
- Correct a mail-status field read that could cause invalid saved values.
- Preserve achievement history rather than resetting it to hide repeated rewards.
- Keep account-specific mailbox cleanup separate from reusable fixes.
- Preserve username-based login behavior for the established account system.
- Add real authentication regression coverage.
- Add quieter auth logging with useful loading/ready information.
- Keep detailed failure information in diagnostic files.

Repeated-login mail behavior was accepted during development. This is not a
proof that every possible mail source has exactly-once delivery through every
crash scenario.

## Easier to run and maintain

- One normal authserver/worldserver pair for the active environment.
- Visible server consoles through the supplied launchers.
- Reuse an already-running supported pair instead of opening another set.
- Guards against accidentally running competing worldservers against the same realm.
- Relative data and log locations for a more portable folder layout.
- Practical MySQL/MariaDB compatibility and matching connector guidance.
- Configuration switches for the principal companion and population policies.
- Build/deployment records that identify the actual installed files.
- Matching executable/debug-symbol pairs retained privately for crash analysis.
- Existing warnings and errors remain available for investigation.
- Diagnostic logging can be quieter without claiming unresolved faults were fixed.

The handoff provides source and build instructions. It is not a separately
validated, ready-made Linux binary distribution; Linux packaging remains
additional work.

## QA without keeping an agent watching

- Scripts can connect to the real server and exercise reviewed gameplay interactions.
- Tests can use disposable accounts and characters.
- Fixture ownership and cleanup reduce the risk of touching unrelated saves.
- Reports can be saved for later review and repairs.
- Checks cover selected authentication, creation, quests, rewards, interactions, groups, vehicles and item use.
- Offline checks can inspect content references and identify missing relationships.
- Transaction tests cover repeated requests and deliberately forced failures.
- The simplified QA workflow uses the same normal server pair.
- The retired observer experiment is not needed to run the reviewed checks.

This is a repeatable QA toolkit, not a proven bot that naturally plays the entire
game. The abandoned circling/under-map observer behavior is not presented as
successful gameplay testing. Database comparisons and scripted contracts help
find gaps; they cannot verify all story expectations or visual behavior alone.

## What is included for the next maintainer

- Complete changed source files at their original project paths.
- A review patch against the pinned upstream base.
- Clean database schemas and feature/content migrations.
- The optional static BattlePay catalogue, separated from player wallets.
- Curated naming data and import/validation tools.
- The expanded chat pack, generator and validator.
- Focused automated regressions and QA tooling.
- Configuration templates and the owner's requested working configurations.
- The large technical `changelog.md` and this plain-language `FEATURES.md`.
- Feature notes, known issues, merge guidance and migration instructions.
- File inventories and checksums for package verification.
- A privacy report identifying the explicit private-config exception.

**Before public sharing:** remove the three working files in `config/*.conf`.
They contain the owner's requested database credentials. The sanitized source
configuration templates are the ones intended for public review. Player saves,
account rows, wallet balances, mail, transaction receipts, crash dumps and
compiled executables are not included in the clean source handoff.

## What still needs work

- More complete playthroughs of starting zones and later quest chains.
- Longer, combat-heavy testing with the 550-bot target.
- Broader dungeon and raid mechanics coverage.
- A rendered Gate of the Setting Sun route test.
- Additional checks of loot rolls and voting with multiple real players.
- Longer observation of the new chat pack and dungeon party flavor.
- Better bot distribution, crowd spacing and recovery from bad paths.
- Flying NPC movement and other unresolved movement behavior.
- Remaining original-world missing text, SmartAI, graveyard, inventory and quest diagnostics.
- More rendered-client verification for vehicles, service windows and cinematics.
- Further Linux packaging and validation.

The goal remains a more stable, playable and maintainable server. The files
explain the progress and the remaining work so the next maintainer can continue
without having to rediscover the history.


## Dalaran and auction scanning

Dalaran now has a configurable visiting population, with walking trips to usable
neutral services, varied pauses, destination crowd limits and faction/floor
checks. Human-controlled companion parties are excluded from visitor selection.

Auction listings now arrive with their cached seller names first, removing the
missing-name prerequisite that makes Auctioneer retry incomplete pages. Native
scans retrieved about6600 listings in45seconds, or0.3seconds using get-all. The
addon's full rendered scan duration still needs an in-game acceptance measurement.
The repeatable QA scripts and exact evidence are documented in
dalaran-auction-followup-2026-11-12.md.

Connection settings are now grouped near the top of both working server configs,
including the playerbots database. Values and thread counts were preserved.
The detailed acceptance checklist is next-city-auction-and-admin-followup.md.
