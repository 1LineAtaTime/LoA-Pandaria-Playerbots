# Bot chat pack and punctuation repair — November 12

**Follow-up:** runtime-noise-and-social-chat-2026-11-12.md supersedes the
main general/party hook selection and public chat frequency described below.
The original10000 variants remain delivered, but general/party social hooks now
use115 standalone lines with shared rate/repeat limits.

Public ambient General/Trade/Guild/LFG chatter remains enabled. Quiet companion
policy applies to a bot sharing a real player's group; it does not globally
silence public bots. Routine greetings and quest accepted/completed whispers
remain disabled. Explicit commands and addon replies retain their response path.
The real-roster four-companion regression verified explicit position replies
and quiet quest sharing after invitation.

## Stored punctuation defect

The imported English text contained UTF-8 punctuation decoded as CP850: for
example U+2019 became the three characters U+00D4 U+00C7 U+00D6. This was stored
data damage, rather than evidence that the client cannot display foreign names.
363 English entries were affected. The data migration repairs exact known
CP850/CP1252/CP437 punctuation sequences and normalizes smart punctuation to
ASCII. It processes the English and eight localized columns; it does not
re-decode entire strings or discard genuine non-Latin letters. The native
text loader applies the same bounded repair to future imports.

## Content and routing

The JSON pack contains 10,000 distinct variants: ten categories, each with20
contextual openers combined with50 conversational codas. These are authored
clause combinations, not10,000 independently handwritten speeches. The pack
uses plain English punctuation and5.4.8-era references, with no URLs, account
identities, credentials, hardcoded item IDs or private chat transcripts.

Nine categories extend existing runtime hooks: general suggestions, gathering,
selling, dungeon groups, quest groups, reputation, accepted/turned-in quests
and ordinary loot. Substitutions come from the actual calling hook. Selling
uses its real item/count/price; the gathering category asks about farming rather
than claiming a nonexistent active buy order. There are no invented spell or
quest links. Old entries remain available; relative selection weights change
within a category while its event probability remains unchanged.

The tenth category is occasional dungeon party flavor. Only human-controlled
companions in the same dungeon group qualify. The generic suggestion action
now permits that specific path; trade/dungeon recruiting actions remain
ineligible for grouped companions. Flavor replaces a public suggestion and is
shared across the human leader's companions, with a minimum60-second interval.
The default is300seconds, not one message per bot. Native SayToParty was a
no-op stub; it now sends an actual party packet through Group::BroadcastPacket.
This does not route routine status narration into party chat.

AiPlayerbot.Companion.PartyFlavor =1
AiPlayerbot.Companion.PartyFlavorInterval =300
AiPlayerbot.Companion.RoutineMessages =0
AiPlayerbot.Companion.SilentGreetings =1

## Reusable SQL and verification

Apply these to the configured playerbots schema, not world/auth/characters:

1. modules/mod_playerbots/data/sql/playerbots/2026_11_12_00_chat_punctuation.sql
2. modules/mod_playerbots/data/sql/playerbots/2026_11_12_01_ambient_chat_pack.sql

The pack uses temporary SHA-256 identity keys for efficient exact-byte
name/text deduplication, without adding a permanent index or forcing IDs. Its
10,000 rows fit the current signed SMALLINT id capacity with the1,707 original
rows. A much larger future pack must review that capacity. Existing localized
rows and customized text are retained. Restart worldserver to refresh the
native text cache. No account or character table is modified.

Run python contrib/bot-chat/validate_chat_pack.py offline. Regenerate with
contrib/bot-chat/generate_chat_pack.py; deterministic JSON/SQL are included.
The local import was repeated:11,707 total rows,10,000 new variants, no duplicate
increase and no remaining known mojibake in English. All variants have unique
(name,text), valid hook-specific substitutions, ASCII punctuation and at most
111 pre-substitution characters. A private all-locale preimage was retained.
The first idempotence probe exposed a slow unindexed comparison; the delivered
SQL uses a temporary indexed digest set and passed both repeated imports.

Native build success and import integrity do not prove every rendered line or
that a specific idle strategy will select dungeon flavor in every situation.
Keep actual public-chat/party observations separate from static validation.
Rollback restores the private text-table snapshot or removes only pack rows
by exact name/text; never truncate a maintainer's text table. Restore prior
native files/config for code rollback. No logger threshold was raised.
