"""Run tracked read-only world-data assertions; these are not gameplay tests."""
import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import sys
from datetime import datetime, timezone


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--config', type=Path, required=True)
    parser.add_argument('--client', type=Path, required=True)
    parser.add_argument('--suite', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    connection = re.search(r'^WorldDatabaseInfo\s*=\s*"([^"]+)"',
                           args.config.read_text(encoding='utf-8-sig'), re.M)
    if not connection:
        raise ValueError('WorldDatabaseInfo missing')
    host, port, username, password, database = connection[1].split(';')
    command = [str(args.client.resolve()), '--no-defaults', '--protocol=TCP',
               '--host=' + host, '--port=' + port, '--user=' + username,
               '--database=' + database, '--batch', '--skip-column-names',
               '--default-character-set=utf8mb4']
    suite = json.loads(args.suite.read_text(encoding='utf-8-sig'))
    checks = suite['checks']
    if not checks or len({c['id'] for c in checks}) != len(checks):
        raise ValueError('Empty suite or duplicate case IDs')
    # Validate the entire tracked suite before making any request. This is a
    # generated-query guard, not a sandbox for arbitrary untrusted SQL.
    for check in checks:
        sql = check['query'].strip().rstrip(';')
        if (not re.match(r'^SELECT\s', sql, re.I) or ';' in sql or
                re.search(r'\b(INTO|OUTFILE|DUMPFILE|SLEEP|BENCHMARK|GET_LOCK)\b|--|/\*|#', sql, re.I)):
            raise ValueError('Only a simple read-only SELECT is allowed: ' + check['id'])
        if not isinstance(check['expected_rows'], list):
            raise ValueError('Expected rows missing: ' + check['id'])
    results = []
    for index, check in enumerate(checks, 1):
        process = subprocess.run(command, input=check['query'], capture_output=True,
                                 text=True, encoding='utf-8', timeout=30,
                                 env=dict(os.environ, MYSQL_PWD=password))
        actual = [r.split('\t') for r in process.stdout.rstrip('\n').split('\n')] if process.stdout else []
        expected = [[str(v) for v in row] for row in check['expected_rows']]
        error = process.stderr.replace(password, '[REDACTED]') if password else process.stderr
        status = 'error' if process.returncode else ('passed' if actual == expected else 'failed')
        results.append({'id': check['id'], 'status': status, 'actual_rows': actual,
                        'expected_rows': expected, 'error': error[:1500]})
        if index % 25 == 0:
            print(json.dumps({'done': index, 'total': len(checks)}), flush=True)
    passed = sum(r['status'] == 'passed' for r in results)
    report = {'suite': suite['name'], 'timestamp_utc': datetime.now(timezone.utc).isoformat(),
              'coverage': 'world-data assertions only', 'gameplay_tested': False,
              'passed': passed, 'total': len(results), 'results': results}
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(json.dumps({k: v for k, v in report.items() if k != 'results'}))
    return 0 if passed == len(results) else 1


if __name__ == '__main__':
    sys.exit(main())
