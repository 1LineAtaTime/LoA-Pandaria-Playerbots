# Read-only Worgen event check — November 8, 2026

No character, quest, database, configuration, binary or server-process changes
were made for this investigation. The user requested checking before repair.

The inspected level 3 Worgen is on incomplete quest 14154, By the Skin of His Teeth,
in Gilneas City. Human form is expected at this point: the bite occurs later in
The Rebel Lord's Arsenal, and the full transformation follows Last Stand. Control
over Two Forms is awarded later at Neither Human Nor Beast 24593. References:
[Worgen story](https://www.wowhead.com/guide/story-worgen-starting-area-lore),
[Two Forms quest](https://www.wowhead.com/mop-classic/quest=24593/neither-human-nor-beast).
Classic material corroborates progression; it is not wholesale replacement data.

Published original 5.4.8 SkyFire candidates:

- [42163b72](https://github.com/ProjectSkyfire/SkyFire_548/commit/42163b724df3528c5906e58d0d6c9c5fce8b699d):
  Sean Dempsey waves, Crowley acceptance/combat and four rooftop actor scripts.
- [dc61c594](https://github.com/ProjectSkyfire/SkyFire_548/commit/dc61c594e41b8e7737e1c88d0159a9753d21c0a8):
  follow-up completion credit/player tracking for 14154, plus the 14159 bite and
  phase/reveal sequence. The first encounter commit alone is not the complete series.

Current LoA evidence:

- Crowley 35077 uses SmartAI and has only event 64 row 3, the restart interaction;
  no event 19 quest-accept row exists for that template.
- Sean Dempsey 35081 has no named script or AIName. The SF wave-controller class
  is not registered in the current LoA module.
- Templates 35167/35170/35188/35456 name npc_worgen_alpha_c2/c1 and
  npc_worgen_runt_c1/c2; startup logs explicitly report those names unregistered.
- LoA's compiled spell_gen_gilneas_prison_periodic_dummy has no binding in
  spell_script_names. Its alternative spell-driven wave path is disconnected too.
- The later LoA restart/chain SQL files 997/977 exist in the repository; their
  presence is not proof the complete original event is installed.

These are concrete integration gaps consistent with absent waves. The SF source
uses a different event implementation/API; it must be reconciled coherently with
LoA's spell/SmartAI implementation and matching SQL, avoiding duplicate event
controllers. No patch, forced transformation, reload or quest completion was applied.
