"""Read-only timed census of natural Dalaran bot movement using one owned fixture."""
import argparse,hashlib,json,secrets,struct,subprocess,sys,time,re,math
from pathlib import Path
R=Path(__file__).resolve().parents[2];sys.path.insert(0,str(R/'contrib/headless-client'))
import headless
from starter_campaign import Database
from qa_actions import Workflow
from qa_cleanup import cleanup_owned
headless.log=lambda *a,**k:None
args=argparse.ArgumentParser();args.add_argument('--seconds',type=int,default=240);args.add_argument('--label',default='after');opt=args.parse_args()
d=Database(R/'exports/worldserver.conf',R/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe');pid=int(subprocess.check_output(['powershell','-NoProfile','-Command','Get-Process worldserver | Select-Object -ExpandProperty Id'],text=True).strip())
name='Qcity'+''.join(secrets.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(5));account=name.upper();password=secrets.token_hex(10).upper();aid=guid=None;w=None;report=dict(passed=False,fixture_only=True);O=R/'out/city-auction-20261112'/('city-'+opt.label);O.mkdir(parents=True,exist_ok=True);events=[]
try:
 sha=hashlib.sha1((account+':'+password).encode()).hexdigest();d.q('Login',"INSERT INTO account(username,sha_pass_hash,expansion) VALUES('"+account+"','"+sha+"',4)");aid=int(d.q('Login',"SELECT id FROM account WHERE username='"+account+"'")[0][0]);d.q('Login',f'INSERT INTO account_access(id,gmlevel,RealmID) VALUES({aid},3,-1)');(O/'fixture-private.json').write_text(json.dumps(dict(account=account,name=name,account_id=aid)))
 key=headless.auth_login('127.0.0.1',3724,account,password);w=headless.WorldClient('127.0.0.1',8085,key,account);w.handshake(key,realm_id=1);w.emit=lambda event,**data:events.append(dict(event=event,**data));w.char_create(name,1,1)
 end=time.monotonic()+30
 while time.monotonic()<end:
  op,b=w.read_packet()
  if op==headless.SMSG_CHAR_CREATE:assert b==bytes([47]);break
 w.char_enum();guid=int(d.q('Character',"SELECT guid FROM characters WHERE name='"+name+"'")[0][0]);d.q('Character',f'UPDATE characters SET level=90,health=100000,map=1,zone=141,position_x=8355,position_y=991,position_z=21.86 WHERE guid={guid}');report.update(account_id=aid,guid=guid,name=name,account=account);(O/'fixture-private.json').write_text(json.dumps(report));wf=Workflow(w,d,guid,dict(quest=0),report,lambda _:None);assert w.player_login(guid);w.pump(2)
 samples=[];start=time.monotonic()
 while time.monotonic()-start<opt.seconds:
  index=len(events);w.send_chat('say','.botscenario '+name+' citycensus');w.pump(1)
  census=[]
  for e in events[index:]:
   if e.get('event')=='chat' and 'CITY name=' in e.get('msg',''):
    row=dict(re.findall(r'(\w+)=([^\s"]+)',e['msg']));row['t']=round(time.monotonic()-start,2);census.append(row)
  samples.append(census);print('Census',len(samples),'surface',sum(float(x['z'])>630 for x in census),flush=True);w.pump(9)
 tracks={}
 for sample in samples:
  for row in sample:tracks.setdefault(row['guid'],[]).append(row)
 moved=[];stops=[];bad=[]
 for ident,rows in tracks.items():
  surface=[r for r in rows if float(r['z'])>630 and r['group']=='0' and r['master']=='0']
  if len(surface)<2:continue
  origin=surface[0];dist=max(math.dist([float(origin[k]) for k in ('x','y','z')],[float(r[k]) for k in ('x','y','z')]) for r in surface)
  if dist>25:moved.append(ident)
  for r in surface:
   if int(r['npc']) and math.dist([float(r[k]) for k in ('x','y','z')],[float(r[k]) for k in ('nx','ny','nz')])<7:stops.append((ident,r['npc']))
   if float(r['floor'])>-10000 and abs(float(r['z'])-float(r['floor']))>5:bad.append(r)
 report.update(samples=samples,tracked_bots=len(tracks),surface_final=sum(float(x['z'])>630 for x in samples[-1]),bots_moved_25yd=len(moved),bots_reached_service=len({x[0] for x in stops}),distinct_services=len({x[1] for x in stops}),large_floor_offsets=len(bad),floor_offset_examples=bad[:5])
 report['passed']=report['surface_final']>=20 and report['bots_moved_25yd']>=10 and report['bots_reached_service']>=5
 print(json.dumps({k:v for k,v in report.items() if k in ['passed','surface_final','bots_moved_25yd','bots_reached_service','distinct_services','large_floor_offsets']}),flush=True)
 assert report['passed'],'City activity acceptance incomplete'
 assert w.logout()
finally:
 if w:
  try:w.logout()
  except Exception:pass
  w.s.close()
 if aid:report['cleanup']=cleanup_owned(d,pid,account,name,aid,guid)
 report['events']=events;(O/'report-private.json').write_text(json.dumps(report,indent=2,default=str))
