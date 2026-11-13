# WoW / MoP Playerbot Human-Like Name Pool (20,000 names)

## What this contains

This pack is a **candidate pool**, not a SQL migration or a change to your emulator. It contains 20,000 **case-insensitively unique** character-name candidates assembled from a curated vocabulary of real first names, short handles, ordinary words, MMO slang, jokes, class references, and occasional race references. This is **synthetic name generation**, not 20,000 individually hand-reviewed or scraped real players' names. Review the style before importing live.

### Key files

| File | Purpose |
|---|---|
| `playerbot_names.csv` | Canonical full pool, with `name`, category, quality tier, optional gender/class/race hints and selection weight |
| `playerbot_names.txt` | Only names, one per line, for a simple importer |
| `preview_5000_assignments.csv` | **Simulation only**: 5,000 case-insensitively unique names with randomized example metadata; not authoritative WoW race/class combinations |
| `review_200_names.csv` | A representative, editable 200-name review shortlist (`decision` and `notes` columns) |
| `validate_and_preview.py` | Format/uniqueness tests, optional existing-name collision exclusion, and 5,000-bot assignment simulation |
| `build_name_pack.py` | Rebuild the pool from named vocabularies using Python standard library |
| `seed_names.json` | Plain given-name/surname inputs to the generator |
| `name_stats.json` | Total counts and category/hint distribution |
| `qa_preview_results.json` | Results from simulated 5,000-bot name assignment |
| `CODEX_HANDOFF.md` | Integration instructions for Codex, including real QA and rollback requirements |
| `PREVIOUS_HANDOFF.md` | Earlier project directions retained for reference |

## Important naming rules

The names have conservative **single-name 2–12 alphabetic ASCII characters**; first letter uppercase, all following letters lowercase. No digits, punctuation, accents, spaces, or camel-case. This is intended for the classic/MoP character-name model, not the new two-part naming feature announced for future retail WoW. Actual emulator-side validation is still authoritative.

**Category tags are optional hints, not restrictions**. A female avatar sometimes has a feminine name; most names remain neutral. A rogue sometimes receives a rogue pun, but many rogues will just have names such as `Kevin`, `Socks`, or `Sillypanda`.

## Run local validation and preview

```bash
cd wow_playerbot_namepack
python3 validate_and_preview.py --simulate 5000
```

Exclude names already in a realm by exporting current character names into either a one-name-per-line TXT or a CSV containing a `name` column:

```bash
python3 validate_and_preview.py --existing existing_character_names.txt --simulate 5000
```

This tests the file and models assignment. It does **not** prove that the server can *create, store, or keep online* 5,000 bots. Integration tests on the **already running QA server** are mandatory. Never point a script from this pack directly at production databases.

## How Codex should integrate

1. Inspect the actual bot-creation code, naming fallback, name table schema, and character table's uniqueness/collation before edits. Do not assume AzerothCore schema or drop this CSV into a random SQL table unchanged.
2. Store/import the candidate pool without overwriting existing human or bot character names. Reserve every already-used realm name; match names case-insensitively.
3. Choose without replacement *per realm*. Prefer quality A/B names, using the optional hints occasionally, not deterministically. No numeric or gibberish fallback when one category is exhausted.
4. On the existing QA instance, validate bot creation at 5,000 **stored characters** (including collision/restart tests) with controlled batching. A 5,000-concurrently-online test is a *separate load and performance objective*.
5. Do not rename existing bot characters without a separately enabled, backed-up migration with rollback.

See `CODEX_HANDOFF.md` for the full handoff.
