"""Disposable seeded Gilneas vehicle regression; requires local test server and GM account creation access.
Checks boarding/spam/exit and actual ship arrival, not full natural quest combat or rewards.
Generated credentials stay in memory; owned identities are removed through native console commands.
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
 d.q('Character',f'UPDATE characters SET level=12,health=1000,map=654,zone=4714,position_x=-1326,position_y=2104,position_z=5.72 WHERE guid={guid}')
 d.q('Character',f'INSERT INTO character_queststatus_rewarded(guid,quest,active) VALUES '+','.join(f'({guid},{q},1)' for q in (14386,14466,24676,24904,24902,24903,24920,24678,24679,24680)))
 d.q('Character',f'INSERT INTO character_queststatus(guid,quest,status) VALUES({guid},24681,3)')
 wf=Workflow(w,d,guid,dict(quest=24681),report,lambda x:None);assert w.player_login(guid);w.pump(3)
 w.send_chat('say','.gm off');w.pump(.5)
 launchers=wf.tracker.actors(38150);report['natural_phase_launcher_visible']=bool(launchers);assert launchers,'No launcher in natural phase'
 launcher=min(launchers,key=lambda x:sum((a-b)**2 for a,b in zip(x[1],w.pos)))[0]
 for i in range(20):w.gossip_hello(launcher);w.spellclick(launcher)
 w.pump(3)
 vehicles=wf.tracker.actors(41599);report['after_20_gossip_and_20_spellclick']=len(vehicles);assert len(vehicles)==1,'Duplicate or missing glaive'
 vg=vehicles[0][0];report['glaive_owner_fields']={hex(k):v for k,v in wf.tracker.object_values.get(vg,{}).items() if k<32};report['client_guid']=w.char_guid;report['glaive_charmed_by_fixture']=owner(wf.tracker,vg)==w.char_guid;w.pump(4);report['glaive_still_boarded_after_cleanup_deadline']=bool(wf.tracker.actors(41599));print('Glaive boarding and bounded allocation checked',flush=True)
 w.send_packet(0x1DC3);w.pump(6);report['empty_glaive_removed']=not wf.tracker.actors(41599);assert report['empty_glaive_removed']
 assert w.logout();w.s.close();w=None
 d.q('Character',f'DELETE FROM character_queststatus WHERE guid={guid} AND quest=24681');d.q('Character',f'INSERT INTO character_queststatus_rewarded(guid,quest,active) VALUES({guid},24681,1)');d.q('Character',f'INSERT INTO character_queststatus(guid,quest,status) VALUES({guid},26706,3)');d.q('Character',f'UPDATE characters SET map=654,position_x=-1290.64,position_y=2134.84,position_z=5.63 WHERE guid={guid}')
 w=connect();w.char_enum();wf=Workflow(w,d,guid,dict(quest=26706),report,lambda x:None);assert w.player_login(guid);w.pump(3)
 birds=wf.tracker.actors(43747);report['natural_phase_hippogryph_visible']=bool(birds);assert birds,'No hippogryph in natural phase'
 bird=birds[0][0];flight=None;end=time.monotonic()+140
 while time.monotonic()<end:
  w.gossip_hello(bird);w.pump(3.2)
  flights=wf.tracker.actors(43751)
  if flights:flight=flights[0][0];break
 assert flight,'Hippogryph did not board in signal window';report['flight_charmed_by_fixture']=owner(wf.tracker,flight)==w.char_guid
 end=time.monotonic()+95
 while time.monotonic()<end:
  w.pump(2)
  if wf.tracker.actors(43566) and (not wf.tracker.actors(43751) or owner(wf.tracker,flight)==0):break
 w.send_chat('say','.save');w.pump(2)
 row=d.q('Character',f'SELECT map,position_x,position_y,position_z FROM characters WHERE guid={guid}')[0];report['flight_saved_position']=row;report['ship_lorna_visible']=bool(wf.tracker.actors(43566));report['arrived_at_ship']=int(row[0])==654 and 3200<float(row[2])<3330 and float(row[3])>100
 report['gameplay_checks_passed']=all(report.get(k) for k in ['natural_phase_launcher_visible','glaive_charmed_by_fixture','empty_glaive_removed','natural_phase_hippogryph_visible','flight_charmed_by_fixture','ship_lorna_visible','arrived_at_ship'])
 print('Vehicle gameplay checks passed',report['gameplay_checks_passed'],flush=True)
 report['logout_acknowledged']=w.logout();report['passed']=report['gameplay_checks_passed'] and report['logout_acknowledged']
finally:
 if w:
  try:w.logout()
  except Exception:pass
  w.s.close()
 if aid:report['cleanup']=cleanup_owned(d,pid,account,name,aid,guid)
 (R/'out/finale-vehicles-regression.json').write_text(json.dumps(report,indent=2,default=str))
 print('Report saved',flush=True)
