"""Account for the previously reported 1,012 predicates and380 field assignments.

Joins pinned source identities to the extended plan's dispositions. ACCOUNTED
means reviewed, not deployed or gameplay verified. No database writes.
"""
from pathlib import Path
import collections
import json
import re

from inventory_full_history import tree, blobs
from assemble_historical_quest_fields import ids
from assemble_release26_quest_batch import split_where
from sql_batch import statements,comma_values

ROOT=Path(__file__).resolve().parents[2]


def main():
    folder=ROOT/'out/full-history-20261109/remaining'
    plan=json.loads((folder/'extended/report.json').read_text(encoding='utf8'))
    baseline=json.loads((ROOT/'doc/historical-quest-field-batch-2026-11-09.json').read_text(encoding='utf8'))
    fields={key.split(':',1)[1] for key in baseline['excluded'] if key.startswith('unmapped_field:')}
    by_statement=collections.defaultdict(collections.Counter)
    by_field=collections.defaultdict(collections.Counter)
    for row in plan['results']:
        for src in row['sources']:
            if 'path' not in src or 'statement' not in src:continue
            key=(src['path'],src['statement'])
            by_statement[key][row['status']]+=1
            by_field[key+(row['source_field'].lower(),)][row['status']]+=1
    for row in plan['predicate_dispositions']:
        by_statement[(row['path'],row['statement'])][row['status']]+=1
    predicates=[]
    files=tree(plan['pin']);paths=[p for p in files if p.startswith('sql/old/5.4.8/world/') or p.startswith('sql/updates/world/')];data=blobs(files[p] for p in paths)
    mappings=[]
    for path in paths:
        text=data[files[path]].decode('utf-8-sig',errors='replace')
        if not re.search(r'UPDATE\s+`?quest_template`?\s+SET',text,re.I):continue
        for ordinal,cmd in enumerate(statements(text),1):
            match=re.match(r'UPDATE\s+`?quest_template`?\s+SET\s+(.*)',cmd,re.I|re.S)
            if not match:continue
            try:body,where=split_where(match[1])
            except ValueError:continue
            if not ids(where):
                outcome=dict(by_statement.get((path,ordinal),{}))
                predicates.append(dict(path=path,statement=ordinal,predicate=where,dispositions=outcome,status='ACCOUNTED' if outcome else 'UNRESOLVED'))
                continue
            names=[]
            for assignment in comma_values(body):
                field=re.match(r'\s*`?(\w+)`?\s*=',assignment)
                if field:names.append(field[1])
            for field in names:
                if field not in fields:continue
                outcome=dict(by_field.get((path,ordinal,field.lower()),{}))
                if field=='WDBVerified':outcome={'metadata_only_no_gameplay_migration':1}
                mappings.append(dict(path=path,statement=ordinal,field=field,dispositions=outcome,status='ACCOUNTED' if outcome else 'UNRESOLVED'))
    if len(predicates)!=1012 or len(mappings)!=380:raise RuntimeError('Historical input count changed; inspect scope before publishing')
    report=dict(pin=plan['pin'],conditional_inputs=len(predicates),field_assignment_inputs=len(mappings),
        unresolved_predicates=sum(r['status']=='UNRESOLVED' for r in predicates),
        unresolved_field_assignments=sum(r['status']=='UNRESOLVED' for r in mappings),
        predicates=predicates,field_assignments=mappings,
        scope='Every counted input has a disposition; preserved/retired/metadata-only is not an applied fix. Native/content gaps outside this quest SQL scope remain separate.')
    path=ROOT/'doc/remaining-quest-input-dispositions-2026-11-09.json';path.write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
    print(json.dumps({k:v for k,v in report.items() if k not in ('predicates','field_assignments')}))


if __name__=='__main__':main()
