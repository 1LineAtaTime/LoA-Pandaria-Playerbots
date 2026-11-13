# LoA runtime repairs — November 8

The final beta startup still exposed stale AI bindings and missing scene text.
This batch repairs the following causes without changing logging thresholds.

- The opening Gilneas Prince Liam speech script was attached to later entry 36451,
  which has no speech groups 0–2. Move that script to opening entry 34850, whose
  existing dialogue matches those groups. Do not invent replacement lines for the
  later quest NPC.
- Enable existing SmartAI for Civilian Recruit 25220, Marshtide Sentry 46714 and
  Marshtide Mage 46775, which had neither native script nor active AIName.
- Remove source_type 0 SmartAI records for 25 templates already owned by native
  scripts present in this build. Both native source presence and absence from the
  running build's unregistered-script diagnostics were checked. Native ownership
  stays unchanged. This removes inactive duplicate behavior definitions; it does
  not claim to implement additional missing quests. Unregistered native bindings
  remain open rather than deleting their only available behavior definition.
- Gilneas guard conversations previously selected a citizen from an empty list
  when no citizens were nearby, underflowing the random range and advancing an
  invalid iterator. Return an empty GUID and retry the scene after 10 seconds.

Before mutation, creature_template, creature_text and smart_scripts were dumped
under ignored out/loa-runtime-repairs-20261108/before.sql. The SQL is guarded by
exact entry/script ownership. Five binding/dialogue data checks and all 85 Gilneas
progression data contracts passed. Deployment and runtime evidence follows below.

Remaining work includes unregistered native bindings, missing reference dialogue,
invalid SmartAI targets/conditions and full gameplay journeys. Passing data checks
does not establish end-to-end quest completion.

Visual Studio build completed with zero failed projects, and all 22 registered native
tests passed. Both EXE/PDB pairs were staged in exports with matching SHA-256
hashes, then launched through the visible console scripts. No client retest of
the absent-citizen scene has yet been performed.

The opening Liam had three existing SmartAI speech timers. Their exact event,
action and target fields confirmed that they only duplicate the native speech
sequence; those three rows were removed after moving the native binding.

Final startup: both ports 3724/8085 listen; distinct SQL diagnostics fell from
1,353 to 1,293 (60 resolved, zero new signatures in this bounded comparison).
The three newly enabled SmartAI templates and opening Liam emit no AIName
mismatch. Other 1,293 content diagnostics remain open. Forgejo and GitHub
matched all 16 refs after the repair commit 6e98decc; main is default on both.
