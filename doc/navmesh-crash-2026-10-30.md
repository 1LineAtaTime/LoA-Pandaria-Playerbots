# Detour navigation crash — 2026-10-30 (C0000005)

## Evidence and scope

- Worldserver: Pandaria 5.4.8 Rev 1253, `RelWithDebInfo`, build hash `bf40a3c47cd8aed8793bf7ff16f23d49243de6e0`, Win64.
- Crash: Windows exception `0xC0000005` (access violation), in `closestPointOnDetailEdges<true>()`, `dep/recastnavigation/Detour/Source/DetourNavMesh.cpp:673`.
- Stack: `Creature::Update` -> `RandomMovementGenerator<Creature>::_setRandomLocation` -> `PathGenerator::CalculatePath` -> `dtNavMeshQuery::findNearestPoly` -> `dtNavMesh::closestPointOnPoly` -> `closestPointOnDetailEdges`.
- Reported movement coordinates (source): X=-10370.799805, Y=-3249.841797, Z=19.525539; destination: X=-10371.894531, Y=-3254.385498, Z=20.004396.
- This is a **native navigation/movement crash**, not evidence of a direct BattlePay, AuctionHouseBot, or playerbot AI failure.

## Source defect and patch

The existing detail-edge search can finish without setting `pmin` and `pmax` when a detail polygon has no suitable marked boundary edges (or no detail triangles). The old code unconditionally called `dtVlerp(closest, pmin, pmax, tmin)` and could dereference null.

The patch in `dep/recastnavigation/Detour/Source/DetourNavMesh.cpp`:

1. Retains the existing detail-edge selection when valid edges are present.
2. Otherwise finds the nearest point on the *base polygon perimeter*, after checking vertex counts, indices, and finite coordinates.
3. If no valid edge exists, returns the input position rather than dereferencing invalid pointers.

No database schemas, world content, map/MMap data files, playerbot configs, or authentication settings are modified by this patch. The patch intentionally changes only Detour's `closestPointOnDetailEdges` fallback logic. Avoid broad navigation rewrites until runtime evidence justifies them.

## Validation and remaining investigation

The patched Detour implementation was compiled and exercised in a focused Linux regression harness with AddressSanitizer and UndefinedBehaviorSanitizer. Cases covered normal marked edges, missing detail boundary flags, and zero detail triangles; no sanitizer errors were reported in those cases.

**Still unverified:** full Windows worldserver compilation from this checkout, long-duration game-server operation, whether the deployed MMap files were generated with matching tools, and whether navigation tiles have independent corruption or lifetime/concurrency issues. Do not mark the incident resolved until a new Windows build runs stably under the original bot/NPC load.

For any repeat crash, capture the new `*.txt` report, matching `*.dmp`, `worldserver.exe` build hash and PDB, and the map/location if available. Compare the new top stack frames with this incident before changing any MMap files.

## Rebuild and Git workflow

From the repository root on Windows:

```powershell
cmake --build --preset vs2022-x64-relwithdebinfo --target worldserver --parallel 4
```

Deploy the newly built worldserver executable and matching debug symbols using the existing staging workflow documented in `doc/windows-vs2022.md`. Building is not the same as deploying. Source changes do not require regenerating MMaps or applying SQL migrations.

Git policy remains `doc/repository-workflow.md`: develop locally, record scoped commits, push only to **Forgejo** (`origin`) when ready, and let Forgejo's configured mirror update GitHub. Never force-push or edit the GitHub mirror directly. If Codex makes further modifications, retain this incident note as context and avoid silently undoing the guard without a tested replacement.
