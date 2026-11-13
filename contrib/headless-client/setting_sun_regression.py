"""One disposable fixture probes restored Gate of the Setting Sun objects and AI.
Not a rendered bridge acceptance test or complete dungeon encounter playthrough.
"""
import hashlib,json,secrets,struct,subprocess,sys,time
from pathlib import Path
R=Path(__file__).resolve().parents[2];sys.path.insert(0,str(R/'contrib/headless-client'))
import headless
from starter_campaign import Database
from qa_actions import Workflow
from qa_cleanup import cleanup_owned
headless.log=lambda *a,**k:None
d=Database(R/'exports/worldserver.conf',R/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
pid=int(subprocess.check_output(['powershell','-NoProfile','-Command','Get-Process worldserver | Select-Object -ExpandProperty Id'],text=True).strip())
name='Qgate'+''.join(secrets.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(5));account=name.upper();password=secrets.token_hex(10).upper();aid=guid=None;w=None
report=dict(fixture_only=True,passed=False,samples=[],scope='Native controller, object delivery and bombardier update survival; no render/encounter completion claim')
O=R/'out/backups/gate-setting-sun-crash-20261112';O.mkdir(parents=True,exist_ok=True)
try:
 sha=hashlib.sha1((account+':'+password).encode()).hexdigest();d.q('Login',"INSERT INTO account(username,sha_pass_hash,expansion) VALUES('"+account+"','"+sha+"',4)");aid=int(d.q('Login',"SELECT id FROM account WHERE username='"+account+"'")[0][0]);d.q('Login',f'INSERT INTO account_access(id,gmlevel,RealmID) VALUES({aid},3,-1)')
 (O/'active-fixture.json').write_text(json.dumps(dict(account=account,name=name,account_id=aid)))
 key=headless.auth_login('127.0.0.1',3724,account,password);w=headless.WorldClient('127.0.0.1',8085,key,account);w.handshake(key,realm_id=1);w.emit=lambda *a,**k:None
 w.char_create(name,1,1)
 end=time.monotonic()+30
 while time.monotonic()<end:
  op,b=w.read_packet()
  if op==headless.SMSG_CHAR_CREATE:assert b==bytes([47]);break
 w.char_enum();guid=int(d.q('Character',"SELECT guid FROM characters WHERE name='"+name+"'")[0][0]);d.q('Character',f'UPDATE characters SET level=90,health=100000 WHERE guid={guid}')
 report.update(account_id=aid,guid=guid,name=name,account=account);(O/'active-fixture.json').write_text(json.dumps(report))
 wf=Workflow(w,d,guid,dict(quest=0),report,lambda x:None);messages=[]
 def emit(event,**data):
  if event=='chat':messages.append(data)
 w.emit=emit
 assert w.player_login(guid);w.pump(2);w.send_chat('say','.gm on');w.pump(.5);w.send_chat('say','.gm fly on');w.pump(.5);w.send_packet(headless.CMSG_SET_DUNGEON_DIFFICULTY,struct.pack('<I',2));w.pump(.5)
 for label,pos in [('entrance',(722.4323,2183.4097,389.4479)),('first_courtyard',(726,2304,390)),('wall_route',(830.512,2304,390)),('saved_fall_position',(830.512,2367.91,390)),('lift',(1195.21,2304.52,432))]:
  w.send_chat('say','.go xyz '+' '.join(str(v) for v in pos)+' 962');w.pump(5);start=len(messages);w.send_chat('say','.gps');w.pump(1)
  if label=='entrance':
   w.areatrigger(8359);w.pump(2)
   doors=wf.tracker.actors(212982,'gameobject');report['entrance_door_states']=[wf.tracker.object_values.get(g,{}).get(19,0)&255 for g,_ in doors]
  sample=dict(label=label,position=list(w.pos),gps=messages[start:],objects={str(e):len(wf.tracker.actors(e,'gameobject')) for e in (212982,212983,214629,215757,211013)},bombardiers=len(wf.tracker.actors(56706)))
  sample['lift_transports']=sum(1 for g,v in wf.tracker.object_values.items() if v.get(5)==211013 and g in wf.tracker.visible)
  report['samples'].append(sample)
  if label=='lift':
   start=len(messages);w.send_chat('say','.botscenario '+name+' transportinfo 211013');w.pump(1);report['native_lift_query']=messages[start:]
  if sample['bombardiers']:w.pump(16)
 w.send_chat('say','.save');w.pump(.5);report['saved_map']=int(d.q('Character',f'SELECT map FROM characters WHERE guid={guid}')[0][0]);report['controller_binding']=d.q('World','SELECT script FROM instance_template WHERE map=962')[0][0]
 report['passed']=report['saved_map']==962 and report['controller_binding']=='instance_gate_of_the_setting_sun' and any(s['bombardiers'] for s in report['samples']) and any(s['objects']['215757'] for s in report['samples']) and report.get('entrance_door_states')==[0]
 report['lift_packet_observed']=any(s['lift_transports'] for s in report['samples']);report['logout_acknowledged']=w.logout();assert report['passed'],report['samples'];print('Dungeon controller/object/update regression passed',flush=True)
finally:
 if w:
  try:
   if not report.get('logout_acknowledged'):w.logout()
  except Exception:pass
  w.s.close()
 if aid:report['cleanup']=cleanup_owned(d,pid,account,name,aid,guid)
 (O/'native-regression.json').write_text(json.dumps(report,indent=2,default=str))
