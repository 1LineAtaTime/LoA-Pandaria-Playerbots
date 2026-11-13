"""Retain explicit local quest identity when assembling older SF field repairs.

Run after the extended assembler, before deployment. No database writes. The
prior batch's exact old/new values make the corrective migration reviewable.
"""
from pathlib import Path
import hashlib
import json

ROOT=Path(__file__).resolve().parents[2]


def matching_assembly(current,original,sql_sha256):
    candidate=current if 'identity_restore_fields' not in current else original
    if not candidate or candidate.get('sql_sha256')!=sql_sha256:
        raise RuntimeError('Assembly SQL/report differ; regenerate the extended assembly before finalizing')
    return candidate


def main():
    folder=ROOT/'out/full-history-20261109/remaining/extended'
    original=folder/'generated-report-before-identity-restore.json'
    current=json.loads((folder/'report.json').read_text(encoding='utf8'))
    cached=json.loads(original.read_text(encoding='utf8')) if original.exists() else None
    raw=(folder/'historical-quest-fields.sql').read_bytes()
    report=matching_assembly(current,cached,hashlib.sha256(raw).hexdigest())
    original.write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
    prior_path=ROOT/'doc/historical-quest-field-batch-2026-11-09.json'
    prior=json.loads(prior_path.read_text(encoding='utf8'))
    conflicts={14009,14012,14274,14279,14283,24784,29067}
    changes=[]
    sql=(folder/'historical-quest-fields.sql').read_text(encoding='utf8')
    sql+='\n-- Restore local lesson/profession chains where the older reference identifies a different quest.\n'
    for old in prior['changes']:
        if old['quest'] not in conflicts:continue
        if not old['before'] or not old['before'].startswith('H'):raise RuntimeError('Original field bytes required')
        row=dict(quest=old['quest'],source_field=old['source_field'],table=old['table'],column=old['column'],before=old['after'],after=old['before'],status='prepared_change',sources=[dict(disposition='restore_local_quest_identity',prior_ledger=prior_path.name)])
        changes.append(row);c=row['column'];t=row['table'];qid=row['quest'];value=row['after'][1:]
        sql+=f"UPDATE `{t}` SET `{c}`=CONVERT(X'{value}' USING utf8mb4) WHERE ID={qid} AND IF(`{c}` IS NULL,'N',CONCAT('H',HEX(CAST(`{c}` AS BINARY))))='{row['before']}';\n"
    report['results'].extend(changes);report['identity_restore_fields']=len(changes)
    report['prior_ledger_sha256']=hashlib.sha256(prior_path.read_bytes()).hexdigest()
    report['counts']['prepared_change']+=len(changes)
    migration=ROOT/'sql/updates/world/2026_11_09_04_world_historical_quest_compatibility.sql'
    migration.write_text(sql,encoding='utf8');report['sql_sha256']=hashlib.sha256(migration.read_bytes()).hexdigest()
    report['required_native_source']='05e94aabf6'
    (folder/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
    (folder/'identity-restores.json').write_text(json.dumps(changes,indent=2)+'\n',encoding='utf8')
    print(json.dumps(dict(fields=report['counts']['prepared_change'],identity_restores=len(changes),sql_sha256=report['sql_sha256'])))


if __name__=='__main__':main()
