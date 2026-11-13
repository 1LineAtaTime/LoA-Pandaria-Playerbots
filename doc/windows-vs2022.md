# Windows x64 build with Visual Studio 2022

Use the existing source directory. The server is a CMake project; adding its source
files to a separate empty C++ project would lose the generated targets, include
paths, and module settings. Generate the solution below, or open the source folder
in Visual Studio and select the `vs2022-x64` CMake preset. The legacy
`CMakeSettings.json` is retained for installations using that settings format.

## Dependencies

Install the Visual Studio 2022 Desktop development with C++ workload, the x64
MSVC v143 tools, and a Windows 10/11 SDK. Use CMake 3.25 or newer for the supplied
presets. All third-party client libraries must be x64 and use the dynamic MSVC
runtime (`/MD` for RelWithDebInfo).

The initial machine inspection on 2026-10-28 found:

| Component | Installed version / location used for baseline |
| --- | --- |
| Visual Studio Community | 17.14.32 |
| MSVC | 19.44.35227, v143 toolset 14.44.35207 |
| Windows SDK | 10.0.26100.0 selected; 10.0.22621.0 also installed |
| CMake | 4.1.0 |
| Boost | 1.88.0, `C:/boost_1_88_0`, static vc143 x64 libraries under `stage/lib` |
| OpenSSL | 1.1.1w, `C:/Program Files/OpenSSL-Win64` |
| MySQL client SDK | 5.7, `C:/Program Files/MySQL/MySQL Server 5.7` |
| Alternative client SDK | MariaDB Connector/C 3.1.28, `C:/Program Files/MariaDB/MariaDB Connector C 64-bit` |

These are observations of existing installations, not a claim that each dependency
release still receives security updates. The connector used to build the server
does not determine the database server family at runtime. Use explicit matching
headers/import libraries when switching connectors, and rerun database tests.
Bundled dependencies (zlib, G3D, Recast/Detour, fmt, SFMT, Lua, StormLib, bzip2,
gSOAP, and Windows support libraries) are built by CMake.

## Configure, build, and stage

Run from the source directory in PowerShell. Dependency paths can be supplied on
the command line or in an ignored `CMakeUserPresets.json`; do not commit personal
paths or credentials.

```powershell
cmake --preset vs2022-x64 `
  '-DBOOST_ROOT=C:/boost_1_88_0' `
  '-DBOOST_LIBRARYDIR=C:/boost_1_88_0/stage/lib'
cmake --build --preset vs2022-x64-relwithdebinfo --parallel 4
```

The preset enables worldserver, authserver, Playerbots, modules, all scripts,
Eluna, and map extraction tools. `RelWithDebInfo` retains optimization and debug
symbols. The generated solution is
`out/build/vs2022-x64/Pandaria_5.4.8.sln`. Executables, PDBs, and default server
configuration files are under `out/build/vs2022-x64/bin/RelWithDebInfo`.

For a MariaDB client build, set both SDK paths together:

```powershell
cmake --preset vs2022-x64 `
  '-DMYSQL_INCLUDE_DIR=C:/Program Files/MariaDB/MariaDB Connector C 64-bit/include' `
  '-DMYSQL_LIBRARY=C:/Program Files/MariaDB/MariaDB Connector C 64-bit/lib/libmariadb.lib'
```

Package built executables, symbols, default configs, and the DLLs actually required
by their PE import tables:

```powershell
& ./contrib/windows/Stage-Runtime.ps1 -BuildDirectory ./out/build/vs2022-x64
```

To create the requested runnable export in the repository root, add
`-DestinationDirectory ./exports`. That directory is ignored by Git. The
package includes `worldserver.exe`, `authserver.exe`, the Playerbots default
configuration, tools, PDBs and resolved non-system DLLs; test executables are
excluded. Copy the `.conf.dist` files to local `.conf` files and fill in your
own database connection strings before running from the export directory.
Run `Start-Authserver.cmd` and `Start-Worldserver.cmd` from the package. Each
launcher selects its own directory before starting the executable, so `DataDir
= "."` and `LogsDir = "./logs"` work even when launched from another working
directory. Place the extracted client data directories beside `worldserver.exe`.
The default configs enable level 1 console logging while diagnosis is ongoing.
On Windows, both servers clear Command Prompt's QuickEdit input flag at startup
to prevent a text selection from pausing the process.

The script stages under `out/build/vs2022-x64/stage/RelWithDebInfo`, follows DLL
dependencies using the installed MSVC `dumpbin`, and fails if a dependency is
unresolved. Use `-DependencyDirectory` for additional installed dependency folders.
It does not distribute Windows system DLLs or the Visual C++ runtime. Install the
Microsoft Visual C++ 2015-2022 x64 Redistributable on the destination machine.
The package includes core licensing/attribution notices and a SHA-256 manifest. No databases, credentials, extracted game
data, or local `.conf` files are packaged.

To build and run the database source tests in this solution:

```powershell
cmake --preset vs2022-x64 -DBUILD_DATABASE_TESTS=ON
cmake --build --preset vs2022-x64-relwithdebinfo --target database_version_tests database_connector_probe
ctest --preset vs2022-x64-relwithdebinfo
```

The connector probe is an executable requiring an explicitly named disposable
database. It is not run by CTest automatically. See `contrib/database-tests` for
the isolated connection test instructions. When the servers are built, CTest also
checks that `--config` preserves paths containing spaces and rejects a missing
argument before opening a default configuration.

`cmake --install out/build/vs2022-x64 --config RelWithDebInfo` installs the
upstream CMake target/configuration set under `out/install/vs2022-x64`; the staging
script is the complete executable/DLL/PDB package path.

## Process-restricted build environments

The initial sandbox allowed compiler detection and serial compilation, but
MSBuild multi-node execution and MSVC `/MP` workers stalled. A normal authorized
build outside that process sandbox can use parallel compilation. If a restricted
environment must be used, configure and build with:

```powershell
cmake --preset vs2022-x64 -DMSVC_PARALLEL_COMPILE_JOBS=1
cmake --build --preset vs2022-x64-relwithdebinfo --parallel 1 -- /nr:false
```

`MSVC_PARALLEL_COMPILE_JOBS` is empty by default (the normal MSVC worker policy).
It accepts a positive integer to bound compiler workers per project. This changes
build concurrency only. Do not disable Playerbots, scripts, services, or database
checks to make a build pass.

## Source identity and validation

Revision generation reads the Git commit hash, count, branch, and commit date,
marks tracked local changes as dirty, and retains Mercurial/archive fallback for
historical distributions. Windows numeric file-version components remain within
their 16-bit range. The build-time regeneration preserves the configured SQL
client executable path used by the updater.

Build logs under `out/build/` are local generated evidence and are ignored. See
the project validation report and database/BattlePay compatibility notes for the
actual results; a successful compiler or version invocation does not prove a
running realm, Playerbots initialization, or a working 5.4.8 game-client store.
