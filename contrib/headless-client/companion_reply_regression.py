"""Real-roster companion command replies and quiet quest-share regression."""
import sys,time,secrets,hashlib,json,subprocess
from pathlib import Path
R=Path(__file__).resolve().parents[2];sys.path.insert(0,str(R/'contrib/headless-client'))
import headless
from starter_campaign import Database
from qa_cleanup import cleanup_owned
headless.log=lambda *a,**k:None
d=Database(R/'exports/worldserver.conf',R/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
pid=int(subprocess.check_output(['powershell','-NoProfile','-Command','Get-Process worldserver | Select-Object -ExpandProperty Id'],text=True).strip())
name='Qreply'+''.join(secrets.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(4));account=name.upper();password=secrets.token_hex(10).upper();aid=guid=None;w=None
O=R/'out/companion-reply-20261112';O.mkdir(parents=True,exist_ok=True);report=dict(passed=False,fixture_only=True,checks={});messages=[]
def wait_until(predicate,seconds):
 end=time.monotonic()+seconds
 while time.monotonic()<end:
  w.pump(.5)
  if predicate():return True
 return False
try:
 sha=hashlib.sha1((account+':'+password).encode()).hexdigest();d.q('Login',"INSERT INTO account(username,sha_pass_hash,expansion) VALUES('"+account+"','"+sha+"',4)");aid=int(d.q('Login',"SELECT id FROM account WHERE username='"+account+"'")[0][0]);d.q('Login',f'INSERT INTO account_access(id,gmlevel,RealmID) VALUES({aid},3,-1)')
 (O/'active-fixture.json').write_text(json.dumps(dict(account=account,name=name,account_id=aid)))
 key=headless.auth_login('127.0.0.1',3724,account,password);w=headless.WorldClient('127.0.0.1',8085,key,account);w.handshake(key,realm_id=1)
 def emit(event,**data):
  if event=='chat':messages.append(data)
 w.emit=emit;w.char_create(name,1,1)
 end=time.monotonic()+30
 while time.monotonic()<end:
  op,b=w.read_packet()
  if op==headless.SMSG_CHAR_CREATE:assert b==bytes([47]);break
 w.char_enum();guid=int(d.q('Character',"SELECT guid FROM characters WHERE name='"+name+"'")[0][0]);d.q('Character',f'UPDATE characters SET level=90,health=100000 WHERE guid={guid}')
 report.update(account_id=aid,guid=guid,name=name,account=account);(O/'active-fixture.json').write_text(json.dumps(report));assert w.player_login(guid);w.pump(3)
 w.send_chat('say','.bot group addRoleBotsToGroup 1 1');w.pump(8);w.send_chat('say','.bot group addRoleBotsToGroup 2 3');assert wait_until(lambda:len(w.group_members)>=4,75),'Four companions did not join'
 report['checks']['four_companions_joined']=True;report['wire_leader']=w.leader_guid;report['group_rows']=d.q('Character',f'SELECT g.guid,g.leaderGuid FROM groups g JOIN group_member m ON m.guid=g.guid WHERE m.memberGuid={guid}');report['checks']['human_leader']=report['group_rows'] and int(report['group_rows'][0][1])==guid;assert report['checks']['human_leader']
 members=[(n,g) for n,g in w.group_members.items() if (g&0xffffffff)!=guid];assert members
 target=members[0][0];messages.clear()
 w.send_chat('whisper','position stay ?',target=target);w.pump(12)
 report['explicit_reply_messages']=messages.copy()
 replies=[m for m in messages if 'Position stay' in m.get('msg','')]
 report['checks']['explicit_position_reply']=bool(replies);assert replies,'Explicit queued position reply absent'
 messages.clear();w.send_chat('say','.quest add 783');w.pump(1);w.quest_share(783);w.pump(5)
 assert not [m for m in messages if m.get('type')=='WHISPER'],'Routine quest whisper escaped quiet policy'
 report['checks']['routine_share_still_quiet']=True
 report['passed']=True;print('Companion explicit reply and quiet share passed',flush=True)
finally:
 if w:
  try:
   owned_companions=list(w.group_members.items());w.send_chat('say','.group disband '+name);w.pump(2)
   for n,g in owned_companions:
    if (g&0xffffffff)!=guid:w.send_chat('say','.npcbot remove '+n);w.pump(.3)
   w.logout()
  except Exception:pass
  w.s.close()
 if aid:report['cleanup']=cleanup_owned(d,pid,account,name,aid,guid)
 report['messages']=messages;(O/'report.json').write_text(json.dumps(report,indent=2,default=str))
