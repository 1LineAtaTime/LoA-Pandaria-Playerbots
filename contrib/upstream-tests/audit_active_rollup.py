"""Read-only comparison of historical adapted SF ledgers against the ACTIVE world.

Historical deployment labels are not evidence after restoring another database.
This checks data contracts, not quest/spell behavior or whole-tree equivalence.
"""
import argparse
import collections
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'contrib/content-audit'))
from catalogue import World


def ident(value):
    if not re.fullmatch(r'[A-Za-z_][A-Za-z0-9_]*', value):
        raise ValueError('Invalid ledger identifier')
    return '`' + value + '`'


def literal(value):
    if value == 'N':
        return 'NULL'
    if not value.startswith('H') or not re.fullmatch('[0-9a-fA-F]*', value[1:]):
        raise ValueError('Invalid hex ledger value')
    return "CONVERT(X'" + value[1:] + "' USING utf8mb4)"


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()
    db = World(ROOT/'exports/worldserver.conf', ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
    schema = db.q('SELECT DATABASE()')[0][0]
    columns = collections.defaultdict(dict)
    for table, column, kind in db.q('SELECT TABLE_NAME,COLUMN_NAME,DATA_TYPE FROM information_schema.COLUMNS WHERE TABLE_SCHEMA=DATABASE()'):
        columns[table.lower()][column.lower()] = kind
    checks, results = [], []

    def append(source, index, table, values, absent=False):
        missing = [c for c in values if c.lower() not in columns[table.lower()]]
        row = dict(source=source, index=index, table=table, absence_expected=absent)
        if missing:
            row.update(status='schema_gap', columns=missing)
            results.append(row)
            return
        predicates = []
        for column, value in values.items():
            name = ident(column)
            encoded = re.fullmatch(r"CONVERT\(X'([0-9a-fA-F]*)' USING utf8mb4\)", value)
            decoded = bytes.fromhex(encoded[1]).decode('utf-8') if encoded else value
            if columns[table.lower()][column.lower()] == 'set' and re.fullmatch(r'[0-9]+',decoded):
                predicates.append(f'({name}+0) <=> {decoded}')
            elif columns[table.lower()][column.lower()] in ('float', 'double') and value.upper() != 'NULL':
                predicates.append(f'ABS({name}-({value}))<=GREATEST(0.00001,ABS({value})*0.000001)')
            elif columns[table.lower()][column.lower()] in ('char','varchar','text','tinytext','mediumtext','longtext','enum','set'):
                predicates.append(f'BINARY {name} <=> BINARY ({value})')
            else:
                predicates.append(f'{name} <=> {value}')
        row['status'] = 'pending'
        n = len(results)
        results.append(row)
        checks.append((n, f"SELECT {n},EXISTS(SELECT 1 FROM {ident(table)} WHERE " + ' AND '.join(predicates) + ')'))

    for stem in ('release26-services-ledger-2026-11-06', 'sf27-native-spell-data', 'sf27-interaction-data', 'sf27-typed-loot-data'):
        data = json.loads((ROOT/'doc'/f'{stem}.json').read_text(encoding='utf-8'))
        for n, row in enumerate(data['desired']):
            # Values are published, tracked SQL literals; turn strings into hex
            # before passing through the strict single-SELECT reader.
            values = {}
            for key, value in row['values'].items():
                if value.startswith("'") and value.endswith("'"):
                    text = value[1:-1]
                    text = re.sub(r"\\(.)", lambda m: {'n':'\n','r':'\r','t':'\t','0':'\0'}.get(m[1],m[1]), text)
                    text = text.replace("''", "'")
                    values[key] = "CONVERT(X'" + text.encode('utf-8').hex() + "' USING utf8mb4)"
                elif re.fullmatch(r'-?[0-9]+(?:\.[0-9]+)?|0x[0-9a-f]+|NULL', value, re.I):
                    values[key] = value
                else:
                    raise ValueError('Nonliteral desired value: ' + stem)
            append(stem, n, row['table'], values)
    stem = 'sf27-remaining-content-data'
    for n, row in enumerate(json.loads((ROOT/'doc'/f'{stem}.json').read_text())['changes']):
        values = row.get('after') or row['before']
        if row['operation']=='update':
            before=row['before']; keys=row.get('keys') or []
            values={c:v for c,v in values.items() if c in keys or before.get(c)!=v}
        append(stem, n, row['table'], {c:literal(v) for c,v in values.items()}, row.get('after') is None)
    stem = 'sf27-authored-damage-data'
    for n, row in enumerate(json.loads((ROOT/'doc'/f'{stem}.json').read_text())['values']):
        append(stem, n, 'creature_template', dict(entry=str(row['entry']), sf27_damage_multiplier=str(row.get('multiplier',row.get('value')))))
    for start in range(0, len(checks), 100):
        for n, exists in db.q(' UNION ALL '.join(query for _,query in checks[start:start+100])):
            row = results[int(n)]
            row['status'] = 'match' if (exists == '0') == row['absence_expected'] else 'differs'
    summary = collections.defaultdict(collections.Counter)
    for row in results:
        summary[row['source']][row['status']] += 1
    report = dict(schema=schema, scope='Historical adapted data contracts only; differences require reconciliation, not blind replay. No gameplay acceptance implied.',
                  summary={k:dict(v) for k,v in summary.items()}, checks=len(results), results=results)
    a.report.parent.mkdir(parents=True,exist_ok=True)
    a.report.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(dict(schema=schema,checks=len(results),summary=report['summary'])))


if __name__ == '__main__':
    main()
