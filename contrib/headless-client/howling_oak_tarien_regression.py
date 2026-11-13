"""Disposable Howling Oak quest turn-in and Tarien merchant interaction regression.
Seeds only owned fixture prerequisites; normal quest reward and gossip packets.
Ordinary quest and merchant packets verify restored world data.
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
name='Qoak'+''.join(secrets.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(4));account=name.upper();password=secrets.token_hex(10).upper();aid=guid=None;w=None
report=dict(fixture_only=True,passed=False);(R/'out/howling-oak-active-fixture.json').write_text(json.dumps(dict(account=account,name=name)))
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
 w.char_enum();guid=int(d.q('Character',"SELECT guid FROM characters WHERE name='"+name+"'")[0][0]);report.update(account_id=aid,guid=guid,name=name,account=account);(R/'out/howling-oak-active-fixture.json').write_text(json.dumps(report))


 d.q('Character',f'UPDATE characters SET level=12,health=1000,map=1,zone=141,position_x=8355,position_y=991,position_z=21.86 WHERE guid={guid}')
 d.q('Character',f'INSERT INTO character_queststatus_rewarded(guid,quest,active) VALUES({guid},26706,1),({guid},14434,1)')
 wf=Workflow(w,d,guid,dict(quest=28517),report,lambda x:None);assert w.player_login(guid);w.pump(3);w.send_chat('say','.gm off');w.pump(.3)
 krennan=wf.tracker.actors(42968)[0][0];w.quest_hello(krennan);w.pump(.5);w.quest_accept(28517,krennan);w.pump(.5)
 w.send_chat('say','.go xyz 10313.7 2442.08 1330.1 1');w.pump(4);w.send_chat('say','.modify phase 1');w.pump(.5)
 genn=wf.tracker.actors(48736);assert len(genn)==1,'Genn missing or duplicated';genn=genn[0][0]
 wf.menu_events.clear();w.quest_hello(genn);w.pump(.5);w.quest_complete(28517,genn);w.pump(.5);report['turnin_response_opcodes']=[hex(x) for x in wf.menu_events]
 assert 0x074f in wf.menu_events,'No normal offer-reward dialogue'
 w.quest_reward(28517,0,genn);w.pump(1);w.send_chat('say','.save');w.pump(.5)
 report['quest_rewarded']=d.q('Character',f'SELECT quest FROM character_queststatus_rewarded WHERE guid={guid} AND quest=28517')==[['28517']];assert report['quest_rewarded']
 w.send_chat('say','.go xyz 10149.4 2350.4 1332.05 1');w.pump(4)
 vendor=wf.tracker.actors(52644)[0][0];packets=[];original=w.read_packet
 def capture():
  op,b=original()
  if op==0x1aae:packets.append(b)
  return op,b
 w.read_packet=capture
 w.send_packet(0x02d8,headless.packed_guid(vendor,(6,7,3,1,2,0,4,5),(0,7,1,6,4,3,5,2)));w.pump(1)
 assert packets,'No merchant inventory response'
 reader=headless.ByteReader(packets[-1]);reader.read_bits(5);report['merchant_item_count']=reader.read_bits(18);report['merchant_response_bytes']=len(packets[-1]);assert report['merchant_item_count']>0
 report['passed']=True;report['logout_acknowledged']=w.logout();print('Howling Oak turn-in and Tarien stock passed:',report['merchant_item_count'],'items',flush=True)
finally:
 if w:
  try:
   if not report.get('logout_acknowledged'):w.logout()
  except Exception:pass
  w.s.close()
 if aid:report['cleanup']=cleanup_owned(d,pid,account,name,aid,guid)
 (R/'out/howling-oak-tarien-regression.json').write_text(json.dumps(report,indent=2,default=str))
