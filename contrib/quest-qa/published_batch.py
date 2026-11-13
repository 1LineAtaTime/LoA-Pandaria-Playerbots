"""Back up and apply the exact pinned LoA SQL range to a stopped owned realm.

Only the world schema is mutated. Source commits and individual SQL hashes are
retained. Native deployment is separate; this command never restarts a server.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import socket
import subprocess
from pool import ROOT
from starter_campaign import Database
from evidence import write_json

BASE='8a26cd41fece10e18335ebc078f6ff7e99412c2d'
TIP='68f8098431'


def source_files(base=BASE,tip=TIP):
    names=subprocess.check_output(['git','diff','--name-only',base+'..'+tip,'--','sql/updates/world'],cwd=ROOT,text=True).splitlines()
    result=[]
    for name in names:
        path=(ROOT/name).resolve()
        if not path.is_relative_to(ROOT/'sql/updates/world') or not path.is_file():raise RuntimeError('Unexpected source SQL path')
        published=subprocess.check_output(['git','show',tip+':'+name],cwd=ROOT)
        if path.read_bytes().replace(b'\r\n',b'\n')!=published.replace(b'\r\n',b'\n'):raise RuntimeError('Working SQL differs from pinned published source')
        result.append(dict(name=name,sha256=hashlib.sha256(path.read_bytes()).hexdigest()))
    return result


def tables(files):
    found=set()
    for item in files:
        sql=(ROOT/item['name']).read_text(encoding='utf-8-sig')
        sql=re.sub(r"'(?:''|\\.|[^'\\])*'","''",sql,flags=re.S)
        sql=re.sub(r'--[^\n]*','',sql)
        if re.search(r'\b(DROP\s+DATABASE|TRUNCATE|USE\s+|CREATE\s+USER|GRANT\s+)',sql,re.I):raise RuntimeError('Unexpected global operation in source SQL')
        found.update(re.findall(r'\b(?:UPDATE|INTO|FROM)\s+`?([a-z][a-z0-9_]*)`?',sql,re.I))
    return sorted(found)


def main():
    p=argparse.ArgumentParser();p.add_argument('--target',choices=['qa','played'],required=True);p.add_argument('--apply',action='store_true');p.add_argument('--refresh-backup',action='store_true');p.add_argument('--replay',action='store_true');p.add_argument('--verify-repeat',action='store_true')
    p.add_argument('--base',default=BASE);p.add_argument('--tip',default=TIP);a=p.parse_args()
    base=subprocess.check_output(['git','rev-parse','--verify',a.base+'^{commit}'],cwd=ROOT,text=True).strip()
    tip=subprocess.check_output(['git','rev-parse','--verify',a.tip+'^{commit}'],cwd=ROOT,text=True).strip()
    runtime=ROOT/('out/quest-qa-runtime' if a.target=='qa' else 'exports');port=8086 if a.target=='qa' else 8085
    db=Database(runtime/'worldserver.conf',ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
    host,dbport,user,password,schema=db.connections['World'];expected='mop_loa_qa_world' if a.target=='qa' else 'mop_loa_world'
    if schema!=expected:raise RuntimeError('Unexpected world schema; refusing mutation')
    folder=ROOT/'out/quest-qa'/('published-'+tip[:10])/a.target;folder.mkdir(parents=True,exist_ok=True);manifest=folder/'migration.json';files=source_files(base,tip);affected=tables(files)
    if not files:raise RuntimeError('Pinned SQL range is empty')
    report=json.loads(manifest.read_text()) if manifest.exists() else dict(base=base,tip=tip,target=schema,files=files,affected_tables=affected,applied=[])
    if report['base']!=base or report['tip']!=tip:raise RuntimeError('Pinned range differs from the recorded source manifest')
    if report['files']!=files:raise RuntimeError('SQL differs from the recorded source manifest')
    report['affected_tables']=affected
    backup=folder/'world-before.sql'
    if a.refresh_backup:
        if report['applied']:raise RuntimeError('Cannot replace a pre-change backup after applying SQL')
        if report.get('backup_sha256'):
            old=report['backup_sha256'];backup.rename(folder/('world-before.'+old+'.sql'));write_json(folder/('migration-backup.'+old+'.json'),report)
            report.pop('backup_sha256');report.pop('backup_bytes')
    if not report.get('backup_sha256'):
        if backup.exists():raise RuntimeError('Unjournaled backup exists; inspect it before recovery')
        client=ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb-dump.exe'
        backup_user=os.environ.get('MOP_BACKUP_USER',user);backup_password=os.environ.get('MOP_BACKUP_PASSWORD',password)
        cmd=[str(client),'--no-defaults','--protocol=TCP','--host='+host,'--port='+dbport,'--user='+backup_user,'--lock-tables','--quick','--routines','--triggers','--events','--hex-blob','--default-character-set=utf8mb4',schema]
        with backup.open('wb') as f:r=subprocess.run(cmd,stdout=f,stderr=subprocess.PIPE,env=dict(os.environ,MYSQL_PWD=backup_password),timeout=900)
        if r.returncode:raise RuntimeError(r.stderr.decode(errors='replace').replace(password,'[redacted]'))
        if backup.stat().st_size<1000000:raise RuntimeError('World backup unexpectedly small')
        report.update(backup_sha256=hashlib.sha256(backup.read_bytes()).hexdigest(),backup_bytes=backup.stat().st_size,backup_note='Read-locked world tables,including MyISAM; no personal auth/character copy')
        report['table_engines']=db.q('World','SELECT ENGINE,COUNT(*) FROM information_schema.TABLES WHERE TABLE_SCHEMA=DATABASE() GROUP BY ENGINE');write_json(manifest,report)
    if not a.apply:print(json.dumps(dict(target=schema,sql_files=len(files),backup_bytes=report['backup_bytes'],manifest=str(manifest))));return
    if hashlib.sha256(backup.read_bytes()).hexdigest()!=report['backup_sha256']:raise RuntimeError('Backup hash changed')
    with socket.socket() as s:
        s.settimeout(1)
        if s.connect_ex(('127.0.0.1',port))==0:raise RuntimeError('Realm is still listening; gracefully stop it before applying paired SQL')
    before_repeat=db.q('World','CHECKSUM TABLE '+','.join('`'+t+'`' for t in affected)+' EXTENDED') if a.verify_repeat else None
    for item in files:
        if item['name'] in report['applied'] and not a.replay:continue
        sql=(ROOT/item['name']).read_text(encoding='utf-8-sig');db.q('World',sql)
        if item['name'] not in report['applied']:report['applied'].append(item['name'])
        write_json(manifest,report)
    checks=db.q('World','CHECKSUM TABLE '+','.join('`'+t+'`' for t in affected)+' EXTENDED')
    report['after_checksums']=checks;report['status']='APPLIED_NATIVE_PAIR_REQUIRED';write_json(manifest,report)
    if a.verify_repeat:
        report['repeat_stable']=before_repeat==checks;write_json(manifest,report)
        if not report['repeat_stable']:raise RuntimeError('Published replay changed table checksums; review the retained manifests')
    print(json.dumps(dict(target=schema,applied=len(report['applied']),affected_tables=len(affected),manifest=str(manifest))))


if __name__=='__main__':main()
