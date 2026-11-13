"""Disposable static-gunship packet/rope check; real-client rendering is separate.
Creates an owned seeded fixture and removes it through native console cleanup.
"""
import sys,time,secrets,hashlib,json,struct,subprocess
from pathlib import Path
R=Path(__file__).resolve().parents[2];sys.path.insert(0,str(R/'contrib/headless-client'))
import headless
from starter_campaign import Database
from qa_actions import Workflow
from qa_cleanup import cleanup_owned
headless.log=lambda *a,**k:None
d=Database(R/'exports/worldserver.conf',R/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
pid=int(subprocess.check_output(['powershell','-NoProfile','-Command','Get-Process worldserver | Select-Object -ExpandProperty Id'],text=True).strip())
name='Qfinal'+''.join(secrets.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(4));account=name.upper();password=secrets.token_hex(10).upper();aid=guid=None;w=None
report=dict(fixture_only=True,passed=False);(R/'out/finale-active-fixture.json').write_text(json.dumps(dict(account=account,name=name)))
def connect():
 key=headless.auth_login('127.0.0.1',3724,account,password);client=headless.WorldClient('127.0.0.1',8085,key,account);client.emit=lambda *a,**k:None;client.handshake(key,realm_id=1);return client
def owner(tracker,g):
 v=tracker.object_values.get(g,{});return v.get(0x0e,0)+(v.get(0x0f,0)<<32)
try:
 sha=hashlib.sha1((account+':'+password).encode()).hexdigest();d.q('Login',"INSERT INTO account(username,sha_pass_hash,expansion) VALUES('"+account+"','"+sha+"',4)");aid=int(d.q('Login',"SELECT id FROM account WHERE username='"+account+"'")[0][0]);d.q('Login',f'INSERT INTO account_access(id,gmlevel,RealmID) VALUES({aid},3,-1)')
 w=connect();packets=[];orig=w.read_packet
 def capture():
  op,b=orig()
  if op in (0x9d8,0x143a,0x095a,0x4af,0x40b,0x139,0x156):packets.append([hex(op),b.hex()])
  return op,b
 w.read_packet=capture;w.char_create(name,22,4)
 end=time.monotonic()+30
 while time.monotonic()<end:
  op,b=w.read_packet()
  if op==headless.SMSG_CHAR_CREATE:assert b==bytes([47]);break
 w.char_enum();guid=int(d.q('Character',"SELECT guid FROM characters WHERE name='"+name+"'")[0][0]);report.update(account_id=aid,guid=guid,name=name,account=account);(R/'out/finale-active-fixture.json').write_text(json.dumps(report))
 d.q('Character',f'UPDATE characters SET level=12,health=1000,map=654,zone=4714,position_x=-1465.15,position_y=3256.89,position_z=191.68 WHERE guid={guid}')
 d.q('Character',f'INSERT INTO character_queststatus_rewarded(guid,quest,active) VALUES '+','.join(f'({guid},{q},1)' for q in (14386,14466,24676,24904,24902,24903,24920,24678,24679,24680)))
 d.q('Character',f'INSERT INTO character_queststatus(guid,quest,status) VALUES({guid},24681,3)')
 wf=Workflow(w,d,guid,dict(quest=24681),report,lambda x:None);assert w.player_login(guid);w.pump(3)
 w.send_chat('say','.gm off');w.pump(.5)
 w.send_chat('say','.gm on');w.pump(2)
 ships=wf.tracker.actors(800100,'gameobject');assert len(ships)==1,'Expected restored static ship'
 values=wf.tracker.object_values[ships[0][0]];report['wire_ship_type']=(values.get(0x12,0)>>8)&255;report['wire_display']=values.get(0xa,0)
 report['passed']=report['wire_ship_type']==14 and report['wire_display']==8253
 print('Ship wire compatibility',report['wire_ship_type'],report['wire_display'],report['passed'],flush=True)
 report['logout_acknowledged']=w.logout()

finally:
 if w:
  try:w.logout()
  except Exception:pass
  w.s.close()
 if aid:report['cleanup']=cleanup_owned(d,pid,account,name,aid,guid)
 (R/'out/gunship-wire-regression.json').write_text(json.dumps(report,indent=2,default=str))
 print('Report saved',flush=True)
