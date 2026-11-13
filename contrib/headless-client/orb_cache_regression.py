"""Disposable Orb of Loyalty native cache invalidation and metadata probe."""
import hashlib,json,secrets,struct,subprocess,sys,time
from pathlib import Path
R=Path(__file__).resolve().parents[2];sys.path.insert(0,str(R/'contrib/headless-client'))
import headless
from starter_campaign import Database
from qa_actions import Workflow
from qa_cleanup import cleanup_owned
headless.log=lambda *a,**k:None
d=Database(R/'exports/worldserver.conf',R/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe');pid=int(subprocess.check_output(['powershell','-NoProfile','-Command','Get-Process worldserver | Select-Object -ExpandProperty Id'],text=True).strip())
name='Qcache'+''.join(secrets.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(5));account=name.upper();password=secrets.token_hex(10).upper();aid=guid=None;w=None;report=dict(passed=False,fixture_only=True);O=R/'out/orb-cache-20261112';O.mkdir(parents=True,exist_ok=True);events=[]
try:
 sha=hashlib.sha1((account+':'+password).encode()).hexdigest();d.q('Login',"INSERT INTO account(username,sha_pass_hash,expansion) VALUES('"+account+"','"+sha+"',4)");aid=int(d.q('Login',"SELECT id FROM account WHERE username='"+account+"'")[0][0]);d.q('Login',f'INSERT INTO account_access(id,gmlevel,RealmID) VALUES({aid},3,-1)');(O/'cache-fixture.json').write_text(json.dumps(dict(account=account,name=name,account_id=aid)))
 key=headless.auth_login('127.0.0.1',3724,account,password);w=headless.WorldClient('127.0.0.1',8085,key,account);w.handshake(key,realm_id=1);w.emit=lambda event,**data:events.append(dict(event=event,**data));w.char_create(name,1,1)
 end=time.monotonic()+30
 while time.monotonic()<end:
  op,b=w.read_packet()
  if op==headless.SMSG_CHAR_CREATE:assert b==bytes([47]);break
 w.char_enum();guid=int(d.q('Character',"SELECT guid FROM characters WHERE name='"+name+"'")[0][0]);d.q('Character',f'UPDATE characters SET level=90,health=100000 WHERE guid={guid}');report.update(account_id=aid,guid=guid,name=name,account=account);(O/'cache-fixture.json').write_text(json.dumps(report));wf=Workflow(w,d,guid,dict(quest=0),report,lambda _:None);assert w.player_login(guid);w.pump(2)
 responses=[];read=w.read_packet
 def capture():
  op,b=read()
  if op==0x103b:responses.append(b)
  return op,b
 w.read_packet=capture
 for typ in (1344507586,2442913102):
  bw=headless.BitWriter();bw.bits(1,21)
  for _ in range(8):bw.bit(0)
  w.send_packet(0x158d,struct.pack('<I',typ)+bw.flush()+struct.pack('<I',35626));w.pump(1)
 payloads={struct.unpack_from('<I',b,8)[0]:b[16:] for b in responses if len(b)>=16 and struct.unpack_from('<I',b)[0]==35626}
 assert set(payloads)=={1344507586,2442913102},'Both item cache records required'
 sparse=payloads[2442913102]
 report['quality']=struct.unpack_from('<I',sparse,4)[0];report['sell_price']=struct.unpack_from('<I',sparse,36)[0];report['stackable']=struct.unpack_from('<I',sparse,92)[0]
 assert report['quality']==5 and report['sell_price']==0 and report['stackable']==20,report
 assert b'Adds currency to your account of 100 Battle Shop Coins.' in sparse
 assert d.q('World','SELECT COUNT(*) FROM hotfix_data WHERE entry=35626 AND type IN(1344507586,2442913102)')==[['2']]
 report['description']=True;report['hotfix_invalidation_rows']=2;report['passed']=True;assert w.logout();print('Orb cache: Legendary, unsellable, stack20 and description passed',flush=True)
finally:
 if w:
  try:w.logout()
  except Exception:pass
  w.s.close()
 if aid:report['cleanup']=cleanup_owned(d,pid,account,name,aid,guid)
 report['events']=events;(O/'report.json').write_text(json.dumps(report,indent=2,default=str))
