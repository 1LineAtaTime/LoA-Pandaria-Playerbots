"""Disposable mounted Grand Expedition Yak merchant and reforger service probe."""
import hashlib,json,secrets,struct,subprocess,sys,time
from pathlib import Path
R=Path(__file__).resolve().parents[2];sys.path.insert(0,str(R/'contrib/headless-client'))
import headless
from starter_campaign import Database
from qa_actions import Workflow
from qa_cleanup import cleanup_owned
headless.log=lambda *a,**k:None
d=Database(R/'exports/worldserver.conf',R/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe');pid=int(subprocess.check_output(['powershell','-NoProfile','-Command','Get-Process worldserver | Select-Object -ExpandProperty Id'],text=True).strip())
name='Qyak'+''.join(secrets.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(5));account=name.upper();password=secrets.token_hex(10).upper();aid=guid=None;w=None;report=dict(passed=False,fixture_only=True);O=R/'out/orb-yak-20261112';events=[]
try:
 sha=hashlib.sha1((account+':'+password).encode()).hexdigest();d.q('Login',"INSERT INTO account(username,sha_pass_hash,expansion) VALUES('"+account+"','"+sha+"',4)");aid=int(d.q('Login',"SELECT id FROM account WHERE username='"+account+"'")[0][0]);d.q('Login',f'INSERT INTO account_access(id,gmlevel,RealmID) VALUES({aid},3,-1)');(O/'yak-fixture.json').write_text(json.dumps(dict(account=account,name=name,account_id=aid)))
 key=headless.auth_login('127.0.0.1',3724,account,password);w=headless.WorldClient('127.0.0.1',8085,key,account);w.handshake(key,realm_id=1);w.emit=lambda event,**data:events.append(dict(event=event,**data));w.char_create(name,1,1)
 end=time.monotonic()+30
 while time.monotonic()<end:
  op,b=w.read_packet()
  if op==headless.SMSG_CHAR_CREATE:assert b==bytes([47]);break
 w.char_enum();guid=int(d.q('Character',"SELECT guid FROM characters WHERE name='"+name+"'")[0][0]);d.q('Character',f'UPDATE characters SET level=90,health=100000 WHERE guid={guid}');report.update(account_id=aid,guid=guid,name=name,account=account);(O/'yak-fixture.json').write_text(json.dumps(report));wf=Workflow(w,d,guid,dict(quest=0),report,lambda _:None);assert w.player_login(guid);w.pump(2)
 for spell in (33388,33391,122708):w.send_chat('say','.learn '+str(spell));w.pump(.4)
 w.mount(122708);w.pump(5);merchant=wf.tracker.actors(62822);bird=wf.tracker.actors(62821);report['cast_events']=w.events;report['passengers']={str(e):len(wf.tracker.actors(e)) for e in (62821,62822)};assert merchant and bird,'Mounted passengers missing'
 report['npc_flags']={str(e):wf.tracker.npc_flags.get(g,0) for e,g in [(62822,merchant[0][0]),(62821,bird[0][0])]};assert report['npc_flags']['62821']&134217728
 responses=[];read=w.read_packet
 def capture():
  op,b=read()
  if op in (0x1aae,0x141e):responses.append((op,b))
  return op,b
 w.read_packet=capture;w.send_packet(0x02d8,headless.packed_guid(merchant[0][0],(6,7,3,1,2,0,4,5),(0,7,1,6,4,3,5,2)));w.pump(1);vendor=[b for op,b in responses if op==0x1aae];assert vendor;reader=headless.ByteReader(vendor[-1]);reader.read_bits(5);report['vendor_items']=reader.read_bits(18);assert report['vendor_items']==14
 slot=int(d.q('Character',f'SELECT slot FROM character_inventory WHERE guid={guid} AND bag=0 AND slot<19 LIMIT 1')[0][0]);npc=bird[0][0];body=struct.pack('<III',0,255,slot)+headless.guid_mask_bits(npc,(1,0,5,3,4,2,7,6))+headless.guid_byte_seq(npc,(4,6,3,1,2,7,0,5));w.send_packet(0x0c4f,body);w.pump(1);reforges=[b for op,b in responses if op==0x141e];report['reforge_response']=[b.hex() for b in reforges];assert reforges and reforges[-1][0]&128
 report['passed']=True;assert w.logout();print('Mounted yak:14 vendor items and successful Birdhat reforge response',flush=True)
finally:
 if w:
  try:w.logout()
  except Exception:pass
  w.s.close()
 if aid:report['cleanup']=cleanup_owned(d,pid,account,name,aid,guid)
 report['events']=events;(O/'yak-services.json').write_text(json.dumps(report,indent=2,default=str))
