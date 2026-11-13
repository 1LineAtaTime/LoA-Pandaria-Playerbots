"""Read-only inventory of unresolved native names from one server error log.

Definition text is a triage lead, never proof of a compiled/registered script.
No aliases are applied and no database bindings are removed by this tool.
"""
import argparse, collections, hashlib, json, re
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--error-log',type=Path,required=True)
    p.add_argument('--out',type=Path,required=True)
    a=p.parse_args();raw=a.error_log.read_bytes()
    names=sorted(set(re.findall(r"ScriptName '([^']+)' exists in database, but no core script found!",raw.decode('utf-8-sig',errors='replace'))))
    definitions=collections.defaultdict(list)
    pattern=re.compile(r'(?:CreatureScript|GameObjectScript|SpellScriptLoader|AreaTriggerScript|AreaTriggerEntityScript|AchievementCriteriaScript|InstanceMapScript)\("([^"]+)"')
    for path in (ROOT/'src/server/scripts').rglob('*.cpp'):
        source=path.read_text(encoding='utf-8',errors='replace')
        for match in pattern.finditer(source):
            if match[1] in names:
                definitions[match[1]].append(dict(file=path.relative_to(ROOT).as_posix(),line=source.count('\n',0,match.start())+1))
    report=dict(source_error_log_sha256=hashlib.sha256(raw).hexdigest(),
                scope='Only missing-name errors in the supplied log; not all broken content or all historical patches',
                missing_name_count=len(names),
                categories=dict(collections.Counter(n.split('_',1)[0] for n in names)),
                definition_text_is_not_registration_proof=True,
                full_gameplay_certified=False,
                missing=[dict(name=n,definition_text=definitions[n],status='REQUIRES_SEMANTIC_RECONCILIATION') for n in names])
    a.out.parent.mkdir(parents=True,exist_ok=True)
    a.out.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({k:report[k] for k in ('missing_name_count','categories')}))
if __name__=='__main__':main()
