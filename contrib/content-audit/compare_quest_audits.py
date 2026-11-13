"""Fail on new quest dependency findings or lost quest definitions; never certify gameplay."""
import argparse
import csv
import hashlib
import json
from pathlib import Path

TRACKED = ('quests.csv', 'findings.csv', 'repair-queue.csv')
STATUSES = {'structural_failure', 'native_binding_review', 'client_store_review'}


def load(directory):
    directory = Path(directory)
    summary = json.loads((directory / 'summary.json').read_text(encoding='utf-8-sig'))
    if summary.get('version') != 1 or summary.get('read_only') is not True:
        raise ValueError('Unsupported or unlabelled audit: ' + str(directory))
    data = {}
    for name in TRACKED:
        raw = (directory / name).read_bytes()
        if hashlib.sha256(raw).hexdigest() != summary['output_hashes'][name]:
            raise ValueError('Artifact hash mismatch: ' + str(directory / name))
        with (directory / name).open(encoding='utf-8-sig', newline='') as f:
            data[name] = list(csv.DictReader(f))
        if any(None in r or None in r.values() for r in data[name]):
            raise ValueError('Malformed CSV: ' + name)
    for name, key in [('quests.csv', 'quests'), ('findings.csv', 'findings'),
                      ('repair-queue.csv', 'repair_queue_entries')]:
        if len(data[name]) != summary[key]:
            raise ValueError('Row count mismatch: ' + name)
    quests = {int(r['quest']) for r in data['quests.csv']}
    if len(quests) != len(data['quests.csv']):
        raise ValueError('Duplicate quest IDs')
    if any(r['status'] not in STATUSES for r in data['findings.csv']):
        raise ValueError('Unknown finding status')
    return dict(summary=summary, quests=quests, findings=data['findings.csv'])


def compare(before, after):
    if before['summary']['version'] != after['summary']['version']:
        raise ValueError('Audit formats differ')
    fields = ('quest', 'issue', 'role', 'target_type', 'target_id', 'source', 'status')

    def keyed(data):
        return {tuple(str(r[k]) for k in fields): r for r in data['findings']}

    old, new = keyed(before), keyed(after)
    added = [new[k] for k in sorted(new.keys() - old.keys())]
    removed = [old[k] for k in sorted(old.keys() - new.keys())]
    lost = sorted(before['quests'] - after['quests'])
    return dict(regression_free=not added and not lost, new_findings=added,
                absent_previous_findings=removed, removed_quest_definitions=lost,
                added_quest_definitions=sorted(after['quests'] - before['quests']),
                gameplay_certified=False,
                note='Absent findings may reflect a repair, binding deletion or log change; review before declaring resolved.')


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('baseline', type=Path)
    p.add_argument('current', type=Path)
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()
    result = compare(load(a.baseline), load(a.current))
    a.report.parent.mkdir(parents=True, exist_ok=True)
    a.report.write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    print(json.dumps({k: len(v) if isinstance(v, list) else v for k, v in result.items()}))
    return int(not result['regression_free'])


if __name__ == '__main__':
    raise SystemExit(main())
