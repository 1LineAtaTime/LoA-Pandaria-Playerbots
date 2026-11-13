"""Disposable legacy reward spell and optional learning payload regression."""
import hashlib,json,secrets,struct,subprocess,sys,time
from pathlib import Path
R=Path(__file__).resolve().parents[2];sys.path.insert(0,str(R/'contrib/headless-client'))
import headless
from starter_campaign import Database
from qa_actions import Workflow
from qa_cleanup import cleanup_owned
headless.log=lambda *a,**k:None
d=Database(R/'exports/worldserver.conf',R/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe');pid=int(subprocess.check_output(['powershell','-NoProfile','-Command','Get-Process worldserver | Select-Object -ExpandProperty Id'],text=True).strip())
name='Qspell'+''.join(secrets.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(5));account=name.upper();password=secrets.token_hex(10).upper();aid=guid=None;w=None;report=dict(passed=False,fixture_only=True);O=R/'out/log-noise-20261112';events=[]
try:
 sha=hashlib.sha1((account+':'+password).encode()).hexdigest();d.q('Login',"INSERT INTO account(username,sha_pass_hash,expansion) VALUES('"+account+"','"+sha+"',4)");aid=int(d.q('Login',"SELECT id FROM account WHERE username='"+account+"'")[0][0]);d.q('Login',f'INSERT INTO account_access(id,gmlevel,RealmID) VALUES({aid},3,-1)');(O/'spell-fixture.json').write_text(json.dumps(dict(account=account,name=name,account_id=aid)))
 key=headless.auth_login('127.0.0.1',3724,account,password);w=headless.WorldClient('127.0.0.1',8085,key,account);w.handshake(key,realm_id=1);w.emit=lambda event,**data:events.append(dict(event=event,**data));w.char_create(name,1,1)
 end=time.monotonic()+30
 while time.monotonic()<end:
  op,b=w.read_packet()
  if op==headless.SMSG_CHAR_CREATE:assert b==bytes([47]);break
 w.char_enum();guid=int(d.q('Character',"SELECT guid FROM characters WHERE name='"+name+"'")[0][0]);d.q('Character',f'UPDATE characters SET level=90,health=100000,map=1,zone=141,position_x=8355,position_y=991,position_z=21.86 WHERE guid={guid}');report.update(account_id=aid,guid=guid,name=name,account=account);(O/'spell-fixture.json').write_text(json.dumps(report));wf=Workflow(w,d,guid,dict(quest=0),report,lambda _:None);assert w.player_login(guid);w.pump(2)
 w.send_chat('say','.gm on');w.pump(1)
 w.send_chat('say','.botscenario '+name+' unlearn 2458');w.pump(1)
 w.send_chat('say','.botscenario '+name+' hasspell 2458');w.pump(1)
 report['native_before']=[e.get('msg') for e in events if e.get('event')=='chat' and 'hasspell 2458' in e.get('msg','')]
 assert report['native_before'] and 'spell=0' in report['native_before'][-1]
 events.clear()
 start=(R/'exports/logs/Errors.txt').stat().st_size
 w.send_chat('say','.botscenario '+name+' castself 8616');w.pump(3)
 # Direct cast exercises a generic learning wrapper with its absent payload.
 w.send_chat('say','.botscenario '+name+' castself 483');w.pump(3)
 w.send_chat('say','.botscenario '+name+' hasspell 2458');w.pump(1)
 report['native_spell_reply']=[e.get('msg') for e in events if e.get('event')=='chat' and 'hasspell 2458' in e.get('msg','')]
 assert any('spell=1' in m for m in report['native_spell_reply']),report['native_spell_reply']
 assert w.logout();w.pump(1)
 spells=d.q('Character',f'SELECT spell FROM character_spell WHERE guid={guid} AND spell IN(0,20252)')
 report['invalid_saved_spells']=spells;assert not spells
 errors=(R/'exports/logs/Errors.txt').read_bytes()[start:].decode('utf8',errors='replace');report['new_invalid_spell_errors']=[line for line in errors.splitlines() if 'Non-existed in SpellStore' in line];assert not report['new_invalid_spell_errors']
 report['passed']=True;print('Legacy reward teaches2458, not20252; empty payload adds no spell/error',flush=True)
finally:
 if w:
  try:w.logout()
  except Exception:pass
  w.s.close()
 if aid:report['cleanup']=cleanup_owned(d,pid,account,name,aid,guid)
 report['events']=events;(O/'spell-regression.json').write_text(json.dumps(report,indent=2,default=str))
