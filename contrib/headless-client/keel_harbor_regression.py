"""Disposable vehicle durability/vendor/quest-eligibility check on a local test realm.
Prerequisite rewards are seeded only on the owned fixture; no user progress is changed.
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
  if op in (0x9d8,0x143a,0x095a,0x4af,0x40b,0x139,0x156,0x1aae):packets.append([hex(op),b.hex()])
  return op,b
 w.read_packet=capture;w.char_create(name,22,4)
 end=time.monotonic()+30
 while time.monotonic()<end:
  op,b=w.read_packet()
  if op==headless.SMSG_CHAR_CREATE:assert b==bytes([47]);break
 w.char_enum();guid=int(d.q('Character',"SELECT guid FROM characters WHERE name='"+name+"'")[0][0]);report.update(account_id=aid,guid=guid,name=name,account=account);(R/'out/finale-active-fixture.json').write_text(json.dumps(report))
 d.q('Character',f'UPDATE characters SET level=12,health=1000,map=654,zone=4714,position_x=-1326,position_y=2104,position_z=5.72 WHERE guid={guid}')
 d.q('Character',f'INSERT INTO character_queststatus_rewarded(guid,quest,active) VALUES '+','.join(f'({guid},{q},1)' for q in (14386,14466,24676,24904,24902,24903,24920,24678,24679,24680)))
 d.q('Character',f'INSERT INTO character_queststatus(guid,quest,status) VALUES({guid},24681,3)')
 wf=Workflow(w,d,guid,dict(quest=24681),report,lambda x:None);assert w.player_login(guid);w.pump(3)
 w.send_chat('say','.gm off');w.pump(.5)
 w.send_chat('say','.modify phase 1073741824');w.pump(.5)
 w.send_chat('say','.npc add temp 38150');w.pump(.5)
 launcher=wf.tracker.actors(38150)[0][0];w.gossip_hello(launcher);w.pump(1)
 vehicle=wf.tracker.actors(41599)[0][0];before=wf.tracker.health[vehicle]
 w.send_chat('say','.npc add temp 37916');w.pump(8)
 target=wf.tracker.actors(37916)[0][0];after=wf.tracker.health.get(vehicle,0)
 report.update(vehicle_max_health=wf.tracker.max_health.get(vehicle),vehicle_health_before=before,vehicle_health_after=after,survived_normal_enemy_hits=0<after<before)
 print('Glaive durability',before,after,flush=True)
 w.set_selection(target);w.send_chat('say','.npc delete');w.pump(.5);w.send_packet(0x1DC3);w.pump(2)
 w.set_selection(launcher);w.send_chat('say','.npc delete');w.pump(.5)
 w.send_chat('say','.modify phase 1048576');w.pump(.5)
 vendor=wf.approach(38783);w.gossip_hello(vendor);w.pump(1)
 options=[o for o in (w.gossip or {}).get('options',[]) if 'browse' in o['text'].lower()];report['vendor_option_visible']=len(options)==1;assert options,'Merchant option missing'
 w.gossip_select(options[0]['id']);w.pump(1);report['vendor_inventory_packet']=any(op=='0x1aae' and len(body)>80 for op,body in packets)
 assert w.logout();w.s.close();w=None
 d.q('Character',f'DELETE FROM character_queststatus WHERE guid={guid} AND quest=24681');d.q('Character',f'INSERT INTO character_queststatus_rewarded(guid,quest,active) VALUES({guid},24681,1)');d.q('Character',f'UPDATE characters SET map=654,position_x=-1299.28,position_y=2128.4,position_z=5.63 WHERE guid={guid}')
 w=connect();w.char_enum();wf=Workflow(w,d,guid,dict(quest=26706),report,lambda x:None);assert w.player_login(guid);w.pump(2)
 lorna=wf.tracker.actors(43727)[0][0];w.quest_hello(lorna);w.pump(1);report['endgame_offer_after_prerequisite']=bool(wf.menu_events)
 w.quest_accept(26706,lorna);w.pump(1);w.send_chat('say','.save');w.pump(1);report['endgame_accepted']=d.q('Character',f'SELECT status FROM character_queststatus WHERE guid={guid} AND quest=26706')==[['3']]
 report['passed']=all(report.get(k) for k in ['survived_normal_enemy_hits','vendor_option_visible','vendor_inventory_packet','endgame_offer_after_prerequisite','endgame_accepted'])
 print('Follow-up gameplay checks',report['passed'],flush=True);report['logout_acknowledged']=w.logout()

finally:
 if w:
  try:w.logout()
  except Exception:pass
  w.s.close()
 if aid:report['cleanup']=cleanup_owned(d,pid,account,name,aid,guid)
 (R/'out/keel-harbor-followup-regression.json').write_text(json.dumps(report,indent=2,default=str))
 print('Report saved',flush=True)
