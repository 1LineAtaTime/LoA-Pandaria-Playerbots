"""Real 18414 vehicle/camera teardown and character re-entry regression.

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
        actor=db.q('World','SELECT c.guid,c.map,c.position_x,c.position_y,c.position_z FROM creature c JOIN creature_template t ON t.entry=c.id WHERE c.id=35552 AND t.ScriptName=\'npc_lord_darius_crowley_c2\' LIMIT 1')[0]
        spawn,mapid,x,y,z=actor
        db.q('Character',f"UPDATE characters SET level=10,map={mapid},zone=4755,position_x={x},position_y={y},position_z={z} WHERE guid={report['guid']}")
        db.q('Character',f"INSERT INTO character_queststatus(guid,quest,status) VALUES({report['guid']},14212,3)")
        report['fixture']=dict(disposable=True,gm_observer=True,seeded_incomplete_quest=14212,quest_not_rewarded=True)
        log=ROOT/'exports/logs/Server.log';offset=log.stat().st_size
        tracker=ObservedWorld(w);previous=w.auto
        def auto(op,body):
            previous(op,body)
            try:tracker.handle(op,body,time.time())
            except (ValueError,KeyError,IndexError,struct.error,EOFError):tracker.decode_errors+=1
        w.auto=auto
        assert w.player_login(report['guid'])
        w.send_chat('say','.gm on');w.pump(3)
        crowley=(0xF13<<52)|(35552<<32)|int(spawn)
        w.gossip_hello(crowley);w.pump(2)
        options=[o for o in (w.gossip or {}).get('options',[]) if "let's ride" in o['text'].lower()]
        assert len(options)==1,'Ride retry gossip option missing'
        w.gossip_select(options[0]['id']);w.pump(10)
        entries={(g>>32)&0xfffff for g in tracker.ever_seen if g>>52==0xF13}
        with log.open('rb') as f:f.seek(offset);ride_log=f.read().decode('utf-8',errors='replace')
        assert f"rider={report['guid']} started path35231" in ride_log,'Owned ride did not start after seating Crowley'
        report['checks'].append(dict(horse=35231,mounted_actor=35230,ride_start_oracle='server trace after both seats validated',ride_started=True));save()
        assert w.logout(),'Mid-ride logout failed';w.pump(3)
        assert w.player_login(report['guid']),'Character could not re-enter after mid-ride logout'
        w.pump(3);assert w.logout();w.s.close();w=None
        with log.open('rb') as f:f.seek(offset);text=f.read().decode('utf-8',errors='replace')
        failures=[line for line in text.splitlines() if name in line and ('FATAL' in line or 'cannot remove current viewpoint' in line)]
        assert not failures,'Viewpoint teardown diagnostic recurred'
        report['checks'].append(dict(mid_ride_logout_reentry=True,viewpoint_failures=0,quest_not_completed_by_fixture=True))
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
