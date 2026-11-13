# example account mailbox cleanup and investigation

## User acceptance — 2026-11-04

The user logged ExampleCharacter in twice after deployment and confirmed that achievement
mail spam/duplication stopped. This satisfies the repeated-login acceptance case;
it does not establish crash-boundary exactly-once delivery for every mail source.

The user explicitly requested clearing all mail on example account. Worldserver was
shut down through its normal console command so cached mail could not resurrect
deleted rows. Private SQL backups of mail, mail_items and attached item_instance
rows were saved under ignored out/example account-mail-20261104-* before deletion.

Seven characters were covered. The transaction removed 107 mail records, 88
attached-item links, and 87 unreferenced attached item instances. One item already
referenced by inventory was preserved. A second backed-up transaction removed 315
orphan attachment links: all had no parent mail, physical item or inventory record.
Final verification returned zero mail and zero attachment links for the account.
Pending mailed money was removed with the messages; bags, bank items, account DP,
BattlePay delivery receipts and achievement history were preserved.

Repeated mail was predominantly achievement rewards from senders 28951/32216,
at ten separate delivery times. Identical Keep Collecting subjects can also refer
to distinct achievements 5875 and 5877, so matching subject text alone does not
establish a duplicate. Other mount/pet rewards repeated across the ten deliveries.
Those records and their timing are preserved in the private backups.

An independently confirmed loader bug reads mail.checked TINYINT through
GetUInt32 on a prepared raw result. This can include adjacent bytes and explains
the historical out-of-range checked save error. It is changed to GetUInt8. Such
save failures can disrupt persistence, but this alone does not prove every repeated
achievement reward is fixed. Verify two subsequent logins, character/account
achievement persistence and mail counts; inspect durable reward idempotency if
repeated deliveries continue. No achievement rows were reset or erased.

Auth logging now has a dedicated quiet profile: WARN/ERROR/FATAL on console,
INFO in Auth.log, SQL errors in AuthErrors.txt, timestamped backups and size
rotation. SQL DEBUG/TRACE chatter is disabled. Authserver startup is sequenced
after worldserver is online because its existing realm query excludes flag 3.

## Confirmed repeated-reward initialization defect

The current AccountInfoQueryHolderPerRealm contained only global account settings and tutorials. Its InitializeSessionCallback never loaded the existing account achievements or criteria. The older LoadGlobalAccountData method contained those loads but had no callers. Thus a fresh session could re-earn already persisted account-wide pet/mount achievements and mail their rewards again. The holder now asynchronously loads both achievement result sets and populates AccountAchievementMgr before AUTH_OK/character login. Those prepared statements are available on both connection types. Socketless bot sessions restore the same account state explicitly before HandlePlayerLogin. The mail.checked width fix remains independently necessary. Both repairs require login/relogin acceptance testing; the mailbox cleanup alone is not a duplication fix.

## Runtime validation

Both login paths and the mailbox flag repair are built and deployed. Worldserver is revision 8f9187cf; the auth lifecycle build is tracked separately at 48597412. Staged EXE/PDB pairs match their build outputs. CTest passed 10/10, including 256 raw byte-field cases. The account-achievement queries prepare on synchronous and asynchronous connections. Re-login reward replay acceptance remains pending; no achievement completion/progress records were cleared by the mailbox cleanup. Mail and attachment links were verified empty immediately before the patched worldserver restart. Quiet auth logging and the dedicated loading/ready lifecycle messages were verified in Auth.log and the listener on port 3724.
