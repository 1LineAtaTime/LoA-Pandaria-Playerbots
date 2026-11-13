# Howling Oak questgiver and Tarien vendor

Quest28517 The Howling Oak names Genn Greymane48736 as its turn-in NPC.
Gwen Armstead48735 is the nearby innkeeper and vendor; her lack of a turn-in
is expected. The original world had the correct creature_questender relation
but no map1 spawn of48736. A quest relation without its actor cannot complete
through normal NPC interaction. Quest text reference:
https://www.wowhead.com/ptr/quest=28517/the-howling-oak .

SQL06 restores one stationary Genn beside Gwen at the Howling Oak, on Gwen’s
measured floor and normal phase1. It derives coordinates from the existing
Gwen spawn and skips insertion if any map1 Genn48736 already exists. This is
a local functional placement, not a reproduction of a retail-sniffed spawn or
the full Tyrande/Genn welcome scene. Gwen’s services and quest relations remain
unchanged; no player quest status is edited.

Tarien Silverdew52644 already had the vendor flag but no npc_vendor entries.
The migration adds missing stock from Laida Gembold52584, the corresponding
Alliance jewelcrafting supplier. All92 referenced item templates exist. Slots,
limited-stock timers, item type and ExtendedCost currency requirements are
preserved. Existing Tarien keys/custom stock are retained. This restores an
operational profession vendor; an item-by-item historical retail catalogue
comparison is not claimed.

Only world data changed; no native rebuild was necessary. Worldserver was
shut down through server shutdown1, SQL06 applied and the same compiled native
pair restarted visibly. Scoped before-data and rollback SQL are private under
out/backups/howling-oak-tarien-20261112. Both scopes were empty before the change.

contrib/headless-client/howling_oak_tarien_regression.py creates one owned
Worgen fixture, seeds only its departure prerequisites, normally accepts28517
from Krennan, approaches the restored Genn, receives real gossip and offer-reward
responses0x0244/0x074f, and completes the normal quest reward exchange. A separate
ordinary CMSG_LIST_INVENTORY to Tarien receives92 items in a3714-byte response.
GM mode is off for these interactions. Administrative transfers shorten travel;
this does not certify natural navigation or the entire welcome roleplay scene.
Logout was acknowledged and the fixture/account/dependencies removed.
Private report: out/howling-oak-tarien-regression.json.

## Idriana banker follow-up

Idriana4155 already had banker/gossip flags131075. Her menu2404 option0 named
checking the deposit box had action type0 and required NPC flag0 in both
canonical and legacy fields, so selection could not dispatch the bank service.
SQL07 restores BANKER9 and flag131072 in both field representations. Text and
other menus remain intact. A read-only comparison of equivalent banker options
found only this one broken entry. Reloading gossip_menu_option applied the fix
without restarting either server or recompiling the executables.

The disposable ordinary-interaction test selected the deposit-box option and
received SMSG_SHOW_BANK0x0007. Logout and native fixture/account/dependency cleanup
completed. Runner: contrib/headless-client/idriana_bank_regression.py; private
report: out/idriana-bank-regression.json. Scoped before-data and rollback are in
out/backups/idriana-bank-20261112. No player bank items or money were modified.
