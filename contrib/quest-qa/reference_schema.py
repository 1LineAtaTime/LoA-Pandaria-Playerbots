"""Preserve audited row-only alias triggers; never clone arbitrary database code."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import socket
from pool import ROOT
from starter_campaign import Database
from evidence import write_json

TABLES={'creature_equip_template','creature_text','gossip_menu','gossip_menu_option','points_of_interest','quest_poi','quest_poi_points'}
SQL=ROOT/'sql/updates/world/2026_11_09_00_world_loa_compat_alias_triggers.sql'


def safe_body(body):
    value=re.sub(r"'(?:''|\\.|[^'\\])*'",'',body)
    value=re.sub(r'\b(?:NEW|OLD)\.`[a-zA-Z0-9_]+`','N',value)
    words=set(re.findall(r'[A-Za-z_]+',value.upper()))
    return words<={'BEGIN','END','IF','NOT','IS','NULL','OR','AND','THEN','SET','CAST','AS','CHAR','IN','N'}


def install(runtime):
    db=Database(runtime/'worldserver.conf',ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
    if db.connections['World'][4] not in ('mop_loa_qa_world','mop_loa_world'):raise RuntimeError('Unexpected world schema')
    text=SQL.read_text();bodies=re.findall(r'FOR EACH ROW (.*?)\$\$',text,re.S)
    if len(bodies)!=14 or not all(safe_body(b) for b in bodies):raise RuntimeError('Alias SQL is not exactly14 audited row-only triggers')
    db.q('World',text)
    return db.q('World',"SELECT TRIGGER_NAME,EVENT_OBJECT_TABLE FROM information_schema.TRIGGERS WHERE TRIGGER_SCHEMA=DATABASE() AND TRIGGER_NAME LIKE 'loa_compat_%' ORDER BY TRIGGER_NAME")


def copy_bot_texts():
    source=Database(ROOT/'exports/worldserver.conf',ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
    target=Database(ROOT/'out/quest-qa-runtime/worldserver.conf',ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
    if target.connections['World'][4]!='mop_loa_qa_world':raise RuntimeError('Reference text copy must target isolated QA')
    columns=['name','text','say_type','reply_type']+['text_loc'+str(i) for i in range(1,9)]
    if target.q('World','SELECT COUNT(*) FROM mop_loa_qa_playerbots.ai_playerbot_texts')[0][0]!='0':return dict(status='already_populated')
    expressions=','.join("IF(`"+c+"` IS NULL,'N',CONCAT('H',HEX(CAST(`"+c+"` AS BINARY))))" for c in columns)
    rows=source.q('World','SELECT '+expressions+' FROM mop_loa_playerbots.ai_playerbot_texts')
    if not rows:raise RuntimeError('Played reference bot-text table is empty')
    # Values below contain only typed hex,never shell text or personal rows.
    def literal(v):return 'NULL' if v=='N' else "CONVERT(X'"+v[1:]+"' USING utf8mb4)"
    values=['('+','.join(literal(v) for v in row)+')' for row in rows]
    target.q('World','INSERT INTO mop_loa_qa_playerbots.ai_playerbot_texts ('+','.join('`'+c+'`' for c in columns)+') VALUES '+','.join(values))
    return dict(status='copied_public_reference_only',rows=len(rows))


def main():
    p=argparse.ArgumentParser();p.add_argument('--export',action='store_true');p.add_argument('--target',choices=['qa','played']);a=p.parse_args()
    if a.export:
        db=Database(ROOT/'exports/worldserver.conf',ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
        rows=db.q('World',"SELECT TRIGGER_NAME,EVENT_OBJECT_TABLE,ACTION_TIMING,EVENT_MANIPULATION,ACTION_STATEMENT FROM information_schema.TRIGGERS WHERE TRIGGER_SCHEMA=DATABASE() AND TRIGGER_NAME LIKE 'loa_compat_%' ORDER BY TRIGGER_NAME")
        if len(rows)!=14:raise RuntimeError('Expected14 compatibility triggers')
        sql=['-- Pure NEW/OLD row-alias synchronization. No definer, schema-qualified calls or external writes.','-- Required by both modern source SQL and legacy native readers.','DELIMITER $$']
        for name,table,timing,event,body in rows:
            if table not in TABLES or timing!='BEFORE' or event not in ('INSERT','UPDATE') or not re.fullmatch(r'loa_compat_[a-z_]+',name) or not safe_body(body):raise RuntimeError('Unreviewed trigger; refusing export')
            sql += [f'DROP TRIGGER IF EXISTS `{name}`$$',f'CREATE TRIGGER `{name}` {timing} {event} ON `{table}` FOR EACH ROW {body}$$']
        sql+=['DELIMITER ;',''];SQL.write_text('\n'.join(sql));print(json.dumps(dict(exported=len(rows),file=str(SQL),sha256=hashlib.sha256(SQL.read_bytes()).hexdigest())));return
    if not a.target:raise RuntimeError('Choose export or a stopped target')
    port=8086 if a.target=='qa' else 8085
    with socket.socket() as s:
        s.settimeout(1)
        if s.connect_ex(('127.0.0.1',port))==0:raise RuntimeError('Gracefully stop the selected realm first')
    rows=install(ROOT/('out/quest-qa-runtime' if a.target=='qa' else 'exports'))
    texts=copy_bot_texts() if a.target=='qa' else dict(status='played_reference_unchanged')
    write_json(ROOT/'out/quest-qa/published-68f8098431'/a.target/'alias-triggers.json',dict(installed=rows,source_sha256=hashlib.sha256(SQL.read_bytes()).hexdigest(),bot_texts=texts))
    print(json.dumps(dict(target=a.target,installed=len(rows))))


if __name__=='__main__':main()
