# Database compatibility and validation

The same source connects to MySQL 5.5 and MariaDB 10.5. On 2026-10-28,
isolated portable Windows x64 servers MySQL **5.5.62** and MariaDB **10.5.29**
passed the schema and connector checks below. Full world/playerbots startup and
in-game behavior require additional data and are separate validation steps.

## Server identity and client connector

Every newly opened or reconnected session executes
`SELECT VERSION(), @@version_comment`. It parses three bounded numeric version
components and identifies MariaDB from either string, case-insensitively. Only
an identified MariaDB server has the `5.5.5-` compatibility prefix stripped.
Malformed identities fail connection setup. Server selection no longer depends
on the library used to compile the application.

The runtime capability floor is MySQL **5.5.3** (the first version with
`utf8mb4`) or MariaDB **10.2.9**, retaining the previous MariaDB floor. The
tested legacy MySQL version is **5.5.62**; admission by version alone does not
certify every older patch or server configuration. Both initial charset setup
and `mysql_set_character_set` errors fail connection setup.

Client requirements remain separate: MySQL client >=5.7 or MariaDB Connector/C
>=3.0.3, with matching compile/runtime library versions. Do not build the server
against MySQL 5.5 headers just because the database server is 5.5. The
`MYSQL_ROOT_DIR` CMake hint accepts MariaDB Connector/C installations; explicit
`MYSQL_INCLUDE_DIR` and `MYSQL_LIBRARY` resolve ambiguous installations. Headers,
import library, runtime DLL and authentication plugins must come from the same
connector installation and architecture. FindMySQL also recognizes
`mariadb_config`, `mariadb`, and `include/mariadb` layouts.

The installed connectors tested here are MySQL **5.7.44** and MariaDB
Connector/C **3.1.28**. This is a record of installed versions, not a claim they
are the newest maintained releases. A newer connector requires its own rebuild
and matrix run. MySQL 5.5 native-password authentication over loopback was
tested. TLS, remote authentication and alternate plugins were not tested;
compatibility work does not weaken TLS policies or change server authentication.

## MySQL 5.5 schema prerequisites

The source uses `utf8mb4`; the supported collations are ordinary
`utf8mb4_bin` / `utf8mb4_unicode_ci`. It does not substitute a lossy character
set. Full `VARCHAR(200)` update keys need 800 bytes, while the default old
InnoDB index limit is 767 bytes. The characters `banned_addons(Name,Version)`
unique key needs 2040 bytes.

For the isolated MySQL 5.5 test instance, its private configuration explicitly
sets `innodb_file_format=Barracuda`, `innodb_file_per_table=1` and
`innodb_large_prefix=1`. The affected base tables now explicitly use
`ROW_FORMAT=DYNAMIC`. No key is shortened and uniqueness semantics are retained.
See `contrib/database-tests/mysql55-test.ini.example`.

Existing schemas can use `sql/compat/mysql55/auth.sql` and `characters.sql`
after those prerequisites are checked. These ALTERs rebuild tables and require
normal deployment planning. They were not applied to any production server.
The base SQL files contain DROP/CREATE statements and upstream demo data; use
them only for fresh disposable databases, never as an upgrade migration.

Current database implementation prepared statements and active `sql/updates`
were scanned for JSON, window functions, fractional datetime, generated columns
and newer collation dependencies. None were found in that scope. Archived
`sql/old/characters_maintenance/2026_10_05_01_characters_dedupe_bot_names.sql`
uses `ROW_NUMBER() OVER` and is incompatible with MySQL 5.5; it is not part of
the active upgrade path. This audit does not certify every historical/manual
script or an external world dump. Playerbots base imports use some MyISAM tables;
new purchase ledgers and receipts require InnoDB for their transaction semantics.

## Transaction behavior

The core checks START and COMMIT results. It explicitly disables connector
automatic reconnect, and never reconnects/replays a failed statement inside a
transaction. A failed rollback discards the session and destroys prepared
statements before closing its connection. The next operation opens and prepares
a new session. A missing COMMIT acknowledgement returns failure with an unknown
outcome: durable receipt state must be queried before retrying a business action.

Deadlock retries apply only while the current error is a deadlock; a subsequent
different error stops retries. `AppendChecked(sql, expectedAffectedRows)` rolls
back all statements on a row-count mismatch. `DirectExecuteChecked` executes
prepared wallet mutations once, returns affected rows and does not transparently
replay a mutation after a lost acknowledgement. Cross-database payment and
delivery use BattlePay durable accounting rather than pretending these local
SQL transactions span both databases.

## Test matrix and evidence

All database data is under ignored `out/build/database-tests`; the standalone
servers bind only to `127.0.0.1:33555` and `127.0.0.1:33556`, with no Windows
service installed. No production connection, migration or account was used.

| Check | MySQL 5.5.62 | MariaDB 10.5.29 |
|---|---|---|
| Import `sql/base/auth.sql` | Pass, 45 tables/views | Pass, 45 tables/views |
| Import `sql/base/characters.sql` | Pass, 129 tables | Pass, 129 tables |
| Import playerbots `playerbots_database_full.sql` | Pass, 13 tables | Pass, 13 tables |
| MySQL 5.7.44 C connector probe | Pass | Pass |
| MariaDB 3.1.28 C connector probe | Pass | Pass |
| Actual core transaction/reconnect probe | Pass | Pass |
| Authserver startup/prepared statements | Pass, actual Login pool/listener | Pass, actual Login pool/listener |
| Login, Character and Playerbots prepared statements on imported base schemas | Pass | Pass |
| Full external world schema and world startup | Not yet certified | Not yet certified |
| Full Playerbots initialization / 5.4.8 game client | Not yet certified | Not yet certified |

The dependency-free C++ version tests passed **26 cases** with Visual Studio
2022 MSVC 19.44, x64 RelWithDebInfo, through CTest. Cases include prefixed and
unprefixed MariaDB, identity through version_comment, case variations, actual
MySQL 5.5.5, version boundaries, malformed strings and numeric-overflow input.
Both connectors on both servers passed real binary prepared statements,
four-byte UTF-8 round trips, conditional debit affected rows, insufficient-funds
rejection and explicit rollback. These probes operate on temporary tables.
Authserver startup logs are retained in ignored
`out/build/vs2022-x64/auth-smoke/33555` and `33556`; they show actual Login pool
opening, prepared statements and test-only listener startup on both families.
The linked core transaction probe passed affected-row guard rollback, duplicate
statement rollback, prepared-statement affected-row checks, self-KILL during an
active transaction followed by rollback/reconnect/reprepare, and a subsequent
successful checked commit on both servers. Network loss during the COMMIT
acknowledgement itself still requires a fault proxy for deterministic testing;
the implementation reports that outcome as unknown rather than retrying it.

The bundled `playerbots_database_full.sql` creates only 13 of the Playerbots
tables currently referenced by prepared statements. The disposable fixture
therefore loaded the missing 15 table definitions from the individual base SQL
files before the full pool probe. `Prepare-PlayerbotsSchemaFixture.ps1` repeats
that schema-only step on the fixed loopback test ports. Both server families
then prepared all Login, Character and Playerbots statements successfully.
This check does not import the large travel-node and text data from those
individual files; a fresh production Playerbots installation must use the
complete current Playerbots SQL dataset, not just the combined dump.

## Reproduce

With the normal CMake project configured, set `BUILD_DATABASE_TESTS=ON` and
build `database_version_tests`, `database_connector_probe` and
`database_transaction_probe`. CTest runs the dependency-free parser tests.
The connector and core transaction probes require explicit disposable database
arguments and refuse a database name without the `mop_test_` prefix:

```powershell
ctest --preset vs2022-x64-relwithdebinfo --output-on-failure
./out/build/vs2022-x64/bin/RelWithDebInfo/database_connector_probe.exe 33555 mop_test_auth
./out/build/vs2022-x64/bin/RelWithDebInfo/database_transaction_probe.exe 33555 mop_test_auth
```

Add `--schema-pools` to the core probe to prepare actual Login, Character and
Playerbots database statements against the three fixed disposable databases
`mop_test_auth`, `mop_test_characters` and `mop_test_playerbots`. This performs
statement preparation without creating accounts or starting the game world.
Before this check, run `contrib/database-tests/Prepare-PlayerbotsSchemaFixture.ps1`
with `-MySqlExecutable` pointing to the isolated client's executable and
`-Port 33555` or `33556` to supply tables absent from the combined dump.

Place the selected connector DLL next to the executable or on its process PATH.
The optional environment variables `MOP_TEST_DB_USER` and
`MOP_TEST_DB_PASSWORD` supply credentials without logging them in command
arguments. Probes force loopback. The core transaction probe creates/drops only
its named `mop_core_transaction_probe` fixture in the disposable database.

Parser and connector tests can also be configured independently of all server
dependencies:

```powershell
cmake -S contrib/database-tests -B out/build/database-tests-standalone -G "Visual Studio 17 2022" -A x64
cmake --build out/build/database-tests-standalone --config RelWithDebInfo
ctest --test-dir out/build/database-tests-standalone -C RelWithDebInfo --output-on-failure
```

Add `MYSQL_INCLUDE_DIR` and `MYSQL_LIBRARY` when configuring to build the optional
connector probe. The actual core probe requires the full project.

Official portable archives used:

- MySQL 5.5.62: `https://cdn.mysql.com/archives/mysql-5.5/mysql-5.5.62-winx64.zip`;
  published MD5 matched `bccba89f022e3ce3996880a02a4dc759` (transport HTTPS).
- MariaDB 10.5.29: `https://archive.mariadb.org/mariadb-10.5.29/winx64-packages/mariadb-10.5.29-winx64.zip`;
  published SHA256 matched
  `a6b11c7ed51a204d33f0a9d0344b680d1067534153ec3c9432dbcaf789b01cab`.

References: [MariaDB handshake version prefix](https://mariadb.com/docs/server/reference/clientserver-protocol/1-connecting/connection),
[MariaDB Connector/C API](https://mariadb.com/docs/connectors/mariadb-connector-c/mariadb-connector-c-guide),
[MySQL InnoDB format/index default changes](https://dev.mysql.com/worklog/task/?id=7703).
