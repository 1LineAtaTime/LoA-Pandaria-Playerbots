"""Real 18414 Shattrath flask-vendor reputation and inventory regression.

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
        key=headless.auth_login('127.0.0.1',3724,account,password)
        w=headless.WorldClient('127.0.0.1',8085,key,account);w.emit=lambda *args,**kwargs:None
        w.handshake(key,realm_id=1);assert not w.char_enum();w.char_create(name,1,1)
        for _ in range(100):
            op,body=w.read_packet()
            if op==headless.SMSG_CHAR_CREATE:
                assert body==bytes([47]);break
        else:raise AssertionError('No character creation response')
        assert any(c['name']==name for c in w.char_enum()),'New character missing from enumeration'
        report['guid']=int(db.q('Character',f"SELECT guid FROM characters WHERE name='{name}'")[0][0]);save()
        actor=db.q('World','SELECT guid,map,position_x,position_y,position_z FROM creature WHERE id=23484 AND phaseMask=1 AND phaseId=0 LIMIT 1')[0]
        spawn,mapid,x,y,z=actor
        db.q('Character',f"UPDATE characters SET level=85,map={mapid},position_x={x},position_y={y},position_z={z} WHERE guid={report['guid']}")
        report['fixture']=dict(disposable=True,level=85,seeded_reputation_only=True)
        assert w.player_login(report['guid'])
        actor_guid=(0xF13<<52)|(23484<<32)|int(spawn)
        w.gossip_hello(actor_guid);w.pump(2)
        assert w.gossip is not None,'No vendor denial gossip'
        assert not w.gossip.get('options'),'Unqualified character received a trade option'
        report['checks'].append(dict(no_reputation_trade_denied=True));save()
        assert w.logout()
        db.q('Character',f"INSERT INTO character_reputation(guid,faction,standing,flags) VALUES({report['guid']},932,42000,1),({report['guid']},935,42000,1),({report['guid']},942,42000,1) ON DUPLICATE KEY UPDATE standing=VALUES(standing),flags=VALUES(flags)")
        assert w.player_login(report['guid'])
        w.gossip_hello(actor_guid);w.pump(2)
        options=(w.gossip or {}).get('options',[])
        assert len(options)==1,'Exalted character did not receive trade option'
        packets=[];previous=w.auto
        def auto(op,body):
            previous(op,body)
            if op==0x1AAE:packets.append(body)
        w.auto=auto;w.gossip_select(options[0]['id']);w.pump(5,stop_on=(0x1AAE,))
        assert packets,'No inventory response from enabled vendor'
        report['checks'].append(dict(three_required_reputations_exalted=True,vendor_inventory_received=True))
        assert w.logout();w.s.close();w=None;report['passed']=True
    except Exception as e:
        report['error']=str(e).replace(password,'[redacted]')
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
