# Public distribution boundary

Public source and SQL contain no working operator credentials, account saves,
character records, mail, wallet balances, purchase history, private keys or dumps.
The two cleaned base schemas retain only schema/static definitions. The optional
shop catalogue is static game content, not financial/customer history.

Full gameplay configuration examples are under config/examples and use DB_USER /
CHANGE_ME. Fill in connections locally. No private config exception applies to
this public fork. The local operator's runtime configs remain outside this tree.

Nicalicious is intentionally credited publicly; upstream/imported authorship is
retained. Private development history is not a parent of the contribution commits.
Public upstream history remains upstream history, not a privacy certification of
all earlier upstream releases. Review upstream baseline database assets before
importing them.

Run contrib/release/verify_distribution.py to verify146 SQL checksums and detect
common secret/runtime files. The result is one check, not a guarantee against
every possible secret format. No compiled EXEs/DLLs/PDBs/client map assets are
included in Git. See CONTRIBUTION.md and UPGRADE.md at the repository root.
