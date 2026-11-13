"""Owned two-character currency/rollback regression and mounted yak services.
Never changes a real account balance or character progress. Max2 characters/account.
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
name='Qorb'+''.join(secrets.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(5));alt='Qalt'+''.join(secrets.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(5));account=name.upper();password=secrets.token_hex(10).upper();aid=guid=aguid=None;w=None;trigger=None
O=R/'out/orb-yak-20261112';O.mkdir(parents=True,exist_ok=True);report=dict(passed=False,checks={},fixture_only=True);messages=[]
def connect(g):
 key=headless.auth_login('127.0.0.1',3724,account,password);c=headless.WorldClient('127.0.0.1',8085,key,account);c.handshake(key,realm_id=1);c.emit=lambda event,**kw:messages.append(kw) if event=='chat' else None;c.char_enum();assert c.player_login(g);c.pump(2);return c
def orbrows(g):return d.q('Character',f'SELECT i.guid,c.bag,c.slot,i.count,i.flags FROM item_instance i JOIN character_inventory c ON c.item=i.guid WHERE c.guid={g} AND i.itemEntry=35626 ORDER BY i.guid')
def amount():return int(d.q('Login',f'SELECT dp FROM account WHERE id={aid}')[0][0])
def qty(g):return sum(int(x[3]) for x in orbrows(g))
def use(g):
 row=orbrows(g)[0];bag=255 if row[1]=='0' else int(d.q('Character',f'SELECT slot FROM character_inventory WHERE guid={g} AND item={row[1]}')[0][0]);w.use_item(bag,int(row[2]),(0x400<<52)|int(row[0]));w.pump(.8)
try:
 sha=hashlib.sha1((account+':'+password).encode()).hexdigest();d.q('Login',"INSERT INTO account(username,sha_pass_hash,expansion) VALUES('"+account+"','"+sha+"',4)");aid=int(d.q('Login',"SELECT id FROM account WHERE username='"+account+"'")[0][0]);d.q('Login',f'INSERT INTO account_access(id,gmlevel,RealmID) VALUES({aid},3,-1)');(O/'active-fixture.json').write_text(json.dumps(dict(account=account,account_id=aid,name=name,alt=alt)))
 key=headless.auth_login('127.0.0.1',3724,account,password);w=headless.WorldClient('127.0.0.1',8085,key,account);w.handshake(key,realm_id=1);w.emit=lambda *a,**k:None
 for n in [name,alt]:
  w.char_create(n,1,1)
  end=time.monotonic()+30
  while time.monotonic()<end:
   op,b=w.read_packet()
   if op==headless.SMSG_CHAR_CREATE:assert b==bytes([47]);break
 rows=d.q('Character',f'SELECT name,guid FROM characters WHERE account={aid}');ids=dict(rows);guid=int(ids[name]);aguid=int(ids[alt]);d.q('Character',f'UPDATE characters SET level=90,health=100000,money=10000000 WHERE account={aid}')
 report.update(account_id=aid,guid=guid,alt_guid=aguid,name=name,alt=alt,account=account);(O/'active-fixture.json').write_text(json.dumps(report));w.char_enum();assert w.player_login(guid);w.pump(2)
 w.send_chat('say','.botscenario '+name+' additem 35626 22');w.pump(1);w.send_chat('say','.save');w.pump(1)
 report['initial_stacks']=orbrows(guid);assert sorted(int(x[3]) for x in report['initial_stacks'])==[2,20];assert all(int(x[4])&1 for x in report['initial_stacks']);report['checks']['stack20_and_bound']=True
 for n in range(3):use(guid);assert qty(guid)==21-n and amount()==100*(n+1)
 report['checks']['one_item_per_100_dp']=True
 d.q('Login',f'UPDATE account SET dp=2147483600 WHERE id={aid}');before=qty(guid);use(guid);assert amount()==2147483600 and qty(guid)==before;d.q('Login',f'UPDATE account SET dp=300 WHERE id={aid}');report['checks']['overflow_retains_orb']=True
 # A fixture-only duplicate receipt forces rollback AFTER the wallet/item update.
 existing=int(d.q('Character',f'SELECT MIN(id) FROM battlepay_orb_redemption WHERE account_id={aid}')[0][0]);trigger='qa_orb_rollback_'+str(guid)
 d.q('Character',f'CREATE TRIGGER {trigger} BEFORE INSERT ON battlepay_orb_redemption FOR EACH ROW SET NEW.id=IF(NEW.character_guid={guid},{existing},NEW.id)')
 try:use(guid)
 except Exception:pass
 d.q('Character','DROP TRIGGER '+trigger);trigger=None;w.s.close();w=None
 for _ in range(30):
  if d.q('Character',f'SELECT online FROM characters WHERE guid={guid}')==[['0']]:break
  time.sleep(.25)
 assert amount()==300 and qty(guid)==before;w=connect(guid);assert qty(guid)==before;report['checks']['forced_receipt_failure_rolls_back_wallet_and_item']=True
 assert w.logout();w.s.close();w=connect(aguid);w.send_chat('say','.botscenario '+alt+' additem 35626 1');w.pump(.7);w.send_chat('say','.save');w.pump(.7)
 packets=[];original=w.send_packet
 def capture(op,body):
  if op==headless.CMSG_USE_ITEM:packets.append((op,body))
  return original(op,body)
 w.send_packet=capture;use(aguid);assert qty(aguid)==0 and amount()==400;assert packets;original(*packets[-1]);w.pump(.5);assert amount()==400;report['checks']['account_shared_last_item_and_replay']=True
 # Test actual mount passengers and existing service opcodes, not stationary stand-ins.
 wf=Workflow(w,d,aguid,dict(quest=0),report,lambda _:None);w.send_chat('say','.learn 33388');w.pump(.3);w.send_chat('say','.learn 33391');w.pump(.3);w.send_chat('say','.learn 122708');w.pump(.4);w.mount(122708);w.pump(4)
 merchant=wf.tracker.actors(62822);bird=wf.tracker.actors(62821);assert merchant and bird,'Yak passengers not spawned';report['yak_npc_flags']={str(e):wf.tracker.npc_flags.get(g,0) for e,g in [(62822,merchant[0][0]),(62821,bird[0][0])]};assert report['yak_npc_flags']['62821']&134217728
 responses=[];read=w.read_packet
 def read_capture():
  op,b=read()
  if op in (0x1aae,0x141e):responses.append((op,b))
  return op,b
 w.read_packet=read_capture;w.send_packet(0x02d8,headless.packed_guid(merchant[0][0],(6,7,3,1,2,0,4,5),(0,7,1,6,4,3,5,2)));w.pump(1)
 vendor=[b for op,b in responses if op==0x1aae];assert vendor;reader=headless.ByteReader(vendor[-1]);reader.read_bits(5);report['yak_vendor_items']=reader.read_bits(18);assert report['yak_vendor_items']==14;report['checks']['mounted_cousin_stock']=True
 slot=int(d.q('Character',f'SELECT slot FROM character_inventory WHERE guid={aguid} AND bag=0 AND slot<19 LIMIT1'.replace('LIMIT1','LIMIT 1'))[0][0]);npc=bird[0][0];body=struct.pack('<III',0,255,slot)+headless.guid_mask_bits(npc,(1,0,5,3,4,2,7,6))+headless.guid_byte_seq(npc,(4,6,3,1,2,7,0,5));w.send_packet(0x0c4f,body);w.pump(1)
 reforges=[b for op,b in responses if op==0x141e];assert reforges and reforges[-1] and reforges[-1][0]&128;report['checks']['mounted_birdhat_reforge_request']=True
 report['passed']=True;assert w.logout();print('Orb currency, rollback, replay and mounted yak service checks passed',flush=True)
finally:
 if trigger:d.q('Character','DROP TRIGGER IF EXISTS '+trigger)
 if w:
  try:w.logout()
  except Exception:pass
  w.s.close()
 if aguid:
  subprocess.run([sys.executable,str(R/'contrib/windows/Send-ServerConsoleCommand.py'),str(pid),'character erase '+alt],check=True)
  for _ in range(40):
   if not d.q('Character',f'SELECT guid FROM characters WHERE guid={aguid}'):break
   time.sleep(.25)
 if aid:
  report['cleanup']=cleanup_owned(d,pid,account,name,aid,guid);d.q('Character',f'DELETE FROM battlepay_orb_redemption WHERE account_id={aid}')
 report['messages']=messages;(O/'report.json').write_text(json.dumps(report,indent=2,default=str))
