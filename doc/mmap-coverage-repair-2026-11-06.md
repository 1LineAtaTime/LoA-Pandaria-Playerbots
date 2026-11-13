# Navigation coverage repair — November 6, 2026

The earlier single-tile fix did not establish complete coverage. This batch scans
missing terrain/navigation pairs, generates actual geometry with the repaired
format-10 generator, and adds validated files to the current test deployment.
No logger levels, warning filters, navigation-core code, executables or databases
are changed by this batch.

## Inputs and method

The initial inventory contains 6,748 installed tiles and 259 parent meshes. A
header/version/size/origin audit found no defects in those fields or duplicate
positions; it does not certify every polygon or path.
Among missing terrain/navigation pairs, 1,653 meet the existing strict empty-grid
predicate. Another 1,617 require investigation: 1,295 have an existing parent and
valid installed VMap root and are included in this generation plan. The excluded
inputs are 256 map 169 grids without a parent and 66 grids on maps 616/721/754/995
without a VMap root. These exclusions remain documented input limitations; no
parent or collision geometry is fabricated.

The generation plan is `mmap-coverage-plan-2026-11-06.csv`. Candidate does not mean
broken: collision models may not intersect a grid, and global-WMO terrain files
may be placeholders. The generator skips uniform no-height placeholders. A
successful run without output is recorded separately from an installed tile and
is not evidence that an arbitrary runtime navigation request is valid.

Per-cell liquid arrays were inspected before generation. All 113 inspected arrays
contain some ordinary water (typically 16 ordinary ocean cells and 240 dark ocean
cells); these were not classified as empty or hidden by the loader. Their actual
water navigation is generated from the installed inputs. In particular, the
map 571, grid 37/24 tile is generated at its real coordinate; the transposed 24/37 file is
never substituted.

Generator SHA256:
`9043e717f2215dc25e6957d7dbafe57e69b5fff9b310d8e1492943a4219d3419`.
The CLI takes Y,X for the runtime X,Y filename, for example:
`mmaps_generator.exe 571 --tile 24,37 --threads 1 --silent`.

Every generated tile is checked using `mmap_load_probe` with the original runtime
parent and available installed neighbors. Checks cover real load, duplicate load,
unload/reload, missing-map rejection, parent origin and neighboring coexistence.
Only successful files are installed, by atomic new-file rename. Existing tiles
and parent parameters are preserved. One final audit checks headers and parent
capacity; this is not a full world gameplay or long-duration navigation test.

## Reproduction and recovery

Run from the source root on Windows with the operator's own installed maps/vmaps:

```powershell
python contrib/navigation/repair_missing_tiles.py --inventory doc/mmap-coverage-plan-2026-11-06.csv --work out/mmap-coverage-reproduction --workers 8 --install
```

Omit `--install` to generate/probe without installing. Use a new work directory
when changing source assets or generator. Existing runtime files are skipped;
this tool does not regenerate or replace them. Result/log/probe details are in
`out/mmap-coverage-repair-20261106`; client assets and private runtime logs are not
committed. The checked-in result catalog records coordinates, outcomes and hashes;
`exports/manifest.json` pins package additions.

To undo, stop worldserver normally, preserve a copy of the new files, and remove
only files marked installed in this batch whose hashes still match the catalog.
Do not remove other files or parent meshes. No SQL rollback or EXE replacement is
needed. The original Detour crash acceptance and map 571 zone 0 caller investigation
remain separate; generating navigation does not resolve invalid world positions.

## Results

Completed all 1,295 candidates: 495 new tiles installed and 800 runs without generated geometry. No generator failures, probe rejections or exceptions. New tile payload totals 76,117,276 bytes. Installed coverage is now 7,243 tiles. Every staged parent is byte-identical to the runtime parent and no map exceeds its existing parent tile capacity. Exact per-tile outcomes and SHA256 values are in mmap-coverage-results-2026-11-06.csv.

Worldserver shut down normally and reopened visibly as PID 61632 at
01:32:46 local time. It is listening on 8085; authserver 61228 was left running.
The fresh log observation at 01:33:45 contains zero MMap warnings in Server.log,
Errors.txt and DBErrors.log. Generator logs contain no error-text matches.
This is fresh-start observation, not sustained travel acceptance. Separate
content-validation errors remain in the error logs and on the project checklist;
this repair does not hide them or claim an error-free release. Previous logs are
preserved in out/runtime-before-mmap-coverage-20261106-013245.

The pre/post DBErrors comparison contains no new exact error signatures
(1,450 records before, 1,448 after at this observation); existing content faults
remain separate. All 544 package manifest entries match their installed hashes.
