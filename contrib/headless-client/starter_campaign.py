"""Normal-account starter smoke campaign over SRP/world TCP, with persisted assertions.
No quest completion, teleport, item grant, level or GM commands are used.
World DB is read-only; only uniquely named disposable auth accounts are inserted.
Use --world-pid for native cleanup. Reports contain no account passwords.
"""
import argparse, concurrent.futures, hashlib, json, math, os, re, secrets, subprocess, sys, time, struct
from pathlib import Path
import headless
from qa_spec import load_suite, SUPPORTED

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'contrib/qa-automation'))
from evidence import write_json
CASES=[(1,1),(2,4),(3,3),(4,11),(5,4),(6,1),(7,8),(8,7),(9,4),(10,2),(11,5),(22,4),(24,10),(1,9),(1,6)]
STARTERS={1:28766,2:25152,5:24959,22:14078,24:30039}


def logout_when_ready(client, combat_wait=45):
 deadline=time.monotonic()+combat_wait
 while True:
  if client.logout():return True
  # Retry only the server's explicit combat refusal. Falling/duel/freeze and
  # missing acknowledgements remain failures, rather than being papered over.
  if client.last_logout_result!=1 or time.monotonic()>=deadline:return False
  client.attack_stop();client.pump(min(3,max(0,deadline-time.monotonic())))

class Database:
 def __init__(self,config,client):
  text=config.read_text(encoding='utf-8-sig')
  self.connections={k:re.search(r'^'+k+r'DatabaseInfo\s*=\s*"([^"]+)"',text,re.M)[1].split(';') for k in ('Login','Character','World')}
  self.client=str(client.resolve())
 def q(self,kind,sql):
  host,port,user,password,db=self.connections[kind]
  marker='__CAMPAIGN_QUERY_COMPLETE__'
  for attempt in range(3):
   p=subprocess.run([self.client,'--no-defaults','--protocol=TCP','--host='+host,'--port='+port,'--user='+user,'--database='+db,'--batch','--skip-column-names'],input=sql.rstrip(';')+"; SELECT '"+marker+"';",text=True,capture_output=True,env=dict(os.environ,MYSQL_PWD=password),timeout=45)
   if p.returncode:raise RuntimeError(p.stderr.replace(password,'[redacted]')[:400])
   if p.stdout.rstrip().endswith(marker):
    data=p.stdout.rsplit(marker,1)[0].strip('\r\n')
    return [line.split('\t') for line in data.splitlines()]
   if not sql.lstrip().upper().startswith('SELECT'):
    raise RuntimeError('Incomplete mutation response; inspect account before retrying')
  raise RuntimeError('Incomplete read response after three attempts')

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--config',type=Path,default=ROOT/'exports/worldserver.conf');p.add_argument('--client',type=Path,default=ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe');p.add_argument('--report',type=Path,required=True);p.add_argument('--world-pid',type=int,required=True);p.add_argument('--combat',action='store_true');p.add_argument('--only-human',action='store_true');p.add_argument('--workers',type=int,default=3);p.add_argument('--host',default='127.0.0.1');p.add_argument('--suite',type=Path);a=p.parse_args()
 suite=load_suite(a.suite) if a.suite else None
 cases=suite['scenarios'] if suite else [dict(id=f'legacy-{r}-{c}',race=r,class_id=c,mode='creation_rejected' if c==6 else ('delivery' if (r,c) in [(2,4),(5,4),(22,4)] else 'accept' if (r,c) in [(1,1),(24,10)] else 'create'),quest=STARTERS.get(r) if (r,c) in [(1,1),(2,4),(5,4),(22,4),(24,10)] else None,response=59) for r,c in CASES]
 db=Database(a.config,a.client);headless.log=lambda *args,**kwargs:None
 token=''.join(secrets.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(5))
 def run(index,case):
  if case['mode'] not in SUPPORTED:return dict(id=case['id'],status='untested',reason=case['reason'],checks=[])
  suffix='';n=index
  while True:
   suffix=chr(97+n%26)+suffix;n=n//26-1
   if n<0:break
  race,klass=case['race'],case['class_id'];name=('Qz'+token+suffix).capitalize();account=name.upper();password=secrets.token_hex(8).upper();w=None
  result=dict(id=case['id'],mode=case['mode'],last_step='create_account',name=name,account=account,race=race,class_id=klass,status='failed',checks=[])
  trace=[]
  def mark(step):
   result['last_step']=step;trace.append(dict(step=step,time_utc=__import__('datetime').datetime.now(__import__('datetime').timezone.utc).isoformat()))
   write_json(a.report.parent/('trace-'+str(index)+'.json'),dict(id=case['id'],steps=trace,
    account=account,name=name,account_created=result.get('account_created',False),
    account_id=result.get('account_id'),guid=result.get('guid')))
  mark('create_account')
  try:
   assert not db.q('Login',"SELECT id FROM account WHERE username='"+account+"'")
   sha=hashlib.sha1((account+':'+password).encode()).hexdigest()
   db.q('Login',"INSERT INTO account(username,sha_pass_hash,expansion) VALUES('"+account+"','"+sha+"',4)")
   result['account_created']=True
   result['account_id']=int(db.q('Login',"SELECT id FROM account WHERE username='"+account+"'")[0][0])
   mark('account_created')
   key=headless.auth_login(a.host,3724,account,password);w=headless.WorldClient(a.host,8085,key,account);w.emit=lambda *args,**kwargs:None;w.handshake(key,realm_id=1)
   assert not w.char_enum(),'account is not empty'
   w.char_create(name,race,klass)
   end=time.monotonic()+30
   while time.monotonic()<end:
    op,body=w.read_packet()
    if op==headless.SMSG_CHAR_CREATE:
     if case['mode']=='creation_rejected' and body==bytes([case['response']]):
      result['checks'].append('death_knight_level_requirement_enforced');mark('verified');result['status']='passed';return result
     assert case['mode']!='creation_rejected','creation unexpectedly succeeded'
     assert body==bytes([47]),'creation response '+body.hex();break
   else:raise RuntimeError('creation timeout')
   chars=w.char_enum();char=next(c for c in chars if c['name']==name)
   result['guid']=int(db.q('Character',"SELECT guid FROM characters WHERE name='"+name+"'")[0][0])
   mark('character_created')
   if case['mode']=='gossip_item':
    mark('fixture_setup_before_world_entry')
    fixture_guid=result['guid']
    actors=db.q('World',f"SELECT guid,map,position_x,position_y,position_z FROM creature WHERE id={case['npc']} AND phaseMask=1 AND phaseId=0 ORDER BY guid")
    assert actors,'No phase-one actor for fixture'
    spawn,mapid,x,y,z=actors[0];pos=tuple(map(float,(x,y,z)))
    db.q('Character',f"UPDATE characters SET level={case.get('fixture_level',85)},map={int(mapid)},position_x={pos[0]},position_y={pos[1]},position_z={pos[2]} WHERE guid={fixture_guid}")
    if case.get('seed_quest',True):db.q('Character',f"INSERT INTO character_queststatus (guid,quest,status) VALUES ({fixture_guid},{case['quest']},3)")
    result['fixture']=dict(disposable_only=True,level=case.get('fixture_level',85),position=pos,seeded_incomplete_quest=case['quest'] if case.get('seed_quest',True) else None,setup_is_not_quest_acceptance=True)
   workflow=None
   if case['mode']=='workflow':
    from qa_actions import Workflow
    workflow_guid=result['guid']
    workflow=Workflow(w,db,workflow_guid,case,result,mark)
   assert w.player_login(char['guid']),'world entry failed'
   w.pump(2);result['checks'].append('create_immediate_enum_login');result['map']=w.map_id
   guid=result['guid']
   before=db.q('Character',f'SELECT level,xp,money FROM characters WHERE guid={guid}')[0]
   if workflow:
    result['quest']=case['quest']
    workflow.execute()
    mark('workflow_logout')
    if not logout_when_ready(w):
     result['logout_result']=w.last_logout_result
     raise AssertionError('Workflow logout failed; server result '+str(w.last_logout_result))
    workflow.verify(before)
    assert w.player_login(guid),'Workflow re-entry failed'
    assert w.logout(),'Workflow re-entry logout failed'
    result['checks'].append('logout_character_reentry')
    mark('verified');result['status']='passed';return result
   if case['mode']=='gossip_item':
    mark('gossip_item_request')
    npc=(0xF13<<52)|(case['npc']<<32)|int(spawn)
    w.gossip_hello(npc);w.pump(2)
    assert w.gossip,'No gossip response'
    options=[o for o in w.gossip['options'] if case['option_text'].lower() in str(o).lower()]
    result['matching_options']=len(options)
    if case.get('seed_quest',True):
     assert options,'Expected quest option missing'
     result['gossip_option_evidence']=options
     option=options[0]
     option_id=option['id']
     w.gossip_select(option_id);w.pump(2)
     # A second normal menu selection must not create a duplicate quest item.
     w.gossip_hello(npc);w.pump(1)
     options=[o for o in (w.gossip or {}).get('options',[]) if case['option_text'].lower() in str(o).lower()]
     if options:w.gossip_select(options[0]['id']);w.pump(1)
    else:assert not options,'Quest item option offered without its quest'
    mark('verify_persisted_gossip_item')
    assert w.logout(),'Gossip scenario logout failed'
    count=int(db.q('Character',f"SELECT COALESCE(SUM(count),0) FROM item_instance WHERE owner_guid={guid} AND itemEntry={case['item']}")[0][0])
    assert count==case['expected_count'],f"Expected {case['expected_count']} items, found {count}"
    result['item_count']=count;result['checks']+=['normal_gossip_response','exact_persisted_item_count','duplicate_or_no_quest_guard']
    mark('verified');result['status']='passed';return result
   if case['mode']=='invalid_accept':
    probe=case['request_quest'];result['request_quest']=probe
    assert not db.q('World',f'SELECT ID FROM quest_template WHERE ID={probe}'),'Probe ID exists; refusing invalid-quest test'
    mark('send_invalid_quest_accept')
    w.quest_accept(probe,0);w.pump(2)
    assert w.logout(),'Server did not acknowledge logout after invalid request'
    assert not db.q('Character',f'SELECT quest FROM character_queststatus WHERE guid={guid} AND quest={probe}'),'Invalid quest persisted'
    assert not db.q('Character',f'SELECT quest FROM character_queststatus_rewarded WHERE guid={guid} AND quest={probe}'),'Invalid quest rewarded'
    assert w.player_login(guid),'Character re-entry failed after invalid request'
    assert w.logout(),'Re-entry logout failed'
    result['checks']+=['invalid_accept_ignored','no_invalid_quest_persistence','logout_character_reentry']
    mark('verified');result['status']='passed';return result
   quest=case.get('quest')
   def approach(relation):
    rows=db.q('World',f'SELECT c.guid,c.id,c.position_x,c.position_y,c.position_z FROM creature_{relation} r JOIN creature c ON c.id=r.id WHERE r.quest={quest} AND c.map={int(w.map_id)}')
    assert rows,'no quest actor spawn'
    row=min(rows,key=lambda r:math.dist(tuple(map(float,r[2:])),w.pos));pos=tuple(map(float,row[2:]));assert math.dist(pos,w.pos)<100,'actor too far for bounded smoke route'
    w.move_walk([pos]);w.pump(1)
    result.setdefault('actors',[]).append(dict(relation=relation,entry=int(row[1]),spawn=int(row[0]),position=pos))
    return (0xF13<<52)|(int(row[1])<<32)|int(row[0])
   if quest:
    mark('accept_quest')
    npc=approach('queststarter');w.gossip_hello(npc);w.pump(1);w.quest_accept(quest,npc);w.pump(2)
    result['quest']=quest
    combat_complete=False
    if a.combat and (race,klass)==(1,1):
     credits=[];auto=w.auto
     def observe(op,body):
      if op==0x1645 and len(body)>=13:
       count,kind,qid,required,entry=struct.unpack_from('<HBIHI',body)
       if qid==quest:credits.append(dict(count=count,required=required,entry=entry))
      auto(op,body)
     w.auto=observe
     targets=db.q('World','SELECT guid,id,position_x,position_y,position_z FROM creature WHERE id=49871 AND map=0')
     targets.sort(key=lambda r:math.dist(tuple(map(float,r[2:])),w.pos))
     for row in targets[:14]:
      pos=tuple(map(float,row[2:]))
      if math.dist(pos,w.pos)>150:continue
      w.move_walk([pos]);w.pump(.5)
      target=(0xF13<<52)|(int(row[1])<<32)|int(row[0]);old=len(credits)
      w.attack(target)
      deadline=time.monotonic()+22
      while time.monotonic()<deadline and len(credits)==old:w.pump(.5)
      w.attack_stop()
      if credits and credits[-1]['count']>=credits[-1]['required']:break
     result['combat_credits']=credits
     assert credits and credits[-1]['count']>=credits[-1]['required'],'kill objective not completed within bounded route'
     result['checks'].append('six_real_kills_with_server_credit');combat_complete=True
    if case['mode']=='delivery' or combat_complete:
     mark('turn_in_quest')
     npc=approach('questender');w.gossip_hello(npc);w.pump(1);w.quest_complete(quest,npc);w.pump(1);w.quest_reward(quest,0,npc);w.pump(2)
   mark('logout_and_verify_persistence')
   assert w.logout(),'graceful logout failed'
   time.sleep(1)
   if quest:
    rewarded=db.q('Character',f'SELECT quest FROM character_queststatus_rewarded WHERE guid={guid} AND quest={quest}')
    state=db.q('Character',f'SELECT status FROM character_queststatus WHERE guid={guid} AND quest={quest}')
    if case['mode']=='delivery' or combat_complete:
     assert rewarded,'turn-in not persisted'
     after=db.q('Character',f'SELECT level,xp,money FROM characters WHERE guid={guid}')[0]
     if not case.get('expected_reward'):
      assert int(after[0])>int(before[0]) or int(after[1])>int(before[1]),'XP reward missing'
     for key,expected in case.get('expected_reward',{}).items():
      col=1 if key=='xp_delta' else 2
      assert int(after[col])-int(before[col])==expected, f'{key}: expected {expected}, got {int(after[col])-int(before[col])}'
      result['checks'].append('exact_'+key)
     result['checks']+=['quest_accepted_and_rewarded','xp_reward'];result['reward_before']=before;result['reward_after']=after
    else:
     for attempt in range(5):
      if state and int(state[0][0]) in (1,3):break
      time.sleep(1);state=db.q('Character',f'SELECT status FROM character_queststatus WHERE guid={guid} AND quest={quest}')
     assert state and int(state[0][0]) in (1,3),'quest acceptance not persisted'
     result['checks'].append('quest_acceptance_only')
   if quest:
    mark('character_reentry')
    assert w.player_login(guid),'character re-entry failed'
    assert w.logout(),'re-entry logout failed'
    result['checks'].append('logout_character_reentry')
   mark('verified');result['status']='passed'
  except Exception as e:
   result['error']=str(e).replace(password,'[redacted]')
   if isinstance(e,(ConnectionError,TimeoutError)) or 'bounded smoke route' in str(e):result['status']='blocked'
  finally:
   if w:
    try:w.s.close()
    except OSError:pass
  return result
 a.report.parent.mkdir(parents=True,exist_ok=True)
 with concurrent.futures.ThreadPoolExecutor(max_workers=a.workers) as pool:
  results=list(pool.map(lambda indexed:run(*indexed),enumerate(cases[:1] if a.only_human else cases)))
 report=dict(suite=suite['name'] if suite else 'legacy-starter',oracle='Independent scenario expectations plus persisted server state; world DB used for route discovery only',combat_enabled=a.combat,scope='Only the supplied scenarios and listed assertions are covered. Acceptance-only is not quest completion. No general kill/collect/instance/store coverage claimed.',results=results,cleanup=[])
 a.report.write_text(json.dumps(report,indent=2)+'\n')
 from qa_cleanup import cleanup_owned
 for r in results:
  if not r.get('account_created'):continue
  report['cleanup'].append(cleanup_owned(db,a.world_pid,r['account'],r['name'],r['account_id'],r.get('guid')))
 a.report.write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps(report,indent=2));return int(any(r['status']!='passed' for r in results) or any(not c['account_absent'] or not c['character_absent'] for c in report['cleanup']))
if __name__=='__main__':sys.exit(main())
