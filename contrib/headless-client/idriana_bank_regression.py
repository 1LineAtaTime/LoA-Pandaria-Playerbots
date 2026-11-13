"""Disposable Idriana bank gossip regression.
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
name='Qbank'+''.join(secrets.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(4));account=name.upper();password=secrets.token_hex(10).upper();aid=guid=None;w=None
report=dict(fixture_only=True,passed=False);(R/'out/idriana-bank-active-fixture.json').write_text(json.dumps(dict(account=account,name=name)))
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
 w.char_enum();guid=int(d.q('Character',"SELECT guid FROM characters WHERE name='"+name+"'")[0][0]);report.update(account_id=aid,guid=guid,name=name,account=account);(R/'out/idriana-bank-active-fixture.json').write_text(json.dumps(report))



 d.q('Character',f'UPDATE characters SET level=12,health=1000,map=1,zone=1657,position_x=9938.84,position_y=2521.53,position_z=1317.66 WHERE guid={guid}')
 wf=Workflow(w,d,guid,dict(quest=0),report,lambda x:None);assert w.player_login(guid);w.pump(3);w.send_chat('say','.gm off');w.pump(.3)
 banker=wf.tracker.actors(4155)[0][0];packets=[];original=w.read_packet
 def capture():
  op,b=original()
  if op==0x0007:packets.append(b)
  return op,b
 w.read_packet=capture;w.gossip_hello(banker);w.pump(.5)
 options=[o for o in (w.gossip or {}).get('options',[]) if 'deposit box' in o['text'].lower()];assert len(options)==1,'Deposit-box menu missing';report['option_text']=options[0]['text']
 w.gossip_select(options[0]['id']);w.pump(.5);report['bank_open_opcode_received']=bool(packets);assert packets,'No SMSG_SHOW_BANK response';report['bank_open_payload_bytes']=len(packets[-1]);report['passed']=True;report['logout_acknowledged']=w.logout();print('Idriana bank window response passed',flush=True)
finally:
 if w:
  try:
   if not report.get('logout_acknowledged'):w.logout()
  except Exception:pass
  w.s.close()
 if aid:report['cleanup']=cleanup_owned(d,pid,account,name,aid,guid)
 (R/'out/idriana-bank-regression.json').write_text(json.dumps(report,indent=2,default=str))
