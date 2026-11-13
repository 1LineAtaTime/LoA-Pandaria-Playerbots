"""Disposable Endgame return camera/movement and actual quest-turn-in regression.
Seeds only owned fixture prerequisites; boards the native wyvern with a test ride spell.
Camera fields are protocol evidence, not a pixel-framing certification.
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
 w=connect();w.char_create(name,22,4)
 end=time.monotonic()+30
 while time.monotonic()<end:
  op,b=w.read_packet()
  if op==headless.SMSG_CHAR_CREATE:assert b==bytes([47]);break
 w.char_enum();guid=int(d.q('Character',"SELECT guid FROM characters WHERE name='"+name+"'")[0][0]);report.update(account_id=aid,guid=guid,name=name,account=account);(R/'out/finale-active-fixture.json').write_text(json.dumps(report))
 d.q('Character',f'UPDATE characters SET level=12,health=1000,map=654,zone=4714,position_x=-1436.32,position_y=3229.69,position_z=105.43,extra_flags=1 WHERE guid={guid}')
 d.q('Character',f'INSERT INTO character_queststatus_rewarded(guid,quest,active) VALUES '+','.join(f'({guid},{q},1)' for q in (14386,14466,24676,24904,24902,24903,24920,24678,24679,24680,24681)))
 d.q('Character',f'INSERT INTO character_queststatus(guid,quest,status) VALUES({guid},26706,3)')
 wf=Workflow(w,d,guid,dict(quest=26706),report,lambda x:None);assert w.player_login(guid);w.pump(3)
 w.send_chat('say','.gm on');w.pump(.3)
 w.send_chat('say','.npc add temp 43713');w.pump(.5)
 flights=wf.tracker.actors(43713);assert len(flights)==1,'Escape vehicle missing';flight=flights[0][0]
 w.set_selection(flight);w.send_chat('say','.cast 46598');w.pump(2)
 own=wf.tracker.object_values.get(w.char_guid,{});camera=lambda: wf.tracker.object_values.get(w.char_guid,{}).get(0x471,0)+(wf.tracker.object_values.get(w.char_guid,{}).get(0x472,0)<<32)
 report['camera_during_ride']=camera();report['flight_guid']=flight;report['camera_follows_vehicle']=camera()==flight
 assert report['camera_follows_vehicle'],'Camera does not point at escape vehicle'
 end=time.monotonic()+180
 while time.monotonic()<end:
  w.pump(1)
  if not wf.tracker.actors(43713):break
 report['camera_after_ride']=camera();w.send_chat('say','.save');w.pump(1)
 row=d.q('Character',f'SELECT map,position_x,position_y,position_z FROM characters WHERE guid={guid}')[0];report['returned_position']=row
 report['quest_before_turnin']=d.q('Character',f'SELECT status FROM character_queststatus WHERE guid={guid} AND quest=26706')
 # Synchronize the QA client's movement origin after the authoritative server teleport.
 # This is local bookkeeping, not a second server teleport.
 w.pos=tuple(map(float,row[1:4]));w.move_guid=None
 w.move_walk([(-1299.28,2128.4,5.63)]);w.pump(.5);w.send_chat('say','.save');w.pump(1)
 row=d.q('Character',f'SELECT position_x,position_y,position_z FROM characters WHERE guid={guid}')[0];report['walked_server_position']=row
 report['walking_accepted']=abs(float(row[0])+1299.28)<1 and abs(float(row[1])-2128.4)<1
 lorna=wf.tracker.actors(43727)[0][0];wf.menu_events.clear();w.quest_hello(lorna);w.pump(1);w.quest_complete(26706,lorna);w.pump(1)
 report['turnin_dialogue']=bool(wf.menu_events);report['response_opcodes']=[hex(x) for x in wf.menu_events]
 w.quest_reward(26706,0,lorna);w.pump(3);w.send_chat('say','.save');w.pump(1)
 report['rewarded']=d.q('Character',f'SELECT quest FROM character_queststatus_rewarded WHERE guid={guid} AND quest=26706')==[['26706']]
 report['passed']=report['camera_follows_vehicle'] and report['camera_after_ride']==0 and report['walking_accepted'] and report['turnin_dialogue'] and report['rewarded']
 print('Escape camera, walking and actual turn-in',report['passed'],flush=True);report['logout_acknowledged']=w.logout()

finally:
 if w:
  try:w.logout()
  except Exception:pass
  w.s.close()
 if aid:report['cleanup']=cleanup_owned(d,pid,account,name,aid,guid)
 (R/'out/wyvern-camera-turnin-regression.json').write_text(json.dumps(report,indent=2,default=str))
 print('Report saved',flush=True)
