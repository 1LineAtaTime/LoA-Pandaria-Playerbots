# Dalaran city activity and Auctioneer delivery

## Changes

The city service selector previously rejected neutral NPCs by requiring a friendly
reaction. Dalaran's usable neutral vendors and bankers now qualify; hostile NPCs
still do not. Dalaran also rejects services on another floor, without line of
sight, or in the opposite faction's restricted district. Existing limits of three
visitors per destination, previous-destination avoidance, varied pauses and
path recovery remain in place. A city without an available service uses a short
wander rather than repeatedly returning to idle.

The random-bot manager gradually supplies a configurable surface population:
`AiPlayerbot.CityLife.DalaranTarget = 24` and
`AiPlayerbot.CityLife.VisitSeconds = 1200`. Only eligible idle, ungrouped level70+
random bots participate. Human companions, combat, queues, instances, transports,
flights and QA scenarios are excluded. Visits keep normal detailed AI active and
cycle through city services. Arrivals use existing checked relocation at authored
neutral service coordinates, reject a floor more than four yards below/above the
hint, and avoid occupied arrival points. Admissions are paced at most one per
15seconds. Departures wait while a real player is nearby. Set the target to zero
to disable admissions; the target is not an extra global login budget.

Auction searches already use the in-memory auction collection. Increasing SQL
connections or worker threads would not repair a client cache wait. Auctioneer
5.19.5445 waits for seller names when `core.scan.sellernamedelay` is enabled and
retries incomplete records at one-second intervals. The server now sends cached
seller-name replies before the corresponding auction list. Owner names are
deduplicated per response. There are no new SQL queries or mutations, and normal
query delays and get-all cooldowns remain enforced. Set
`Auction.PrefetchSellerNames = 0` to revert this behavior independently.

## Measurements and limits

The original delivery baseline retrieved6642 auctions over133 pages in45.263s.
With name prefetch, the same6642 listings arrived in45.188s and all1241 unique
seller names were present before listing processing. Separate bounded cold name
queries resolved every owner. A get-all request returned6641 listings in0.292s;
large compressed response framing also passed. Auction counts can change during
normal trading. These are native protocol measurements, not a rendered Auctioneer
UI stopwatch result. The reported seven-minute addon scan still needs a rendered
acceptance run; no claim is made that its final duration has been measured.

The first city census deliberately failed acceptance:7 to18 surface bots,
9 moving more than25yd and zero confirmed service visits. This exposed the
neutral-NPC filter and unsafe guessed street anchors; those were repaired before
the final build. The final census result is recorded below.

The final360-second census passed:24 surface bots at completion,12 bots moved
more than25yd,19 reached a service within7yd, and22 distinct services were
observed. No floor offsets above5yd were measured. This was a natural activity
sample, not forced scripted visits. It is not a long soak or a guarantee that
every city path is correct.

## Repeating QA offline

Run against the normal local auth/world pair, using the existing original
`mop_` databases and the working exports configuration:

```
python contrib/headless-client/auction_scan_benchmark.py --label repeat
python contrib/headless-client/auction_scan_benchmark.py --get-all --label getall-repeat
python contrib/headless-client/dalaran_city_census.py --seconds 360 --label repeat
```

Both scripts own and clean their temporary account and character; they do not
complete user quests, trade auctions or change user balances. The city fixture
stays in another capital and uses the privileged read-only `citycensus` diagnostic
so a nearby observer cannot force activity. Raw local reports under
`out/city-auction-20261112` are private and excluded from the handoff.

No SQL migration is needed for this batch. Existing clean SQL remains in the
handoff. No warning/error logger was disabled. Original-world content diagnostics
remain separate work; this batch does not claim to fix every quest or movement
issue.


## Repository check

The implementation milestone is ae776850b5. Both main branches matched after
pushing the authoritative source; both defaults are main. The historical mirror
checker stopped on its privacy precondition: the source repository currently
reports public visibility, while the backup reports private visibility. No
visibility settings were changed. Matching current branch hashes is verified;
the full older all-private mirror check is not reported as passed.
