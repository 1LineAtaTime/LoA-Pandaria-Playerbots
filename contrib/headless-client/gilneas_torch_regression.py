"""Disposable seeded Half-Burnt Torch item-use regression; no GM cast/kill.
Requires local test server; only the temporary private-phase rat is targeted.
"""
import sys,time,secrets,hashlib,json,subprocess
from pathlib import Path
R=Path(__file__).resolve().parents[2];sys.path.insert(0,str(R/'contrib/headless-client'))
import headless
from starter_campaign import Database
from qa_actions import Workflow
from qa_cleanup import cleanup_owned
headless.log=lambda *a,**k:None
d=Database(R/'exports/worldserver.conf',R/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
pid=int(subprocess.check_output(['powershell','-NoProfile','-Command','Get-Process worldserver | Select-Object -ExpandProperty Id'],text=True).strip())
name='Qbattle'+''.join(secrets.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(3));account=name.upper();password=secrets.token_hex(10).upper();guid=None;w=None
report=dict(fixture_only=True,quest=24678,passed=False,isolated_rat=True)
aid=None
try:
 sha=hashlib.sha1((account+':'+password).encode()).hexdigest();d.q('Login',"INSERT INTO account(username,sha_pass_hash,expansion) VALUES('"+account+"','"+sha+"',4)");aid=int(d.q('Login',"SELECT id FROM account WHERE username='"+account+"'")[0][0]);report.update(account=account,name=name,account_id=aid)
 d.q('Login',f'INSERT INTO account_access (id,gmlevel,RealmID) VALUES ({aid},3,-1)')
 key=headless.auth_login('127.0.0.1',3724,account,password);w=headless.WorldClient('127.0.0.1',8085,key,account);w.emit=lambda *a,**k:None;w.handshake(key,realm_id=1);w.char_create(name,22,4)
 end=time.monotonic()+30
 while time.monotonic()<end:
  op,b=w.read_packet()
  if op==headless.SMSG_CHAR_CREATE:assert b==bytes([47]);break
 w.char_enum();guid=int(d.q('Character',"SELECT guid FROM characters WHERE name='"+name+"'")[0][0]);report['guid']=guid;(R/'out/torch-active-fixture.json').write_text(json.dumps(report))
 d.q('Character',f'UPDATE characters SET level=12,health=1000,map=654,zone=4755,position_x=-1668.16,position_y=1664.28,position_z=20.5732 WHERE guid={guid}')
 d.q('Character',f'INSERT INTO character_queststatus_rewarded (guid,quest,active) VALUES '+','.join(f'({guid},{q},1)' for q in (14386,14466,24676,24904,24902,24903,24920)))
 d.q('Character',f'INSERT INTO character_queststatus (guid,quest,status) VALUES ({guid},24678,3)')

 workflow=Workflow(w,d,guid,dict(quest=24678),report,lambda x:None)
 packets=[];original_read=w.read_packet
 def capture():
  op,b=original_read()
  if op==headless.SMSG_SPELL_GO:packets.append(b)
  return op,b
 w.read_packet=capture
 assert w.player_login(guid);w.pump(3)
 w.send_chat('say','.gm off');w.pump(1)
 w.send_chat('say','.modify phase 1073741824');w.pump(1)
 w.send_chat('say','.npc add temp 37889');w.pump(2)
 actors=workflow.tracker.actors(37889);assert len(actors)==1, 'Expected only isolated temporary rat'
 target,pos=actors[0];report['temporary_actor']=target;report['fixture_map']=w.map_id; (R/'out/torch-active-fixture.json').write_text(json.dumps(report))
 assert w.map_id==654 and (target>>32)&0xfffff==37889, 'Only test the owned rat'
 w.send_chat('say','.additem 50220');w.pump(2);w.send_chat('say','.save');w.pump(2)
 items=d.q('Character',f'SELECT ci.bag,ci.slot,i.guid FROM character_inventory ci JOIN item_instance i ON i.guid=ci.item WHERE ci.guid={guid} AND i.itemEntry=50220');assert len(items)==1 and items[0][0]=='0', 'Torch inventory slot unavailable'
 before=workflow.tracker.health.get(target);assert before and before>0
 w.use_item(255,int(items[0][1]),(0x400<<52)|int(items[0][2]));w.pump(3)
 after=workflow.tracker.health.get(target,0);report.update(rat_health_before=before,rat_health_after=after,torch_item=50220,torch_response_passed=after==0)
 report['passed']=report['torch_response_passed'];print('Actual torch item response',before,after,flush=True)
 w.set_selection(target);w.send_chat('say','.npc delete');w.pump(2)
 w.send_chat('say','.combatstop');w.pump(1)

 report['logout_acknowledged']=w.logout()
finally:
 if w:
  try:w.logout()
  except Exception:pass
  w.s.close()
 if aid:
  report['cleanup']=cleanup_owned(d,pid,account,name,aid,guid)
 (R/'out/torch-regression.json').write_text(json.dumps(report,indent=2,default=str))
 print('Report saved; probe passed:',report['passed'],flush=True)
