# Runtime spell/emote repairs and quieter social chat — November 12

This follow-up addresses the console screenshot without changing WARN/ERROR
logging. Other original-world content errors remain visible and are not claimed
fixed by this batch.

## Actual fault repairs

- Correct69 waypoint emote mode mismatches using the loaded Emotes.dbc type.
  Hilda Stoneforge25235 uses waypoint action1120/emote16: the persistent-state
  flag was wrong for this one-shot animation. Reverse mismatches also previously
  discarded authored persistent animations; their state flag is restored.
- Repair the linked melee-weapon action for Riverpaw Scout500 and Mounted
  Ironforge Mountaineer12996. The linked child had a zero-range event instead
  of LINK; retain the parent range, action and targets.
- Remove invalid cross-links between Sanitron500's independently triggered
  respawn/reset/boarding events. Preserve the separate events; do not convert
  boarding into an unconditional spawn action.
- Restore missing authored dialogue from matching-name LoA reference creatures.
  The migration carries48 candidate rows across31 groups for affected actors;
  existing/custom keys win. It includes all available authored groups for each
  affected actor, avoiding a new missing-group error after restoring group0.
- Correct legacy quest reward8616 (The Affray1719): retain valid Berserker Stance
  learning2458 while removing only its missing Intercept20252 learn effect.
  This does not invent a replacement Intercept or modify ordinary combat spells.
- Generic learning wrappers483/55884 use a supplied base-point spell payload.
  An absent zero payload is not a spellbook entry; return before LearnSpell(0).
  Other invalid nonzero spell requests still reach the existing error checks.

World12 and13 are the clean paired data migrations. Changes were backed up in
ignored out/log-noise-20261112 and applied with the realm stopped. Repeated
application preserved text counts,69 corrected modes and repaired event wiring.
An initial empty-string hex encoding and overstrict linked-row guard were
caught during offline application and corrected in the delivered SQL. Some
world tables are MyISAM; do not assume transaction rollback covers those edits.

## Social chat adjustment

The earlier10000 variants combined20 openers with50 codas per category. The
owner's screenshot demonstrated the visible repetition of those openers. The
main general and dungeon flavor hooks now use115 individually written standalone
lines (90 general,25 party), without appended codas. The earlier library remains
available to its other hooks and as optional source material; do not describe
it as10000 independently written conversations.

Public quest play-by-play and poor/normal loot narration are disabled by default;
quest recruiting and uncommon-loot chatter are reduced. Social conversation,
trade and group finding remain enabled. This is a chat preference change, not
suppression of diagnostic errors.

Shared limits apply to successful bot broadcasts, not human chat or NPC dialogue:

- AiPlayerbot.Chat.ChannelMinInterval =30 seconds
- AiPlayerbot.Chat.ChannelMaxInterval =60 seconds
- AiPlayerbot.Chat.PerBotInterval =300 seconds
- AiPlayerbot.Chat.RepeatWindow =1800 seconds

The per-channel delay is randomized within that interval. Local channels are
scoped to faction and zone; guild/world/LFG use their corresponding broader
scope. A per-bot budget also prevents the same bot switching channels to spam.
The repeat key uses the opening sentence, so a different coda cannot evade it.
Successful sends consume the budget; a missing/unjoined channel does not.
Expired state is pruned, and a mutex protects concurrent map-thread access.
Companion whispers retain the existing quiet/explicit-reply policy.

Apply playerbots2026_11_12_02_standalone_social_chat.sql alongside the new native
hooks. Restart to refresh text caches/config. No accounts, quests, wallet or
inventory state is changed by the chat migration.

## Verification and limitations

The native x64 Visual Studio build completed with zero failed projects. The
standalone C++ policy regression passed channel and per-bot limits, cross-bot
repeat rejection, failed-send accounting, expiry and32-thread contention (one
successful sender). Build/run it with CMake from contrib/bot-chat/tests.
Data migrations passed idempotence and all targeted postconditions. New dialogue
was reloaded through the normal creature_text reload command.

The native reward/empty-payload fixture and subsequent runtime observation are
recorded separately under ignored out/log-noise-20261112. Do not infer gameplay
success merely from a cast command acknowledging that it queued a spell.
Longer rendered-client observation remains needed for animation appearance and
chat tone. Existing missing script implementations, loot references, empty
pools and obsolete saved objective data remain on the backlog.
No claim of zero total server errors is made.


The final spell fixture explicitly unlearned2458, confirmed its absence through
native spellbook inspection, cast8616 and confirmed2458 became known. Casting
an empty483 wrapper produced no invalid spell entry/error;0/20252 were absent
from saved spell rows. Owned fixtures were cleaned. An earlier fixture's direct
character_spell-row assertion did not match the native spellbook result; it is
not presented as evidence of a gameplay failure or durable spell-save coverage.

Four missing Gilneas graveyard links for zone4755 were restored from authored
LoA rows1692..1695, verified against WorldSafeLocs on map654. World14 is additive
and preserves existing keys/factions. Repeated application kept four links;
the ordinary graveyard reload activated the repair without another restart.

The reviewed standalone text files can regenerate their exact idempotent SQL
with contrib/bot-chat/generate_standalone_social.py; uniqueness, ASCII, length
and no-placeholder/control-markup checks run before output.
