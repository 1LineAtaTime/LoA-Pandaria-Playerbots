# Gunship rendering crash: build18414

Two original64-bit client dumps failed at RVA0x19d136 while approaching or
relogging at the static gunship. The fault decrements an M2 model reference
through null RCX. The caller at0x6a6910 constructs a model through0x175610
(M2Scene.cpp) and releases it without a null guard.

The captured heap object has client object type5 (gameobject), and its embedded
gameobject GUID decodes to template800100. This identifies the custom static
ship, rather than guessing from the character's position. Display8253 in the
actual GameObjectDisplayInfo.dbc resolves to
`World/wmo/Northrend/Buildings/WinterOrc/ND_HordeGunship.wmo`.

The ship template was generic gameobject type5. The original client's virtual
M2-model getter at RVA0x622bc0 excludes types11,14,15 and33 from its M2 path,
but accepts generic type5. This combination tried to load a WMO as an M2 and
then released the missing model. The earlier protocol-only ride test could
observe boarding and ejection but could not render the ship, so it did not
detect this data defect. Those earlier results were not render acceptance.

Migration `2026_11_12_03_world_gunship_wmo_client_safety.sql` changes only the
custom800100/display8253 generic template to map-object type14. Its position,
orientation, WMO collision model and scene actors are preserved. Type14 avoids
the erroneous M2 loader and does not require a transport animation path.
No client executable is patched and no logger threshold is changed.

The same migration corrects level1/42-HP data on passenger rides43751 and43713
to level12, Health_mod3. These are scripted flight/escape actors with existing
flight protections; no combat spells are added to the passenger action bar.

The affected character was recovered offline to Keel Harbor after graceful
worldserver shutdown. Only location/transport attachment fields were changed;
quest progress, inventory and account credentials were preserved. Private
pre-change rows and the original client dumps are under
`out/backups/gunship-client-crash-20261112/`, excluded from Git and the handoff.
The world template changes are data-only and were loaded by restarting the
existing paired native build. Rollback requires restoring the backed-up template
rows, but restoring the generic ship type also restores the known crash trigger.

`contrib/database-tests/Check-GameObjectModelTypes.py` compares world templates
against actual DBC asset paths. The scoped800100 check passes after repair.
An all-template scan inspected41,361 templates, including582 WMO entries, and
flagged346 other type/asset combinations for review. These are candidates, not
346 reproduced crashes: many templates may be unused and doors/goobers require
individual behavior review. They were not bulk-converted. Real-client rendering
and collision remain separate acceptance checks.

The user confirmed login and flight reached the rendered gunship without the
client crash. That retry exposed overlapping ships and an unsafe rope descent.
The legacy `transports` spawn203428 (guid28, Worgen area) remained globally
visible with no phase rule alongside the replacement static800100. Its moving
transport uses the same display8253. Migration04 removes only that old world
spawn; templates, paths, crew and unrelated ships are preserved.

The rope used NearTeleportTo, which deliberately retains transport attachments.
The native follow-up uses world-space TeleportTo without that flag for rope,
hippogryph dismount and wyvern return. The rope requires an active/completed
Endgame quest and validates a nearby dynamic collision floor in the rope's own
phase, even for a GM. Without a valid floor it refuses the descent instead of
dropping the player. Visual Studio RelWithDebInfo/x64 built with zero failed
projects. The matching EXE/PDB and migration04 are deployed.

The player was recovered online to Keel Harbor through the native teleport
command. The temporary recovery bookmark was removed and its cache reloaded;
no quest status was changed.

Live packet regression observed exactly one static800100 with wire type14 and
display8253. After migration04, the legacy ship spawn count is zero. A separate
completed-quest fixture clicked the real rope and landed on measured collision
at map654,(-1483.42,3250.66,128.997), with transguid0. The fixture's completed
quest state avoids starting the shared boarding controller; this is a rope and
placement test, not a natural combat run. Both owned identities and dependent
rows were removed. Runners: `gunship_wire_regression.py` and
`gunship_rope_regression.py` under `contrib/headless-client`.

The user confirmed the real-client result after migration04: no overlapping
ship, the rope worked, and the character is aboard fighting. This closes the
reported rendering crash, overlap and unsafe descent checks. Complete normal
combat, escape and quest turn-in remain separate from that confirmed result.
The running server was left intact while the player continued the fight.
