"""Run published SQL individually against disposable, privilege-isolated fixtures.

SQL success is compatibility evidence, not a gameplay test or deployment approval.
Credentials stay in subprocess environments; no auth/character data is copied.
"""
import argparse
import collections
import hashlib
import json
import os
from pathlib import Path
import re
import secrets
import subprocess
import time


def identifier(value):
    if not re.fullmatch(r"[A-Za-z0-9_]+", value):
        raise ValueError("Invalid database identifier")
    return "`" + value + "`"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--config", type=Path, required=True)
    parser.add_argument("--client", type=Path, required=True)
    parser.add_argument("--patches", type=Path, required=True)
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--limit", type=int, default=0)
    args = parser.parse_args()
    connection = re.search(r'^WorldDatabaseInfo\s*=\s*"([^"]+)"',
                           args.config.read_text(encoding="utf-8-sig"), re.M)
    if not connection:
        raise ValueError("WorldDatabaseInfo missing")
    host, port, admin, admin_password, source = connection[1].split(";")
    identifier(source)
    suffix = secrets.token_hex(5)
    baseline = "codex_patch_base_" + suffix
    workspace = "codex_patch_work_" + suffix
    username = "codex_patch_" + suffix
    password = secrets.token_hex(32)
    common = [str(args.client.resolve()), "--no-defaults", "--protocol=TCP",
              "--host=" + host, "--port=" + port, "--batch", "--skip-column-names",
              "--default-character-set=utf8mb4"]
    user_host = None
    report = {"source_schema": source, "results": [], "completed": False,
              "gameplay_validated": False, "live_patches_applied": 0}
    args.report.parent.mkdir(parents=True, exist_ok=True)

    def query(sql, privileged=False, database=None, timeout=120, allow_failure=False):
        account = admin if privileged else username
        secret = admin_password if privileged else password
        command = common + ["--user=" + account]
        if database:
            command.append("--database=" + database)
        result = subprocess.run(command, input=sql, text=True, encoding="utf-8",
                                capture_output=True, timeout=timeout,
                                env=dict(os.environ, MYSQL_PWD=secret))
        error = result.stderr.replace(admin_password, "[REDACTED]") if admin_password else result.stderr
        error = error.replace(password, "[REDACTED]")
        if result.returncode and not allow_failure:
            raise RuntimeError(error[:1500])
        return result.returncode, result.stdout, error

    def snapshot():
        names = query("SHOW TABLES;", database=workspace)[1].splitlines()
        result = {}
        for table in names:
            qt = identifier(table)
            values = query("SHOW CREATE TABLE " + qt + "; CHECKSUM TABLE " + qt +
                           " EXTENDED; SELECT COUNT(*) FROM " + qt + ";", database=workspace)[1].splitlines()
            if len(values) < 3 or not values[-1].isdigit():
                raise RuntimeError("Incomplete fixture snapshot for " + table + ": " + repr(values[-3:]))
            ddl, checksum, count = values[0], values[-2].split("\t")[-1], values[-1]
            result[table] = {"rows": int(count), "checksum": checksum,
                             "schema_sha256": hashlib.sha256(ddl.encode()).hexdigest()}
        return result

    def save():
        args.report.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")

    try:
        user_host = query("SELECT SUBSTRING_INDEX(USER(),'@',-1);", privileged=True)[1].strip()
        if not re.fullmatch(r"[A-Za-z0-9_.:%-]+", user_host):
            raise ValueError("Unexpected client host identity")
        account = "'" + username + "'@'" + user_host + "'"
        query("CREATE DATABASE " + identifier(baseline) + " CHARACTER SET utf8mb4;"
              "CREATE DATABASE " + identifier(workspace) + " CHARACTER SET utf8mb4;"
              "CREATE USER " + account + " IDENTIFIED BY '" + password + "';"
              "GRANT SELECT ON " + identifier(baseline) + ".* TO " + account + ";"
              "GRANT SELECT,INSERT,UPDATE,DELETE,CREATE,ALTER,DROP,INDEX,CREATE TEMPORARY TABLES "
              "ON " + identifier(workspace) + ".* TO " + account + ";", privileged=True)
        # Fail closed: the patch executor must not even read or issue a zero-row
        # update against a live source table. This check itself changes no rows.
        for statement in ("SELECT * FROM " + identifier(source) + ".spell_area LIMIT 0;",
                          "UPDATE " + identifier(source) + ".spell_area SET spell=spell WHERE 0;"):
            code, _, error = query(statement, allow_failure=True)
            if not code or not re.search(r"denied", error, re.I):
                raise RuntimeError("Live-schema privilege isolation not proven")
        report["live_schema_access_denied"] = True
        table_rows = query("SELECT TABLE_NAME FROM information_schema.TABLES WHERE TABLE_SCHEMA='" +
                           source + "' AND TABLE_TYPE='BASE TABLE';", privileged=True)[1].splitlines()
        tables = {table.lower(): table for table in table_rows}
        prepared = set()
        files = sorted(args.patches.glob("*.sql"))
        if args.limit:
            files = files[:args.limit]
        for index, path in enumerate(files, 1):
            started = time.monotonic()
            sql = path.read_text(encoding="utf-8-sig")
            # Overinclusive references are safe: comments may cause an extra
            # table copy, but cannot give the executor broader privileges.
            tokens = set(re.findall(r"[A-Za-z_][A-Za-z_0-9]*", sql.lower()))
            referenced = sorted(tables[t] for t in tokens & tables.keys())
            for table in referenced:
                if table not in prepared:
                    qt = identifier(table)
                    query("CREATE TABLE " + identifier(baseline) + "." + qt + " LIKE " +
                          identifier(source) + "." + qt + "; INSERT INTO " + identifier(baseline) +
                          "." + qt + " SELECT * FROM " + identifier(source) + "." + qt + ";",
                          privileged=True)
                    prepared.add(table)
            # Restore an independent baseline before every patch, rather than
            # letting earlier candidates hide the effects of later ones.
            old_tables = query("SHOW TABLES;", database=workspace)[1].splitlines()
            statements = ["DROP TABLE " + identifier(table) + ";" for table in old_tables]
            for table in referenced:
                qt = identifier(table)
                statements.append("CREATE TABLE " + qt + " LIKE " + identifier(baseline) + "." + qt + ";")
                statements.append("INSERT INTO " + qt + " SELECT * FROM " + identifier(baseline) + "." + qt + ";")
            if statements:
                query("".join(statements), database=workspace)
            before = snapshot()
            code, _, error = query(sql, database=workspace, allow_failure=True)
            after = snapshot()
            changed = {t: {"before": before.get(t), "after": after.get(t)}
                       for t in before.keys() | after.keys() if before.get(t) != after.get(t)}
            status = "sql_failed" if code else ("sql_changes_fixture" if changed else "sql_no_observed_change")
            report["results"].append({"patch": path.name, "sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
                                       "status": status, "tables": referenced, "changed": changed,
                                       "error": error[-1500:] if code else "",
                                       "seconds": round(time.monotonic() - started, 2)})
            if index % 10 == 0 or index == len(files):
                save()
                counts = dict(collections.Counter(r["status"] for r in report["results"]))
                print(json.dumps({"done": index, "total": len(files), "counts": counts}), flush=True)
        report["completed"] = True
        report["counts"] = dict(collections.Counter(r["status"] for r in report["results"]))
    finally:
        cleanup = []
        if user_host:
            cleanup.append("DROP USER IF EXISTS '" + username + "'@'" + user_host + "';")
        for name in (workspace, baseline):
            if not name.startswith("codex_patch_") or name == source:
                raise RuntimeError("Refusing unsafe fixture cleanup")
            cleanup.append("DROP DATABASE IF EXISTS " + identifier(name) + ";")
        query("".join(cleanup), privileged=True)
        report["fixtures_removed"] = True
        save()
    print(json.dumps({k: v for k, v in report.items() if k != "results"}), flush=True)


if __name__ == "__main__":
    main()
