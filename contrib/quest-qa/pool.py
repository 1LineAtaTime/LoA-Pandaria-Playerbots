"""Reusable QA identities, account leases and offline character-state checkpoints.

All mutation connections must target the owned isolated mop_loa_qa_ schemas.
Unsupported shared group/guild/mail state is rejected rather than silently reset.
"""
import hashlib
import json
import secrets
import sqlite3
import sys
import time
from contextlib import contextmanager
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'contrib/headless-client'))
sys.path.insert(0,str(ROOT/'contrib/qa-automation'))
from starter_campaign import Database
from evidence import write_json
from run_lock import RunLock
import headless


@contextmanager
def metadata_lock(path):
    deadline=time.monotonic()+10
    while True:
        lock=RunLock(path)
        try:lock.__enter__();break
        except RuntimeError:
            if time.monotonic()>=deadline:raise
            time.sleep(.05)
    try:yield
    finally:lock.__exit__(None,None,None)


class Pool:
    def __init__(self,runtime=None):
        self.runtime=Path(runtime or ROOT/'out/quest-qa-runtime').resolve()
        if not self.runtime.is_relative_to(ROOT/'out'):raise ValueError('QA runtime must stay under out/')
        self.environment=json.loads((self.runtime/'environment.json').read_text())
        if self.environment.get('state')!='ready' or self.environment.get('prefix')!='mop_loa_qa_':raise ValueError('Owned isolated QA environment required')
        self.db=Database(self.runtime/'worldserver.conf',ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
        if any(not v[4].startswith('mop_loa_qa_') for v in self.db.connections.values()):raise ValueError('Refusing non-QA database connection')
        self.path=self.runtime/'pool.json'
        self.state=json.loads(self.path.read_text()) if self.path.exists() else dict(owner=self.environment['owner'],accounts=[],characters=[],token=''.join(secrets.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(4)))
        if self.state['owner']!=self.environment['owner']:raise ValueError('Pool ownership mismatch')
        self.registry=sqlite3.connect(self.runtime/'execution.sqlite',timeout=30)
        self.registry.execute('PRAGMA journal_mode=WAL')
        self.registry.execute('CREATE TABLE IF NOT EXISTS leases(account_id INTEGER PRIMARY KEY,owner TEXT NOT NULL,expires REAL NOT NULL)')
        self.registry.execute('CREATE TABLE IF NOT EXISTS events(run TEXT,scenario TEXT,step TEXT,at REAL,status TEXT,expected TEXT,observed TEXT)')
        self.registry.commit()
        if not self.path.exists():self.save()
        self.owned_leases={}
        headless.log=lambda *args,**kwargs:None

    def save(self,changed_char=None):
        if changed_char is not None:
            self.state['characters']=[changed_char if c['guid']==changed_char['guid'] else c for c in self.state['characters']]
        with metadata_lock(self.runtime/'pool-metadata.lock'):
            if self.path.exists():
                old=json.loads(self.path.read_text())
                for key,identity in [('accounts','id'),('characters','guid')]:
                    merged={r[identity]:r for r in old[key]}
                    for row in self.state[key]:
                        previous=merged.get(row[identity],{})
                        merged[row[identity]]={**previous,**row}
                        if key=='characters':merged[row[identity]]['reset_count']=max(previous.get('reset_count',0),row.get('reset_count',0))
                    self.state[key]=list(merged.values())
            write_json(self.path,self.state)

    def account(self,index):
        while len(self.state['accounts'])<=index:
            i=len(self.state['accounts']);name='MOPQA'+self.state['token'].upper()+str(i);password=secrets.token_hex(8).upper()
            if self.db.q('Login',f"SELECT id FROM account WHERE username='{name}'"):raise RuntimeError('Unowned pool account already exists')
            digest=hashlib.sha1((name+':'+password).encode()).hexdigest()
            self.db.q('Login',f"INSERT INTO account(username,sha_pass_hash,expansion) VALUES('{name}','{digest}',4)")
            ident=int(self.db.q('Login',f"SELECT id FROM account WHERE username='{name}'")[0][0])
            self.state['accounts'].append(dict(id=ident,name=name,password=password));self.save()
        return self.state['accounts'][index]

    def connect(self,account):
        actual=self.db.q('Login',f"SELECT username FROM account WHERE id={account['id']}")
        if not actual or actual[0][0]!=account['name']:raise RuntimeError('Account ownership changed')
        for _ in range(60):
            if self.db.q('Login',f"SELECT online FROM account WHERE id={account['id']}")[0][0]=='0':break
            time.sleep(.5)
        key=headless.auth_login('127.0.0.1',3725,account['name'],account['password'])
        w=headless.WorldClient('127.0.0.1',8086,key,account['name']);w.emit=lambda *args,**kwargs:None
        w.handshake(key,realm_id=1);return w

    def lease(self,account,seconds=900):
        for _ in range(60):
            if not self.db.q('Character',f"SELECT guid FROM characters WHERE account={account['id']} AND online<>0"):break
            time.sleep(.5)
        owner=secrets.token_hex(16);now=time.time()
        self.registry.execute('BEGIN IMMEDIATE')
        old=self.registry.execute('SELECT owner,expires FROM leases WHERE account_id=?',(account['id'],)).fetchone()
        if old and old[1]>now:self.registry.rollback();raise RuntimeError('QA account already leased')
        online=self.db.q('Character',f"SELECT guid FROM characters WHERE account={account['id']} AND online<>0")
        if online:self.registry.rollback();raise RuntimeError('QA account still has an online character')
        self.registry.execute('INSERT OR REPLACE INTO leases VALUES(?,?,?)',(account['id'],owner,now+seconds));self.registry.commit();self.owned_leases[account['id']]=owner;return owner

    def release(self,account,owner):
        self.registry.execute('DELETE FROM leases WHERE account_id=? AND owner=?',(account['id'],owner));self.registry.commit()
        if self.owned_leases.get(account['id'])==owner:self.owned_leases.pop(account['id'])

    def require_lease(self,account_id):
        row=self.registry.execute('SELECT owner,expires FROM leases WHERE account_id=?',(account_id,)).fetchone()
        if not row or row[0]!=self.owned_leases.get(account_id) or row[1]<=time.time():raise RuntimeError('Current worker does not own an active account lease')

    def heartbeat(self,account_id,seconds=900):
        self.require_lease(account_id)
        self.registry.execute('UPDATE leases SET expires=? WHERE account_id=? AND owner=?',(time.time()+seconds,account_id,self.owned_leases[account_id]));self.registry.commit()

    def ensure(self,race,klass):
        self.state=json.loads(self.path.read_text())
        existing=next((c for c in self.state['characters'] if c['race']==race and c['class']==klass and c.get('purpose')!='persistent-adventure'),None)
        if existing:
            self.validate(existing)
            if not existing.get('snapshot_sha256'):raise RuntimeError('Interrupted baseline snapshot requires explicit recovery')
            return existing
        with RunLock(self.runtime/'pool-creation.lock'):
            self.state=json.loads(self.path.read_text())
            return self._ensure(race,klass)

    def _ensure(self,race,klass):
        existing=next((c for c in self.state['characters'] if c['race']==race and c['class']==klass and c.get('purpose')!='persistent-adventure'),None)
        if existing:
            self.validate(existing);return existing
        if not self.state['accounts']:self.account(0)
        counts=[int(self.db.q('Character',f"SELECT COUNT(*) FROM characters WHERE account={a['id']}")[0][0]) for a in self.state['accounts']]
        busy={r[0] for r in self.registry.execute('SELECT account_id FROM leases WHERE expires>?',(time.time(),))}
        available=[i for i,n in enumerate(counts) if n<11 and self.state['accounts'][i]['id'] not in busy]
        if klass==6:
            available=[i for i in available if not self.db.q('Character',f"SELECT guid FROM characters WHERE account={self.state['accounts'][i]['id']} AND class=6")]
        if not available:
            i=len(counts);self.account(i);counts.append(0);available=[i]
        account=self.account(min(available,key=lambda i:counts[i]));owner=self.lease(account)
        name=('Qe'+self.state['token']+chr(97+race//26)+chr(97+race%26)+chr(97+klass//26)+chr(97+klass%26)).capitalize()
        w=None;anchor=None;oldlevel=None
        try:
            # Pandaren faction-selected states are prepared from a neutral body;
            # they are NOT a verified faction-selection quest test.
            create_race=24 if race in (25,26) else race
            if klass==6:
                anchors=self.db.q('Character',f"SELECT guid,level FROM characters WHERE account={account['id']} ORDER BY guid LIMIT 1")
                if not anchors:
                    anchor_name=('Qa'+self.state['token']+chr(97+account['id']//26)+chr(97+account['id']%26)).capitalize()
                    initial=self.connect(account)
                    try:
                        initial.char_enum();initial.char_create(anchor_name,2,1)
                        for _ in range(100):
                            op,body=initial.read_packet()
                            if op==headless.SMSG_CHAR_CREATE:
                                if body!=bytes([47]):raise RuntimeError('Anchor creation response '+body.hex())
                                break
                        else:raise RuntimeError('Anchor creation did not respond')
                    finally:initial.s.close()
                    anchor_id=int(self.db.q('Character',f"SELECT guid FROM characters WHERE name='{anchor_name}' AND account={account['id']}")[0][0])
                    extra=dict(race=2,**{'class':1},name=anchor_name,guid=anchor_id,account_id=account['id'],prepared_faction_state=False,reset_count=0,auxiliary_anchor=True)
                    self.state['characters'].append(extra);self.save();self.snapshot(extra)
                    anchors=[[str(anchor_id),'1']]
                anchor,oldlevel=map(int,anchors[0]);self.db.q('Character',f'UPDATE characters SET level=55 WHERE guid={anchor}')
            w=self.connect(account);w.char_enum();w.char_create(name,create_race,klass)
            for _ in range(100):
                op,body=w.read_packet()
                if op==headless.SMSG_CHAR_CREATE:
                    if body!=bytes([47]):raise RuntimeError('Character creation response '+body.hex())
                    break
            else:raise RuntimeError('Character creation did not respond')
            w.char_enum();guid=int(self.db.q('Character',f"SELECT guid FROM characters WHERE name='{name}' AND account={account['id']}")[0][0])
            char={'race':race,'class':klass,'name':name,'guid':guid,'account_id':account['id'],'prepared_faction_state':race in (25,26),'reset_count':0}
            self.state['characters'].append(char);self.save()
            if klass==6:self.db.q('Character',f'UPDATE characters SET level={oldlevel} WHERE guid={anchor}')
            if race in (25,26):self.db.q('Character',f'UPDATE characters SET race={race} WHERE guid={guid}')
            self.snapshot(char);return char
        finally:
            if w:w.s.close()
            if anchor is not None and oldlevel is not None:self.db.q('Character',f'UPDATE characters SET level={oldlevel} WHERE guid={anchor}')
            self.release(account,owner)

    def validate(self,char):
        rows=self.db.q('Character',f"SELECT name,account FROM characters WHERE guid={char['guid']}")
        if rows!=[[char['name'],str(char['account_id'])]]:raise RuntimeError('Character identity differs from managed registry')

    def tables(self):
        rows=self.db.q('Character',"SELECT TABLE_NAME FROM information_schema.COLUMNS WHERE TABLE_SCHEMA=DATABASE() AND COLUMN_NAME='guid' AND TABLE_NAME LIKE 'character\\_%' ORDER BY TABLE_NAME")
        return ['characters']+[r[0] for r in rows]

    def capture_rows(self,table,where):
        info=self.db.q('Character',f'SHOW COLUMNS FROM `{table}`');columns=[r[0] for r in info]
        expr=','.join("IF(`"+c+"` IS NULL,'N',CONCAT('H',HEX(CAST(`"+c+"` AS BINARY))))" for c in columns)
        return dict(columns=columns,types={r[0]:r[1] for r in info},rows=self.db.q('Character',f'SELECT {expr} FROM `{table}` WHERE {where}'))

    def snapshot(self,char):
        self.validate(char);guid=char['guid'];snapshot=dict(guid=guid,account_id=char['account_id'],scope='character GUID tables and owned starting items; shared guild/group/mail state rejected',tables={})
        # One schema read and one batched data read replace hundreds of client
        # launches per body. Values remain typed hex snapshots, not SQL text.
        if not hasattr(self,'snapshot_schema'):
            names=self.tables()+['item_instance'];quoted=','.join("'"+n+"'" for n in names)
            schema={n:dict(columns=[],types={},rows=[]) for n in names}
            for table,column,kind in self.db.q('Character',f'SELECT TABLE_NAME,COLUMN_NAME,COLUMN_TYPE FROM information_schema.COLUMNS WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME IN ({quoted}) ORDER BY TABLE_NAME,ORDINAL_POSITION'):
                schema[table]['columns'].append(column);schema[table]['types'][column]=kind
            self.snapshot_schema=schema
        snapshot['tables']=json.loads(json.dumps(self.snapshot_schema));queries=[]
        for table,data in snapshot['tables'].items():
            expressions=','.join("IF(`"+column+"` IS NULL,'N',CONCAT('H',HEX(CAST(`"+column+"` AS BINARY))))" for column in data['columns'])
            where=('owner_guid' if table=='item_instance' else 'guid')+f'={guid}'
            queries.append(f"SELECT '{table}',JSON_ARRAY({expressions}) FROM `{table}` WHERE {where}")
        for table,row in self.db.q('Character',' UNION ALL '.join(queries)):
            snapshot['tables'][table]['rows'].append(json.loads(row))
        target=self.runtime/f"snapshot-{guid}.json";write_json(target,snapshot);char['snapshot_sha256']=hashlib.sha256(target.read_bytes()).hexdigest();self.save(char)

    def reset(self,char):
        self.require_lease(char['account_id'])
        self.validate(char);guid=char['guid']
        fresh=json.loads(self.path.read_text());char.update(next(c for c in fresh['characters'] if c['guid']==guid))
        if char.get('purpose') == 'persistent-adventure':
            raise RuntimeError('Persistent adventure character cannot be reset by deterministic QA')
        for _ in range(60):
            if not self.db.q('Character',f'SELECT guid FROM characters WHERE guid={guid} AND online<>0'):break
            time.sleep(.5)
        else:raise RuntimeError('Cannot reset an online character')
        for table,column in [('guild_member','guid'),('group_member','memberGuid'),('mail','receiver')]:
            if self.db.q('Character',f'SELECT 1 FROM `{table}` WHERE `{column}`={guid} LIMIT 1'):
                raise RuntimeError('Shared-state reset driver required: '+table)
        path=self.runtime/f'snapshot-{guid}.json'
        if hashlib.sha256(path.read_bytes()).hexdigest()!=char['snapshot_sha256']:raise RuntimeError('Snapshot changed')
        snapshot=json.loads(path.read_text());sql=['START TRANSACTION']
        # Item-instance deletion precedes inventory restoration; no personal schema
        # is addressable through this pool. More complex mail/group tests are blocked.
        sql.append(f'DELETE FROM item_instance WHERE owner_guid={guid}')
        for table,data in snapshot['tables'].items():
            if table=='item_instance':continue
            if table=='characters':
                row=data['rows'][0]
                sql.append('UPDATE characters SET '+','.join('`'+c+'`='+self.literal(v,data['types'][c]) for c,v in zip(data['columns'],row) if c not in ('guid','account','name'))+f' WHERE guid={guid}')
            else:sql.append(f'DELETE FROM `{table}` WHERE guid={guid}')
        for table,data in snapshot['tables'].items():
            if table=='characters':continue
            if data['rows']:
                sql.append('INSERT INTO `'+table+'` ('+','.join('`'+c+'`' for c in data['columns'])+') VALUES '+','.join('('+','.join(self.literal(v,data['types'][c]) for c,v in zip(data['columns'],row))+')' for row in data['rows']))
        sql.append('COMMIT');self.db.q('Character',';'.join(sql))
        char['reset_count']+=1;self.save(char)

    @staticmethod
    def literal(value,kind='text'):
        if value=='N':return 'NULL'
        raw="X'"+value[1:]+"'"
        return raw if any(t in kind.lower() for t in ('blob','binary','bit')) else 'CONVERT('+raw+' USING utf8mb4)'

    def event(self,run,scenario,step,status,expected,observed):
        self.registry.execute('INSERT INTO events VALUES(?,?,?,?,?,?,?)',(run,scenario,step,time.time(),status,json.dumps(expected),json.dumps(observed)));self.registry.commit()
