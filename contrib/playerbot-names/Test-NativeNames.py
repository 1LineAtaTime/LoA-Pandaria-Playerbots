"""Real character-creation/name-claim concurrency and race/class/gender coverage.
Uses one11-character fixture account and two racing one-character accounts.
No existing player account is changed. All fixtures are journaled and removed.
"""
import sys,time,secrets,hashlib,json,subprocess,concurrent.futures,threading
from pathlib import Path
R=Path(__file__).resolve().parents[2];sys.path.insert(0,str(R/'contrib/headless-client'))
import headless
from starter_campaign import Database
from qa_cleanup import cleanup_owned
headless.log=lambda *a,**k:None
d=Database(R/'exports/worldserver.conf',R/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe');O=R/'out/playerbot-naming-20261112';pid=int(subprocess.check_output(['powershell','-NoProfile','-Command',"Get-CimInstance Win32_Process | Where-Object Name -eq 'worldserver.exe' | Select-Object -ExpandProperty ProcessId"],text=True).strip());fixtures=[];report={'passed':False,'matrix':[]}
free=[x[0] for x in d.q('Character',"SELECT n.name FROM playerbot_curated_names n WHERE n.enabled=1 AND n.owner_guid=0 AND NOT EXISTS(SELECT 1 FROM characters c WHERE c.name=n.name) ORDER BY n.name LIMIT 16")];assert len(free)==16

def connect(f):
 key=headless.auth_login('127.0.0.1',3724,f['account'],f['password']);w=headless.WorldClient('127.0.0.1',8085,key,f['account']);w.emit=lambda *a,**k:None;w.handshake(key,realm_id=1);return w

def fixture():
 name='Qnames'+''.join(secrets.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(4));account=name.upper();password=secrets.token_hex(10).upper();sha=hashlib.sha1((account+':'+password).encode()).hexdigest();d.q('Login',"INSERT INTO account(username,sha_pass_hash,expansion) VALUES('"+account+"','"+sha+"',4)");aid=int(d.q('Login',"SELECT id FROM account WHERE username='"+account+"'")[0][0]);d.q('Login',f'INSERT INTO account_access(id,gmlevel,RealmID) VALUES({aid},3,-1)');f=dict(account=account,password=password,aid=aid,cleanup_name=name);fixtures.append(f);(O/'native-name-test-owners.json').write_text(json.dumps([{k:v for k,v in x.items() if k!='password'} for x in fixtures]));return f

def create(w,name,race,cls,gender):
 w.char_create(name,race,cls,gender=gender);end=time.monotonic()+30
 while time.monotonic()<end:
  op,b=w.read_packet()
  if op==headless.SMSG_CHAR_CREATE:return b[0]
 raise RuntimeError('No creation response')

try:
 f=fixture();w=connect(f)
 cases=[(1,1,0),(2,4,1),(3,3,0),(4,11,1),(5,8,0),(6,7,1),(7,9,0),(8,5,1),(10,2,0),(11,6,1),(24,10,0)]
 for name,(race,cls,gender) in zip(free,cases):
  result=create(w,name,race,cls,gender);assert result==47,(race,cls,result)
  row=d.q('Character',f"SELECT guid,name,race,class,gender FROM characters WHERE account={f['aid']} AND name='{name}'")[0];assert row[1:]==[name,str(race),str(cls),str(gender)];report['matrix'].append(dict(race=race,cls=cls,gender=gender,created=True))
  # Give only the fixture a qualifying existing character for the normal DK gate.
  if len(report['matrix'])==1:d.q('Character',f"UPDATE characters SET level=55 WHERE guid={row[0]}")
 assert len(w.char_enum())==11;first=d.q('Character',f"SELECT guid,name FROM characters WHERE account={f['aid']} ORDER BY guid")[0];assert w.player_login(int(first[0]));w.pump(1);assert w.logout();w.s.close();w=connect(f);assert len(w.char_enum())==11;w.s.close();report['relogin_names_stable']=True
 f1=fixture();f2=fixture();barrier=threading.Barrier(2);race_name=free[11]
 def race_worker(f):
  w=connect(f)
  try:
   barrier.wait(timeout=15);result=create(w,race_name,1,4,0);w.pump(.5);return result
  finally:w.s.close()
 with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:results=list(pool.map(race_worker,[f1,f2]))
 report['racing_creation_responses']=results;assert results.count(47)==1,results
 rows=d.q('Character',f"SELECT guid,account FROM characters WHERE name='{race_name}'");assert len(rows)==1;claim=d.q('Character',f"SELECT owner_guid,owner_account FROM playerbot_curated_names WHERE name='{race_name}'");assert claim[0]==rows[0];report['two_workers_single_claim']=True;report['checks_passed']=True;print('Native naming checks passed; cleanup and survival check follow',flush=True)
finally:
 cleanup=[]
 for f in fixtures:
  rows=d.q('Character',f"SELECT guid,name,online FROM characters WHERE account={f['aid']}")
  for guid,name,online in rows:
   for _ in range(90):
    if d.q('Character',f'SELECT online FROM characters WHERE guid={guid}')==[['0']]:break
    time.sleep(1)
   assert d.q('Character',f'SELECT online FROM characters WHERE guid={guid}')==[['0']]
   subprocess.run([sys.executable,str(R/'contrib/windows/Send-ServerConsoleCommand.py'),str(pid),'character erase '+name],check=True)
   for _ in range(40):
    if not d.q('Character',f'SELECT guid FROM characters WHERE guid={guid}'):break
    time.sleep(.25)
   assert not d.q('Character',f'SELECT guid FROM characters WHERE guid={guid}')
  cleanup.append(cleanup_owned(d,pid,f['account'],f['cleanup_name'],f['aid']))
  d.q('Character',f"UPDATE playerbot_curated_names SET owner_guid=0,owner_account=0 WHERE owner_account={f['aid']} AND NOT EXISTS(SELECT 1 FROM characters c WHERE c.guid=playerbot_curated_names.owner_guid)")
 report['cleanup']=cleanup;report['passed']=report.get('checks_passed',False) and all(x['account_absent'] for x in cleanup);(O/'native-name-api-tests.json').write_text(json.dumps(report,indent=2))
