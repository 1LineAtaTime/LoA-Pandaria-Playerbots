"""Disposable real-protocol companion party: quiet quest share, loot control and LFG removal.
Uses the existing realm. Does not modify user accounts or complete user quests.
"""
import sys,time,secrets,hashlib,json,subprocess
from pathlib import Path
R=Path(__file__).resolve().parents[2];sys.path.insert(0,str(R/'contrib/headless-client'))
import headless
from starter_campaign import Database
from qa_cleanup import cleanup_owned
headless.log=lambda *a,**k:None
d=Database(R/'exports/worldserver.conf',R/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
pid=int(subprocess.check_output(['powershell','-NoProfile','-Command','Get-Process worldserver | Select-Object -ExpandProperty Id'],text=True).strip())
name='Qparty'+''.join(secrets.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(4));account=name.upper();password=secrets.token_hex(10).upper();aid=guid=None;w=None
O=R/'out/companion-party-20261112';O.mkdir(parents=True,exist_ok=True);report=dict(passed=False,fixture_only=True,checks={});messages=[]
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
 w.send_chat('say','.quest add 783');w.pump(1);w.quest_share(783);w.pump(5)
 whispers=[m for m in messages if m.get('type') in ('WHISPER','WHISPER_INFORM')];report['routine_whispers']=whispers;report['checks']['quiet_invite_and_quest_share']=not whispers;assert not whispers
 w.send_chat('say','.partyloot free');w.pump(1);report['checks']['leader_loot_command']=any('Party loot method changed' in m.get('msg','') for m in messages);assert report['checks']['leader_loot_command']
 # Native fixture equipment supplies the real item-level gate, without lowering realm requirements.
 w.send_chat('say','.botscenario '+name+' equip epic nocap');w.pump(8);w.lfg_join(631,roles=2);w.lfg_set_roles(2)
 accepted=set();end=time.monotonic()+90
 while time.monotonic()<end:
  w.pump(.5)
  if w.lfg_proposal_id and w.lfg_proposal_id not in accepted:w.lfg_proposal_result(w.lfg_proposal_id);accepted.add(w.lfg_proposal_id)
  group=d.q('Character',f'SELECT g.guid,g.groupType FROM groups g JOIN group_member m ON m.guid=g.guid WHERE m.memberGuid={guid} AND (g.groupType&8)<>0')
  if group and int(group[0][1])&8:break
 else:raise AssertionError('Real LFG queue did not form a dungeon party')
 report['checks']['real_lfg_formed']=True;report['checks']['lfg_human_leader']=d.q('Character',f'SELECT g.leaderGuid FROM groups g JOIN group_member m ON m.guid=g.guid WHERE m.memberGuid={guid} AND (g.groupType&8)<>0')==[[str(guid)]];assert report['checks']['lfg_human_leader']
 members=[(n,g) for n,g in w.group_members.items() if (g&0xffffffff)!=guid];assert len(members)>=4
 victim_name,victim=members[0];w.group_uninvite(victim,'Owned QA companion removal');assert wait_until(lambda:victim not in w.group_members.values(),15),'Leader could not remove a companion';report['checks']['lfg_bot_removed']=True
 if len(members)>1:
  offline_name,offline=members[1];subprocess.run([sys.executable,str(R/'contrib/windows/Send-ServerConsoleCommand.py'),str(pid),'kick '+offline_name],check=True);wait_until(lambda:d.q('Character',f'SELECT online FROM characters WHERE guid={offline&0xffffffff}')==[['0']],20)
  if offline in w.group_members.values():
   w.group_uninvite(offline,'Owned QA offline companion removal');assert wait_until(lambda:offline not in w.group_members.values(),15),'Offline companion not removable';report['checks']['offline_bot_removed']=True
  else:report['checks']['offline_bot_removed']='native logout already removed member'
 report['passed']=True;print('Companion quiet/loot/LFG removal checks passed',flush=True)
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
