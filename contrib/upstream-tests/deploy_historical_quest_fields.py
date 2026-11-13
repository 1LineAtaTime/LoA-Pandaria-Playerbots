"""Apply the reviewed historical quest batch to a stopped, named local world.

Backs up only affected public world tables. Never mutates auth/characters or
retries an incomplete mutation. Refuses unjournaled or repeated deployment.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import socket
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'contrib/headless-client'))
from starter_campaign import Database


def prerequisite_cycles(graph):
    done=set();result=set()
    for start in graph:
        path=[];seen={};node=start
        while node>0 and node in graph and node not in done:
            if node in seen:
                result.add(tuple(sorted(path[seen[node]:])));break
            seen[node]=len(path);path.append(node);node=graph[node]
        done.update(path)
    return result


def snapshot(db, changes):
    groups={}
    for row in changes:groups.setdefault(row['table'],set()).add(row['column'])
    result={}
    for table,columns in groups.items():
        columns=sorted(columns);qids=sorted({r['quest'] for r in changes if r['table']==table})
        query='SELECT ID,'+','.join("IF(`"+c+"` IS NULL,'N',CONCAT('H',HEX(CAST(`"+c+'` AS BINARY))))' for c in columns)+' FROM `'+table+'` WHERE ID IN ('+','.join(map(str,qids))+')'
        for data in db.q('World',query):
            for column,value in zip(columns,data[1:]):result[(table,int(data[0]),column)]=value
    return result


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--target',choices=['qa','played'],required=True)
    p.add_argument('--plan-dir',type=Path,default=ROOT/'out/full-history-20261109/quest-fields-expanded')
    p.add_argument('--migration',type=Path,default=ROOT/'sql/updates/world/2026_11_09_02_world_historical_quest_fields.sql')
    p.add_argument('--schema-migration',type=Path)
    a=p.parse_args();folder=a.plan_dir.resolve();source=json.loads((folder/'report.json').read_text(encoding='utf8'))
    patch=a.migration.resolve()
    if hashlib.sha256(patch.read_bytes()).hexdigest()!=source['sql_sha256']:raise RuntimeError('Reviewed SQL hash changed')
    changes=[r for r in source['results'] if r['status']=='prepared_change'];assert changes
    tables=sorted({r['table'] for r in changes})
    if not set(tables)<={'quest_template','quest_template_addon','quest_offer_reward','quest_request_items','quest_details'}:raise RuntimeError('Unexpected table')
    schema_sql=None
    if source.get('requires_signed_next'):
        if not a.schema_migration:raise RuntimeError('Matching signed schema/native migration required')
        schema_sql=a.schema_migration.read_text(encoding='utf8')
    runtime=ROOT/('out/quest-qa-runtime' if a.target=='qa' else 'exports');port=8086 if a.target=='qa' else 8085
    with socket.socket() as sock:
        sock.settimeout(.5)
        if sock.connect_ex(('127.0.0.1',port))==0:raise RuntimeError('Stop this world gracefully before deploying')
    client=ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe';db=Database(runtime/'worldserver.conf',client)
    host,dbport,user,password,schema=db.connections['World']
    if schema!=('mop_loa_qa_world' if a.target=='qa' else 'mop_loa_world'):raise RuntimeError('Unexpected world schema')
    if schema_sql and int(db.q('World','SELECT COALESCE(MAX(NextQuestID),0) FROM quest_template_addon')[0][0])>2147483647:
        raise RuntimeError('Unsigned link exceeds the signed native range')
    private=folder/(a.target+'-compatible');private.mkdir(exist_ok=True);manifest=private/'deployment.json';backup=private/'quest-tables-before.sql'
    if manifest.exists() or backup.exists():raise RuntimeError('Deployment journal/backup exists; inspect before any retry')
    before=snapshot(db,changes)
    for row in changes:
        key=(row['table'],row['quest'],row['column'])
        if before.get(key) not in (row['before'],row['after']):raise RuntimeError('Changed precondition: '+str(key))
    graph={int(q):int(prev) for q,prev in db.q('World','SELECT ID,PrevQuestID FROM quest_template_addon')}
    projected=graph.copy()
    for row in changes:
        if row['source_field']=='PrevQuestId':projected[row['quest']]=int(bytes.fromhex(row['after'][1:]).decode())
    prior_cycles=prerequisite_cycles(graph);next_cycles=prerequisite_cycles(projected)
    if next_cycles-prior_cycles:raise RuntimeError('New positive prerequisite cycles: '+str(sorted(next_cycles-prior_cycles)))
    cmd=[str(client.with_name('mariadb-dump.exe')),'--no-defaults','--protocol=TCP','--host='+host,'--port='+dbport,'--user='+user,'--lock-tables','--hex-blob',schema,*tables]
    with backup.open('wb') as f:result=subprocess.run(cmd,stdout=f,stderr=subprocess.PIPE,env=dict(os.environ,MYSQL_PWD=password),timeout=180)
    if result.returncode:raise RuntimeError('Table backup failed; no migration executed')
    if backup.stat().st_size<1000000:raise RuntimeError('Unexpectedly short quest table backup')
    already=sum(before.get((r['table'],r['quest'],r['column']))==r['after'] for r in changes)
    report=dict(schema=schema,tables=tables,fields=len(changes),quests=len({r['quest'] for r in changes}),already_at_target=already,state='BACKED_UP',backup_bytes=backup.stat().st_size,backup_sha256=hashlib.sha256(backup.read_bytes()).hexdigest(),sql_sha256=source['sql_sha256'])
    report.update(prerequisite_cycles_before=len(prior_cycles),prerequisite_cycles_after=len(next_cycles))
    def save():manifest.write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
    if schema_sql:report['schema_sql_sha256']=hashlib.sha256(a.schema_migration.read_bytes()).hexdigest()
    save()
    if schema_sql:db.q('World',schema_sql)
    db.q('World',patch.read_text(encoding='utf-8-sig'))
    after=snapshot(db,changes)
    for row in changes:
        key=(row['table'],row['quest'],row['column'])
        if after.get(key)!=row['after']:raise RuntimeError('Postcondition failed: '+str(key))
    report['state']='APPLIED_FIELDS_VERIFIED';save()
    checksum='CHECKSUM TABLE '+','.join('`'+t+'`' for t in tables)+' EXTENDED'
    first=db.q('World',checksum);db.q('World',patch.read_text(encoding='utf-8-sig'))
    if first!=db.q('World',checksum):raise RuntimeError('Repeat altered quest tables; inspect journal')
    report.update(state='APPLIED_REPEAT_STABLE',checksums=first);save()
    print(json.dumps(report))


if __name__=='__main__':main()
