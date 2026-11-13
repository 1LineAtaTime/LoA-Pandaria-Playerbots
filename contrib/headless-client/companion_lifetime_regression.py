"""Disposable companion login/logout and subsequent real-player relogin regression.
Creates two owned identities; cycles one companion through native add/remove.
This is lifecycle coverage, not proof that every source of memory corruption is fixed.
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
name='Qlife'+''.join(secrets.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(4));account=name.upper();password=secrets.token_hex(10).upper();aid=guid=None;w=None
report=dict(fixture_only=True,passed=False);(R/'out/companion-lifetime-fixture.json').write_text(json.dumps(dict(account=account,name=name)))
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
 w.char_enum();guid=int(d.q('Character',"SELECT guid FROM characters WHERE name='"+name+"'")[0][0]);report.update(account_id=aid,guid=guid,name=name,account=account);(R/'out/companion-lifetime-fixture.json').write_text(json.dumps(report))


 other='Qalt'+''.join(secrets.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(5));otherguid=None
 w.char_create(other,1,5)
 end=time.monotonic()+30
 while time.monotonic()<end:
  op,b=w.read_packet()
  if op==headless.SMSG_CHAR_CREATE:assert b==bytes([47]);break
 w.char_enum();otherguid=int(d.q('Character',"SELECT guid FROM characters WHERE name='"+other+"' AND account="+str(aid))[0][0]);report['companion']=dict(name=other,guid=otherguid)
 (R/'out/companion-lifetime-fixture.json').write_text(json.dumps(report))
 d.q('Character',f'UPDATE characters SET level=90,health=1000,map=1,zone=141,position_x=8355,position_y=991,position_z=21.86 WHERE guid IN({guid},{otherguid})')
 assert w.player_login(guid);w.pump(3);w.send_chat('say','.gm on');w.pump(1)
 report['cycles']=[]
 for i in range(5):
  w.send_chat('say','.npcbot add '+other)
  end=time.monotonic()+30
  while time.monotonic()<end:
   w.pump(.5)
   if d.q('Character',f'SELECT online FROM characters WHERE guid={otherguid}')==[['1']]:break
  assert d.q('Character',f'SELECT online FROM characters WHERE guid={otherguid}')==[['1']],'Companion add failed'
  w.pump(3);w.send_chat('say','.npcbot remove '+other)
  end=time.monotonic()+30
  while time.monotonic()<end:
   w.pump(.5)
   if d.q('Character',f'SELECT online FROM characters WHERE guid={otherguid}')==[['0']]:break
  assert d.q('Character',f'SELECT online FROM characters WHERE guid={otherguid}')==[['0']],'Companion logout failed'
  report['cycles'].append(dict(cycle=i+1,login=True,logout=True));print('Companion lifecycle',i+1,flush=True)
 assert w.logout();w.char_enum();assert w.player_login(otherguid);w.pump(5);report['normal_player_relogin']=True
 # Deliberately replay already-consumed/unsolicited replies; empty-queue pops used to corrupt logout.
 for counter in [0,0,0xdeadbeef,0xffffffff]:w.send_packet(headless.CMSG_TIME_SYNC_RESPONSE,struct.pack('<II',counter,12345))
 w.pump(1);report['duplicate_time_sync_responses_sent']=4;report['logout_acknowledged']=w.logout();assert report['logout_acknowledged'];report['passed']=True
finally:
 if w:
  try:
   if not report.get('logout_acknowledged'):w.logout()
  except Exception:pass
  w.s.close()
 if aid:
  # Erase the second owned character before the single-character cleanup helper.
  if 'otherguid' in locals() and otherguid:
   for _ in range(90):
    if d.q('Character',f'SELECT online FROM characters WHERE guid={otherguid}')==[['0']]:break
    time.sleep(1)
   assert d.q('Character',f'SELECT online FROM characters WHERE guid={otherguid}')==[['0']]
   subprocess.run([sys.executable,str(R/'contrib/windows/Send-ServerConsoleCommand.py'),str(pid),'character erase '+other],check=True)
   for _ in range(30):
    if not d.q('Character',f'SELECT guid FROM characters WHERE guid={otherguid}'):break
    time.sleep(.5)
   report['companion_absent']=not d.q('Character',f'SELECT guid FROM characters WHERE guid={otherguid}')
  report['cleanup']=cleanup_owned(d,pid,account,name,aid,guid)
 (R/'out/companion-lifetime-regression.json').write_text(json.dumps(report,indent=2,default=str))
