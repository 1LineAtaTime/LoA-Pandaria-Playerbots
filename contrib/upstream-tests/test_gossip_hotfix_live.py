"""Real 18414 packet regression: legacy NPC text -> WNPC -> BroadcastText hotfix.

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
        assert w.player_login(report['guid'])
        packets={};previous=w.auto
        def auto(op,body):
            previous(op,body)
            if op in (0x140A,0x103B):packets.setdefault(op,[]).append(body)
        w.auto=auto
        # Three different authored greetings, including the menu-less NPC default.
        for text in (3583,4938,16777215):
            expected=db.q('World',f'SELECT HEX(text0_0) FROM npc_text WHERE ID={text}')[0][0]
            w.send_packet(0x0287,struct.pack('<I',text)+b'\0')
            w.pump(5,stop_on=(0x140A,));body=packets[0x140A][-1]
            ident,size=struct.unpack_from('<II',body);assert ident==text and size==64
            weights=struct.unpack_from('<8f',body,8);ids=struct.unpack_from('<8I',body,40)
            assert weights[0]>0 and ids[0]!=1,'Repressed fallback or missing weight'
            bw=headless.BitWriter();bw.bits(1,21);bw.bits(0,8)
            w.send_packet(0x158D,struct.pack('<I',35137211)+bw.flush()+struct.pack('<I',ids[0]))
            w.pump(5,stop_on=(0x103B,));reply=packets[0x103B][-1]
            record,stamp,kind,size=struct.unpack_from('<4I',reply)
            assert record==ids[0] and kind==35137211 and len(reply)==16+size
            hotfix=reply[16:];assert struct.unpack_from('<I',hotfix)[0]==ids[0]
            length=struct.unpack_from('<H',hotfix,8)[0];actual=hotfix[10:10+length]
            assert actual==bytes.fromhex(expected),'Authored NPC dialogue changed'
            assert b'being repressed' not in actual
            report['checks'].append(dict(text=text,broadcast=ids[0],authored_text_received=True))
            save()
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
