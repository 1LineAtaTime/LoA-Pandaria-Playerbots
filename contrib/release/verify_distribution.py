"""Verify contribution SQL checksums and public-tree hygiene without credentials."""
import csv
import hashlib
import json
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def main():
    rows = list(csv.DictReader((ROOT / 'database/contribution-migrations.csv').open(encoding='utf-8')))
    for row in rows:
        path = (ROOT / row['relative_file']).resolve()
        assert path.is_relative_to(ROOT), 'SQL path escapes checkout'
        assert path.is_file(), 'Missing SQL: ' + row['relative_file']
        assert hashlib.sha256(path.read_bytes()).hexdigest() == row['sha256'], 'Changed SQL: ' + row['relative_file']
    names = subprocess.check_output(['git', 'ls-files', '-z', '--cached', '--others', '--exclude-standard'], cwd=ROOT).decode().split('\0')
    findings = []
    count = 0
    for name in sorted(set(filter(None, names))):
        path = ROOT / name
        if not path.is_file():
            continue
        if path.suffix.lower() in {'.exe', '.dll', '.pdb', '.dmp', '.key', '.pem', '.pfx', '.p12', '.conf'}:
            findings.append({'path': name, 'type': 'runtime-or-secret-file'})
        try:
            text = path.read_text(encoding='utf-8-sig')
        except UnicodeError:
            continue
        count += 1
        for kind, pattern in (
            ('private-key', r'-----BEGIN (?:RSA |EC |OPENSSH |ENCRYPTED )?PRIVATE KEY-----'),
            ('token', r'\b(?:ghp|gho|ghu|ghs|ghr)_[A-Za-z0-9]{20,}\b|\bgithub_pat_[A-Za-z0-9_]{30,}\b'),
            ('url-credential', r'https?://[^\s/@:]+:[^\s/@]+@'),
            ('local-path', r'(?:C:|G:)[\\/]+(?:Users|Codex Stuff|World of Warcraft|Downloads)'),
        ):
            # Pattern definitions in this verifier are not credentials or paths.
            if name != 'contrib/release/verify_distribution.py' and re.search(pattern, text):
                findings.append({'path': name, 'type': kind})
        for match in re.finditer(r'^\s*\w*DatabaseInfo\s*=\s*"([^"]+)"', text, re.M):
            parts = match[1].split(';')
            if len(parts) == 5 and parts[3] not in {'CHANGE_ME', 'password', ''}:
                findings.append({'path': name, 'type': 'database-password'})
    report = {'passed': not findings, 'sql_files_verified': len(rows), 'text_files_scanned': count, 'findings': findings}
    print(json.dumps(report, indent=2))
    return 0 if report['passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
