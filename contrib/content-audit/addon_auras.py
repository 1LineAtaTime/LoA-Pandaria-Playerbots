"""Read-only addon aura reference checks; definitions are not spell-mechanics acceptance."""
import argparse
import hashlib
import json
import re
from pathlib import Path
from catalogue import ROOT, Table, World


def inspect(value, spell_ids):
    tokens = value.split()
    findings = []
    for token in tokens:
        if not re.fullmatch(r'[0-9]+', token) or not 0 < int(token) <= 0xffffffff:
            findings.append(dict(issue='invalid_spell_token', token=token))
        elif int(token) not in spell_ids:
            findings.append(dict(issue='missing_spell_definition', spell=int(token)))
    return dict(formatting_differs=value != ' '.join(tokens), findings=findings)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--config', type=Path, default=ROOT/'exports/worldserver.conf')
    p.add_argument('--client', type=Path, default=ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
    p.add_argument('--data', type=Path, default=ROOT/'exports/dbc')
    p.add_argument('--report', type=Path, default=ROOT/'out/quest-repairs-20261108/addon-auras.json')
    a = p.parse_args(); db = World(a.config, a.client)
    dbc = Table(a.data/'Spell.dbc')
    ids = set(dbc.labels()) | {int(r[0]) for r in db.q('SELECT Id FROM spell_dbc ORDER BY Id')}
    result = dict(read_only=True, gameplay_certified=False, spell_dbc=dbc.meta, tables={}, findings=[])
    for table, key in [('creature_template_addon','entry'),('creature_addon','guid')]:
        rows = db.q('SELECT '+key+",auras FROM "+table+" WHERE auras<>'' ORDER BY "+key)
        formatting = []
        for ident, value in rows:
            checked = inspect(value, ids)
            if checked['formatting_differs']:
                formatting.append(int(ident))
            for finding in checked['findings']:
                result['findings'].append(dict(table=table, id=int(ident), **finding))
        for ident, tier, state in db.q('SELECT '+key+',animTier,standState FROM '+table+' WHERE animTier>=5 OR standState>=10 ORDER BY '+key):
            if int(tier)>=5:
                result['findings'].append(dict(table=table,id=int(ident),issue='invalid_animation_tier',value=int(tier)))
            if int(state)>=10:
                result['findings'].append(dict(table=table,id=int(ident),issue='invalid_stand_state',value=int(state)))
        result['tables'][table] = dict(populated_rows=len(rows), formatting_ids=formatting,
                                       input_sha256=hashlib.sha256(json.dumps(rows).encode()).hexdigest())
    result['passed'] = not result['findings']
    result['limitations'] = 'Does not evaluate spell effects, self-summon recursion, vehicle semantics or actual aura application.'
    a.report.parent.mkdir(parents=True, exist_ok=True)
    a.report.write_text(json.dumps(result, indent=2)+'\n', encoding='utf-8')
    print(json.dumps(dict(passed=result['passed'], findings=len(result['findings']), tables=result['tables'])))
    return int(not result['passed'])


if __name__ == '__main__':
    raise SystemExit(main())
