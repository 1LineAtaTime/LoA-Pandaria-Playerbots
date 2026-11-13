"""Real 18414 packet regression for the published Gilneas prison wave encounter.

Creates one owned disposable account/character, then removes both using native
console commands. Does not touch personal characters or require their passwords.
"""
import argparse
import hashlib
import json
import secrets
import struct
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'contrib/headless-client'))
import headless
from qa_actions import ObservedWorld
from starter_campaign import Database
from qa_cleanup import cleanup_owned


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--world-pid',type=int,required=True)
    p.add_argument('--report',type=Path,required=True)
    a=p.parse_args()
    db=Database(ROOT/'exports/worldserver.conf',ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
    name=('Qz'+''.join(secrets.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(8))).capitalize()
    account=name.upper();password=secrets.token_hex(8).upper();w=None
    report=dict(account=account,name=name,created=False,checks=[],passed=False)
    a.report.parent.mkdir(parents=True,exist_ok=True)
    def save():a.report.write_text(json.dumps(report,indent=2)+'\n')
    def console(command):subprocess.run([sys.executable,str(ROOT/'contrib/windows/Send-ServerConsoleCommand.py'),str(a.world_pid),command],check=True)
    headless.log=lambda *args,**kwargs:None
    try:
        assert not db.q('Login',f"SELECT id FROM account WHERE username='{account}'")
        digest=hashlib.sha1((account+':'+password).encode()).hexdigest()
        db.q('Login',f"INSERT INTO account(username,sha_pass_hash,expansion) VALUES('{account}','{digest}',4)")
        report['created']=True
        report['account_id']=int(db.q('Login',f"SELECT id FROM account WHERE username='{account}'")[0][0]);save()
        db.q('Login',f"INSERT INTO account_access(id,gmlevel,RealmID) VALUES({report['account_id']},3,-1)")
        db.q('Login',f"INSERT INTO rbac_account_permissions(accountId,permissionId,granted,realmId) VALUES({report['account_id']},192,1,-1)")
        key=headless.auth_login('127.0.0.1',3724,account,password)
        w=headless.WorldClient('127.0.0.1',8085,key,account);w.emit=lambda *args,**kwargs:None
        w.handshake(key,realm_id=1);assert not w.char_enum();w.char_create(name,22,1)
        for _ in range(100):
            op,body=w.read_packet()
            if op==headless.SMSG_CHAR_CREATE:
                assert body==bytes([47]);break
        else:raise AssertionError('No character creation response')
        assert any(c['name']==name for c in w.char_enum()),'New character missing from enumeration'
        report['guid']=int(db.q('Character',f"SELECT guid FROM characters WHERE name='{name}'")[0][0]);save()
        db.q('Character',f"UPDATE characters SET level=10,map=654,zone=4755,position_x=-1679.73,position_y=1442.12,position_z=52.3705 WHERE guid={report['guid']}")
        db.q('Character',f"INSERT INTO character_queststatus_rewarded(guid,quest,active) VALUES({report['guid']},14078,1),({report['guid']},28850,1)")
        report['fixture']=dict(disposable=True,gm_observer=True,seeded_predecessors=[14078,28850],quest_not_precompleted=14154)
        tracker=ObservedWorld(w);previous=w.auto
        def auto(op,body):
            previous(op,body)
            try:tracker.handle(op,body,time.time())
            except (ValueError,KeyError,IndexError,struct.error,EOFError):tracker.decode_errors+=1
        w.auto=auto
        assert w.player_login(report['guid'])
        w.send_chat('say','.gm on');w.pump(3)
        crowley=(0xF13<<52)|(35077<<32)|int(db.q('World','SELECT guid FROM creature WHERE id=35077 AND map=654 LIMIT 1')[0][0])
        w.quest_hello(crowley);w.pump(1);w.quest_accept(14154,crowley)
        started=time.monotonic();w.pump(18)
        entries=sorted({(g>>32)&0xfffff for g in tracker.ever_seen if g>>52==0xF13})
        assert 35167 in entries and 35170 in entries,'No first rooftop alpha wave observed'
        report['checks'].append(dict(first_wave_entries=[35167,35170],observed_rooftop_movement=any((g>>32)&0xfffff in (35167,35170) for g in tracker.spl)))
        save()
        while time.monotonic()-started<125:w.pump(min(2,125-(time.monotonic()-started)))
        report['checks'].append(dict(observed_entries=[e for e in entries if e in (35167,35170,35188,35456)],elapsed_seconds=round(time.monotonic()-started)))
        assert w.logout();w.s.close();w=None
        for _ in range(20):
            state=db.q('Character',f"SELECT status FROM character_queststatus WHERE guid={report['guid']} AND quest=14154")
            if state and state[0][0]=='1':break
            time.sleep(.5)
        assert state and state[0][0]=='1','Timed survival completion did not persist'
        report['checks'].append(dict(quest=14154,persisted_complete=True))
        report['passed']=True
    except Exception as e:
        report['error']=str(e).replace(password,'[redacted]')
        report['traceback']=__import__('traceback').format_exc().replace(password,'[redacted]')
    finally:
        if w:w.s.close()
        if report['created']:
            clean=cleanup_owned(db,a.world_pid,account,name,report['account_id'],report.get('guid'))
            report['cleanup_account_absent']=clean['account_absent']
            report['cleanup_character_absent']=clean['character_absent']
            report['cleanup_order']=clean['cleanup_order']
        save()
    print(json.dumps(report))
    return int(not report['passed'] or not report.get('cleanup_account_absent') or not report.get('cleanup_character_absent'))


if __name__=='__main__':sys.exit(main())
