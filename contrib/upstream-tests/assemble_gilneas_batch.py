"""Assemble pinned local Gilneas migrations with explicit schema adaptations."""
import argparse
import hashlib
import json
from pathlib import Path
import re
from sql_batch import statements, comma_values


def assemble(root, manifest):
    output = ['-- Reviewed Gilneas progression batch; back up all affected tables.',
              '-- Requires the ride-snapshot and catapult safety native build.',
              '-- Provenance: contrib/upstream-tests/batches/gilneas-20261105.json.',
              '-- Match the loader optional-orientation contract without rewriting old values.',
              'ALTER TABLE `waypoints` MODIFY COLUMN `orientation` FLOAT NULL DEFAULT NULL;']
    for source in manifest['sources']:
        path = root / source['path']
        text = path.read_text(encoding='utf-8-sig')
        if hashlib.sha256(text.encode()).hexdigest() != source['sha256_lf']:
            raise ValueError('Source changed: '+source['path'])
        output.append('-- Source '+source['path']+'; SHA256(LF) '+source['sha256_lf'])
        for statement in statements(text):
            insert = re.fullmatch(r'INSERT\s+INTO\s+`?creature_text`?\s*\((.*?)\)\s*VALUES\s*(.*)',statement,re.S|re.I)
            if insert:
                columns = comma_values(insert[1])
                identity = next(i for i,c in enumerate(columns) if c.strip('`').lower()=='creatureid')
                if any(c.strip('`').lower()=='entry' for c in columns):
                    raise ValueError('Already adapted source; review manifest')
                rows = []
                for row in comma_values(insert[2]):
                    if not row.startswith('(') or not row.endswith(')'):
                        raise ValueError('Unsupported text INSERT')
                    fields = comma_values(row[1:-1])
                    if len(fields)!=len(columns) or not fields[identity].isdigit():
                        raise ValueError('Malformed creature text row')
                    rows.append('('+','.join([fields[identity]]+fields)+')')
                statement = 'INSERT INTO `creature_text` (`entry`,'+','.join(columns)+') VALUES\n'+',\n'.join(rows)
            elif re.match(r'DELETE\s+FROM\s+`?creature_text`?\b',statement,re.I):
                # Repair legacy rows with one identity column zero. Preflight
                # rejects cross-owned rows where both nonzero IDs disagree.
                statement = re.sub(r'`CreatureID`\s*=\s*(\d+)',
                                   lambda m:'(`CreatureID`='+m[1]+' OR `entry`='+m[1]+')',statement,flags=re.I)
            output.append(statement+';')
    return '\n\n'.join(output)+'\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--manifest',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    manifest = json.loads(args.manifest.read_text(encoding='utf-8-sig'))
    args.output.write_text(assemble(root,manifest),encoding='utf-8')
    print(json.dumps({'sources':len(manifest['sources']),'bytes':args.output.stat().st_size,
                      'sha256':hashlib.sha256(args.output.read_bytes()).hexdigest()}))


if __name__=='__main__':
    main()
