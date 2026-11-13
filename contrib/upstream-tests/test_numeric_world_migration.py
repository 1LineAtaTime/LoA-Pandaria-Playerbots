"""Check guarded numeric UPDATEs in disposable world-table fixtures.

Copies whole referenced world tables, verifies all fields, repeat stability and
cleanup. Never applies the migration to the configured source schema.
"""
import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import re
import secrets
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--config', type=Path, required=True)
    parser.add_argument('--client', type=Path, required=True)
    parser.add_argument('--migration', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    pattern = re.compile(r'UPDATE `(?P<table>\w+)` SET `(?P<column>\w+)`=(?P<after>-?\d+) '
                         r'WHERE `ID`=(?P<id>\d+) AND `(?P=column)`=(?P<before>-?\d+);')
    sql = args.migration.read_text(encoding='utf-8-sig')
    updates = []
    for line in sql.splitlines():
        line = line.strip()
        if not line or line.startswith('--'):
            continue
        match = pattern.fullmatch(line)
        if not match:
            raise ValueError('Only guarded numeric updates are supported')
        updates.append(match.groupdict())
    if not updates:
        raise ValueError('No updates')
    tables = sorted({u['table'] for u in updates})
    connection = re.search(r'^WorldDatabaseInfo\s*=\s*"([^"]+)"',
                           args.config.read_text(encoding='utf-8-sig'), re.M)
    if not connection:
        raise ValueError('WorldDatabaseInfo missing')
    host, port, username, password, source = connection[1].split(';')
    if not re.fullmatch(r'[A-Za-z0-9_]+', source):
        raise ValueError('Invalid source identifier')
    schema = 'codex_numeric_fixture_' + secrets.token_hex(5)
    command = [str(args.client.resolve()), '--no-defaults', '--protocol=TCP',
               '--host=' + host, '--port=' + port, '--user=' + username,
               '--batch', '--skip-column-names', '--default-character-set=utf8mb4']

    def query(statement):
        process = subprocess.run(command, input=statement, capture_output=True, text=True,
                                 encoding='utf-8', timeout=120,
                                 env=dict(os.environ, MYSQL_PWD=password))
        if process.returncode:
            error = process.stderr.replace(password, '[REDACTED]') if password else process.stderr
            raise RuntimeError(error[:1500])
        return process.stdout.rstrip('\n').split('\n') if process.stdout else []

    report = {'passed': False, 'gameplay_tested': False,
              'migration_sha256': hashlib.sha256(sql.encode()).hexdigest()}
    try:
        query('CREATE DATABASE `' + schema + '`;')
        for table in tables:
            query(f'CREATE TABLE `{schema}`.`{table}` LIKE `{source}`.`{table}`;'
                  f'INSERT INTO `{schema}`.`{table}` SELECT * FROM `{source}`.`{table}`;')
        columns = {t: [r.split('\t')[0] for r in query(f'SHOW COLUMNS FROM `{schema}`.`{t}`;')]
                   for t in tables}

        def snapshot():
            result = {}
            for table in tables:
                if columns[table][0] != 'ID':
                    raise ValueError('Fixture expects ID as first column')
                # HEX(binary value) avoids client newline/CR and Unicode field
                # separators corrupting the TSV boundary or comparison.
                selection = 'ID,' + ','.join("IFNULL(HEX(CAST(`" + c + "` AS BINARY)),'NULL')"
                                            for c in columns[table][1:])
                result[table] = {int(r.split('\t')[0]): r.split('\t') for r in
                                 query(f'SELECT {selection} FROM `{schema}`.`{table}` ORDER BY ID;')}
            return result

        before = snapshot()
        expected = copy.deepcopy(before)
        affected = 0
        for update in updates:
            row = expected[update['table']][int(update['id'])]
            index = next(i for i, column in enumerate(columns[update['table']])
                         if column.casefold() == update['column'].casefold())
            if row[index] == update['before'].encode().hex().upper():
                row[index] = update['after'].encode().hex().upper()
                affected += 1
        query('USE `' + schema + '`;' + sql)
        after = snapshot()
        if after != expected:
            raise AssertionError('Unexpected row or field mutation')
        query('USE `' + schema + '`;' + sql)
        if snapshot() != after:
            raise AssertionError('Second application changed data')
        report.update(passed=True, full_row_preservation=True, idempotent=True,
                      guarded_updates=len(updates), guards_matched=affected,
                      table_rows={t: len(before[t]) for t in tables})
    finally:
        query('DROP DATABASE IF EXISTS `' + schema + '`;')
        report['fixture_removed'] = True
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
