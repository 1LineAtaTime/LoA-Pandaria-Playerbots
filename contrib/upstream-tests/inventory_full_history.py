"""Inventory ALL SQL/native files at upstream pins, not just selected update batches.

Read-only Git analysis. File/statement presence is not deployment or gameplay proof.
Unknowns deliberately remain unknown; basename matches never establish equivalence.
"""
import argparse
import collections
import csv
import hashlib
import json
from pathlib import Path
import re
import subprocess

from sql_batch import statements

ROOT = Path(__file__).resolve().parents[2]


def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT)


def tree(ref):
    result = {}
    for row in git('ls-tree', '-r', '-z', ref, '--', 'sql', 'src/server').split(b'\0'):
        if not row:
            continue
        meta, path = row.split(b'\t', 1)
        mode, kind, blob = meta.decode().split()
        path = path.decode('utf-8')
        if kind == 'blob' and (path.endswith('.sql') or path.endswith(('.cpp', '.h'))):
            result[path] = blob
    return result


def blobs(ids):
    ids = sorted(set(ids))
    if not ids:
        return {}
    data = subprocess.check_output(
        ['git', 'cat-file', '--batch'], input=('\n'.join(ids)+'\n').encode(), cwd=ROOT)
    offset = 0
    result = {}
    for requested in ids:
        end = data.index(b'\n', offset)
        oid, kind, size = data[offset:end].decode().split()
        if oid != requested or kind != 'blob':
            raise ValueError('Unexpected Git object')
        size = int(size)
        result[oid] = data[end+1:end+1+size]
        offset = end+size+2
    return result


def canonical(statement):
    # Keep quoted contents exact. Whitespace/case outside quotes is insignificant.
    tokens = re.findall(r"'(?:''|\\.|[^'\\])*'|\"(?:\"\"|\\.|[^\"\\])*\"|`(?:``|[^`])*`|\w+|[^\s]", statement, re.S)
    return ' '.join(t if t[0] in "'\"`" else t.lower() for t in tokens)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True)
    a = p.parse_args()
    a.output.mkdir(parents=True, exist_ok=True)
    refs = {'skyfire': git('rev-parse', 'skyfire/main').decode().strip(),
            'loa': git('rev-parse', 'upstream/playerbot-beta').decode().strip()}
    local_pin = git('rev-parse', 'HEAD').decode().strip()
    local = tree(local_pin)
    upstream = {name: tree(pin) for name, pin in refs.items()}
    content = blobs(list(local.values()) + [v for files in upstream.values() for v in files.values()])
    # LF normalization only: whole-file equivalence still requires identical text.
    digest = {oid: hashlib.sha256(raw.replace(b'\r\n', b'\n')).hexdigest() for oid, raw in content.items()}
    local_hashes = collections.defaultdict(list)
    for path, oid in local.items():
        local_hashes[digest[oid]].append(path)
    ledgers = json.loads((ROOT/'doc/release26-quest-batch-ledger-2026-11-06.json').read_text())
    services = json.loads((ROOT/'doc/release26-services-ledger-2026-11-06.json').read_text())['sources']
    reviews = {r['patch']: {'quest': r, 'services': next(s for s in services if s['patch'] == r['patch'])} for r in ledgers}
    rows = []
    for name, files in upstream.items():
        for path, oid in sorted(files.items()):
            copies = local_hashes[digest[oid]]
            status = 'exact_file_present' if copies else ('different_at_same_path' if path in local else 'absent_at_same_path')
            parts = path.split('/')
            archive = '/'.join(parts[:5] if path.startswith('sql/old/5.4.8/world/') else parts[:4] if path.startswith('sql/old/5.4.8/') else parts[:3])
            row = dict(upstream=name, pin=refs[name], path=path, git_blob=oid,
                       sha256_lf=digest[oid], area=archive, source_status=status,
                       exact_local_paths=';'.join(copies), deployment_status='not_established_by_file_comparison')
            rows.append(row)
    fields = list(rows[0])
    with (a.output/'all-source-files.csv').open('w', encoding='utf-8', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=fields); writer.writeheader(); writer.writerows(rows)
    # The previously subset-selected release26 archive gets statement-level evidence.
    lookup = collections.defaultdict(set)
    for path, oid in local.items():
        if path.startswith('sql/updates/world/') and path.endswith('.sql'):
            for stmt in statements(content[oid].decode('utf-8-sig', errors='replace')):
                lookup[canonical(stmt)].add(path)
    detailed = []
    for path, oid in upstream['skyfire'].items():
        if '/SFDB_release_25.000_to_26.001/' not in path or not path.endswith('.sql'):
            continue
        for index, stmt in enumerate(statements(content[oid].decode('utf-8-sig')), 1):
            exact = sorted(lookup.get(canonical(stmt), ()))
            detailed.append(dict(path=path, statement=index,
                sha256=hashlib.sha256(stmt.encode()).hexdigest(),
                exact_local_statement_paths=exact,
                evidence='exact_statement_present_not_deployment' if exact else 'requires_adaptation_or_baseline_reconciliation',
                sql=stmt))
    (a.output/'release26-statements.json').write_text(json.dumps(detailed, indent=2)+'\n', encoding='utf-8')
    (a.output/'release26-subset-reviews.json').write_text(json.dumps(reviews, indent=2)+'\n', encoding='utf-8')
    counts = collections.Counter((r['upstream'], r['area'], r['source_status']) for r in rows)
    summary = dict(local_pin=local_pin, upstream_pins=refs, files=len(rows),
        counts=[dict(upstream=k[0], area=k[1], source_status=k[2], count=v) for k,v in sorted(counts.items())],
        release26_statements=len(detailed),
        release26_exact_statement_presence=sum(bool(r['exact_local_statement_paths']) for r in detailed),
        release26_remaining_requires_reconciliation=sum(not r['exact_local_statement_paths'] for r in detailed),
        scope='All SQL and src/server C++/headers at both pins, including every archived SQL directory. '
              'Content-presence lower bound only; not a list of safe migrations or certified missing features. '
              'Older SQL can be absorbed by a base dump, adapted, superseded or incompatible. '
              'No database access or mutation. Native file differences include intentional custom behavior.')
    (a.output/'summary.json').write_text(json.dumps(summary, indent=2)+'\n', encoding='utf-8')
    print(json.dumps({k:v for k,v in summary.items() if k != 'counts'}))


if __name__ == '__main__':
    main()
