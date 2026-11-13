"""Disposable Nightwind departure and repeated companion lifetime regression.
Seeds only owned fixture prerequisites; normal quest reward and gossip packets.
The source crash was an invalid spline read; this exercises a repaired stale registry path.
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
name='Qdepart'+''.join(secrets.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(4));account=name.upper();password=secrets.token_hex(10).upper();aid=guid=None;w=None
report=dict(fixture_only=True,passed=False);(R/'out/departure-active-fixture.json').write_text(json.dumps(dict(account=account,name=name)))
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
 w.char_enum();guid=int(d.q('Character',"SELECT guid FROM characters WHERE name='"+name+"'")[0][0]);report.update(account_id=aid,guid=guid,name=name,account=account);(R/'out/departure-active-fixture.json').write_text(json.dumps(report))

 d.q('Character',f'UPDATE characters SET level=90,health=1000,map=654,zone=4714,position_x=-1285,position_y=2099,position_z=5.3 WHERE guid={guid}')
 d.q('Character',f'INSERT INTO character_queststatus_rewarded(guid,quest,active) VALUES({guid},26706,1)')
 wf=Workflow(w,d,guid,dict(quest=14434),report,lambda x:None);assert w.player_login(guid);w.pump(3)
 w.send_chat('say','.gm off');w.send_chat('say','.modify phase 1048576');w.pump(1)
 night=wf.tracker.actors(36616)[0][0];w.gossip_hello(night);w.pump(1);report['initial_menu']=w.gossip
 assert 14434 in [q['id'] for q in w.gossip['quests']],'Departure quest not offered'
 w.quest_accept(14434,night);w.pump(1);w.quest_complete(14434,night);w.pump(1);w.quest_reward(14434,0,night);w.pump(5)
 w.send_chat('say','.save');w.pump(1)
 def position():return d.q('Character',f'SELECT map,position_x,position_y,position_z FROM characters WHERE guid={guid}')[0]
 report['normal_departure_position']=position();assert report['normal_departure_position'][0]=='1','Normal reward did not teleport'
 report['bind']=d.q('Character',f'SELECT mapId,zoneId,posX,posY,posZ FROM character_homebind WHERE guid={guid}')
 assert report['bind'][0][0]=='1','Bind did not move'
 w.send_chat('say','.go xyz -1285 2099 5.3 654');w.pump(4);w.send_chat('say','.modify phase 1048576');w.pump(1)
 night=wf.tracker.actors(36616)[0][0];w.gossip=None;w.gossip_hello(night);w.pump(1);report['retry_menu']=w.gossip
 options=[o for o in w.gossip['options'] if 'ready to sail' in o['text']];assert len(options)==1,'Stranded-player retry option absent'
 w.gossip_select(options[0]['id']);w.pump(5);w.send_chat('say','.save');w.pump(1);report['retry_position']=position();assert report['retry_position'][0]=='1'
 w.send_chat('say','.go xyz 8355 990.66 21.85 1');w.pump(4);w.send_chat('say','.modify phase 1');w.pump(1)
 krennan=wf.tracker.actors(42968)[0][0];w.gossip=None;wf.menu_events.clear();w.quest_hello(krennan);w.pump(1)
 report['howling_oak_menu']=w.gossip;report['howling_oak_opcodes']=[hex(x) for x in wf.menu_events];assert wf.menu_events,'No next quest dialogue response'
 # Some quest-only NPCs directly open quest details instead of a gossip list.
 w.quest_accept(28517,krennan);w.pump(1);w.send_chat('say','.save');w.pump(1)
 report['howling_oak_accepted']=bool(d.q('Character',f'SELECT quest FROM character_queststatus WHERE guid={guid} AND quest=28517'))
 assert report['howling_oak_accepted'],'Next chain quest unavailable'
 report['passed']=True;report['logout_acknowledged']=w.logout();print('Nightwind departure/retry and Howling Oak passed',flush=True)
finally:
 if w:
  try:
   if not report.get('logout_acknowledged'):w.logout()
  except Exception:pass
  w.s.close()
 if aid:report['cleanup']=cleanup_owned(d,pid,account,name,aid,guid)
 (R/'out/nightwind-departure-regression.json').write_text(json.dumps(report,indent=2,default=str))
