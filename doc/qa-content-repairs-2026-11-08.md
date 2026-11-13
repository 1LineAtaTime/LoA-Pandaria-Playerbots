# QA-driven content repairs — November 8

## Repairs

SQL03 restores two missing item templates (68604,72108) and one gameobject
(212375, Iron Belly Spirits) from the original LoA world reference. Every source
field was present in the active schema (140 item fields and 50 gameobject fields).
Existing rows win; importing this patch cannot overwrite an existing template.
The source and backup hashes are in the adjacent JSON report.

These are reference-integrity repairs, not full quest acceptance: the original
item names are incomplete (plants and Tails for the), and there is no current
spawn for gameobject 212375. The associated quest chains remain untested. No
quest requirement or objective count was weakened to make a test pass.

Three creature_addon rows had no owning spawn (564238–564240). They are removed
only while the owner remains absent. Two stationary caster spawns, Dark Shaman
Acolyte 914942 and Gao-Ran Magus 522984, had nonexistent path IDs. Their active
SmartAI contains combat casts and no path-start actions; clear only these unused
path IDs while movement remains stationary and no path exists. This does not
claim to restore patrols or alter their movement mode.

Lord Darius Crowley 35077 and Heart of Arkkoroc 37143 were assigned to absent native
scripts, which disabled their existing SmartAI. The original LoA reference assigns
both to SmartAI with no native script. Restore those exact ownership fields,
guarded by the stale script names. Existing rows implement the prison scene
forcecast after gossip and the aura-triggered player kill credit respectively.
Their event definitions are retained. Live end-to-end scene completion remains
unverified; enabling their handlers is not a complete quest-chain verdict.

## Safeguards and validation

Before mutation, item_template, gameobject_template and creature_addon were dumped
to ignored out/qa-content-repairs-20261108/before.sql; creature_template has a
separate template-before.sql. No account, character or shop catalog was replaced.
The second patch application changed no affected-table checksums.

All five new data checks and 85 existing Gilneas progression assertions passed.
The audit improved from four passing/six failing categories to six passing/four
failing categories. Remaining findings: two missing creature objective targets,
seven missing item targets, one missing gameobject target, and 60 SmartAI ownership
conflicts. These require further source/content review, not blanket deletion.

Worldserver shut down normally and relaunched through its visible export launcher;
authserver remained available. No C++ change or rebuild was required. Errors and
warnings stay enabled. Gameplay regression results follow after the run completes.

## Final regression result

All 15 supported gameplay scenarios passed after restart; 8 capability gaps remain
untested. All 15 temporary accounts and characters were verified removed. Comparing
against the prior full run with identical scenario expectations found zero
regressions or increases in structural violation counts. The normal nonzero QA
exit is due to the four remaining failed content audits.

A bounded startup-log comparison recorded 36 resolved SQL signatures and zero new
signatures (1,319 archived versus 1,283 current). It compares an archived runtime
window with the new startup sample; it is not evidence of full functional coverage.
Both exports servers are listening on their normal ports. The executable/PDB
pair remains the previously verified native build; this batch changes SQL only.
