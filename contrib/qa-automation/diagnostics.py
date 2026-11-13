"""Private diagnostic snapshots, preserving content IDs while grouping volatile GUIDs."""
import hashlib
import re
from collections import Counter
from pathlib import Path

HEADER = re.compile(r'^\S+\s+(WARN|ERROR|FATAL)\s+\[([^]]+)\]\s*(.*)')


def normalize(message):
    # Do not normalize spell, entry, quest, path or error IDs: those identify new failures.
    return re.sub(r'\bGUID\s+\d+', 'GUID <guid>', message)


def category(message):
    s = message.lower()
    if any(x in s for x in ('access violation', 'assertion', 'stack overflow', 'segmentation fault')):
        return 'crash', 0
    if 'deadlock' in s or '[1213]' in s:
        return 'database_deadlock', 1
    if 'sql ' in s or '[sql]' in s or '[error]:' in s:
        return 'database', 1
    if 'no core script found' in s:
        return 'missing_script', 2
    if any(x in s for x in ('mmap', 'spline', 'taxi')):
        return 'movement_navigation', 2
    if 'spell' in s or 'aura' in s:
        return 'spell_aura', 2
    if 'quest' in s or 'phase' in s or 'zoneid' in s:
        return 'quest_phase', 2
    return 'other', 3


def parse(text):
    records = []
    for line in text.splitlines():
        match = HEADER.match(line)
        if match:
            severity, logger, message = match.groups()
            if message.startswith('SQL('):
                # Retain operation/table, never SQL values (which may contain private data).
                target = re.search(r'\b(INSERT INTO|REPLACE INTO|UPDATE|DELETE FROM)\s+`?([\w.]+)', message, re.I)
                message = 'SQL ' + (' '.join(target.groups()) if target else 'statement')
            records.append(dict(severity=severity, logger=logger, message=message))
        elif records and '[ERROR]:' in line:
            # The MariaDB code/detail belongs to the preceding statement.
            records[-1]['message'] += ' ' + line.strip()
    counts = Counter()
    for r in records:
        signature = '[' + r['logger'] + '] ' + normalize(r['message'])
        counts[(r['severity'], signature)] += 1
    return [dict(severity=s, signature=m, count=n, category=category(m)[0], priority=0 if s=='FATAL' else category(m)[1])
            for (s, m), n in sorted(counts.items())]


def snapshot(directory):
    out = {}
    for name in ('Errors.txt', 'AuthErrors.txt', 'DBErrors.log', 'Server.log'):
        p = Path(directory) / name
        if p.exists():
            raw = p.read_bytes()
            out[name] = dict(bytes=len(raw), sha256=hashlib.sha256(raw).hexdigest(),
                             records=parse(raw.decode('utf-8', errors='replace')))
    return out


def compare(before, after):
    def merge(data):
        # An error can be routed to both Server and Errors appenders. Take the maximum
        # per-file frequency, rather than counting the same diagnostic twice.
        result = {}
        for f in data.values():
            for r in f['records']:
                k = (r['severity'], r['signature'])
                if k not in result or r['count'] > result[k]['count']:
                    result[k] = r
        return result
    old, new = merge(before), merge(after)
    added = [r for k, r in new.items() if k not in old]
    recurring = [dict(r, additional_occurrences=r['count'] - old[k]['count'])
                 for k, r in new.items() if k in old and r['count'] > old[k]['count']]
    order = lambda r: (r['priority'], -r.get('additional_occurrences', r['count']), r['signature'])
    return dict(new_signatures=sorted(added, key=order), recurring=sorted(recurring, key=order),
                baseline_absent=[r for k, r in old.items() if k not in new],
                requires_review=bool(added),
                note='Counts are bounded snapshots, not rates; log rotation can reduce counts. Absent does not mean fixed.')
