"""Read-only native auction paging/name benchmark using one disposable owned fixture.
Measures protocol time separately from the rendered Auctioneer addon's cache work.
"""
import argparse,hashlib,json,secrets,struct,subprocess,sys,time,statistics
from pathlib import Path
R=Path(__file__).resolve().parents[2];sys.path.insert(0,str(R/'contrib/headless-client'))
import headless
from starter_campaign import Database
from qa_actions import Workflow
from qa_cleanup import cleanup_owned
headless.log=lambda *a,**k:None
p=argparse.ArgumentParser();p.add_argument('--label',default='baseline');p.add_argument('--get-all',action='store_true');a=p.parse_args()
O=R/'out/city-auction-20261112'/a.label;O.mkdir(parents=True,exist_ok=True)
d=Database(R/'exports/worldserver.conf',R/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe');pid=int(subprocess.check_output(['powershell','-NoProfile','-Command','Get-Process worldserver | Select-Object -ExpandProperty Id'],text=True).strip())
name='Qauc'+''.join(secrets.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(5));account=name.upper();password=secrets.token_hex(10).upper();aid=guid=None;w=None;report=dict(passed=False,fixture_only=True,pages=[]);events=[];replies=[];names={}
def query_body(npc,page,getall=False):
 b=npc.to_bytes(8,'little');bits=headless.BitWriter()
 for i in (3,4,5,2):bits.bit(b[i]!=0)
 bits.bit(getall);bits.bit(0)
 for i in (7,0):bits.bit(b[i]!=0)
 bits.bits(0,8)
 for i in (1,6):bits.bit(b[i]!=0)
 return struct.pack('<IIIBBBIII',0xffffffff,page*50,0xffffffff,0,0,0,0xffffffff,0xffffffff,0)+bits.flush()+bytes(b[i]^1 for i in (6,3,4,0,7,2,1,5) if b[i])
def auction_page(body):
 count=struct.unpack_from('<I',body)[0];pos=4;items=[]
 for _ in range(count):
  ident,item=struct.unpack_from('<II',body,pos);pos+=104
  mask=struct.unpack_from('<I',body,pos)[0];pos+=4+mask.bit_count()*4
  pos+=20;owner=struct.unpack_from('<Q',body,pos)[0];pos+=52
  items.append((ident,item,owner))
 assert pos+8==len(body),(count,pos,len(body))
 total,delay=struct.unpack_from('<II',body,pos);return items,total,delay
try:
 sha=hashlib.sha1((account+':'+password).encode()).hexdigest();d.q('Login',"INSERT INTO account(username,sha_pass_hash,expansion) VALUES('"+account+"','"+sha+"',4)");aid=int(d.q('Login',"SELECT id FROM account WHERE username='"+account+"'")[0][0]);d.q('Login',f'INSERT INTO account_access(id,gmlevel,RealmID) VALUES({aid},3,-1)');(O/'fixture-private.json').write_text(json.dumps(dict(account=account,name=name,account_id=aid)))
 key=headless.auth_login('127.0.0.1',3724,account,password);w=headless.WorldClient('127.0.0.1',8085,key,account);w.handshake(key,realm_id=1);w.emit=lambda event,**data:events.append(dict(event=event,**data));w.char_create(name,1,1)
 deadline=time.monotonic()+30
 while time.monotonic()<deadline:
  op,b=w.read_packet()
  if op==headless.SMSG_CHAR_CREATE:assert b==bytes([47]);break
 w.char_enum();guid=int(d.q('Character',"SELECT guid FROM characters WHERE name='"+name+"'")[0][0]);d.q('Character',f'UPDATE characters SET level=90,health=100000,map=0,zone=1519,position_x=-8811.74,position_y=658.227,position_z=99.616 WHERE guid={guid}');report.update(account_id=aid,guid=guid,name=name,account=account);(O/'fixture-private.json').write_text(json.dumps(report));wf=Workflow(w,d,guid,dict(quest=0),report,lambda _:None);assert w.player_login(guid);w.pump(2);w.send_chat('say','.gm on');w.pump(1)
 actors=wf.tracker.actors(15659);assert actors,'Auctioneer not visible';npc=actors[0][0];read=w.read_packet
 def capture():
  op,b=read()
  if op==0x0982:replies.append((time.monotonic(),b))
  if op==0x169b:
   ident,text=headless.mv_read_name_response(b);names[ident]=(text,time.monotonic())
  return op,b
 w.read_packet=capture;owners=set();auctions=set();page=0;start=time.monotonic()
 while True:
  before=len(replies);sent=time.monotonic();w.send_packet(0x02ea,query_body(npc,page,a.get_all))
  deadline=time.monotonic()+20
  while len(replies)==before and time.monotonic()<deadline:w.pump(.02)
  assert len(replies)>before,'Auction page timed out'
  received,body=replies[-1];items,total,delay=auction_page(body);owners.update(x[2] for x in items if x[2]);auctions.update(x[0] for x in items)
  report['pages'].append(dict(page=page,count=len(items),total=total,wire_delay_ms=delay,reply_ms=1000*(received-sent),bytes=len(body)))
  if a.get_all or (page+1)*50>=total:break
  w.pump(delay/1000.);page+=1
 report['paged_seconds']=time.monotonic()-start;report['auction_ids']=len(auctions);report['owners']=len(owners)
 report['prefetched_owners']=sum(x in names and bool(names[x][0]) for x in owners)
 report['prefetched_seller_coverage']=report['prefetched_owners']/len(owners) if owners else 0
 names.clear()
 # Cold protocol name queries, no client-side name cache. Bounded bursts below default AntiDOS.
 requested={};start=time.monotonic()
 for i,owner in enumerate(sorted(owners)):
  requested[owner]=time.monotonic();w.send_packet(0x0328,headless.mv_name_query_body(owner))
  if i%25==24:w.pump(.5)
 deadline=time.monotonic()+20
 while not owners<=names.keys() and time.monotonic()<deadline:w.pump(.1)
 report['names_seconds']=time.monotonic()-start;report['missing_names']=len([x for x in owners if x not in names or not names[x][0]]);report['name_reply_ms_p95']=statistics.quantiles([1000*(names[x][1]-requested[x]) for x in owners if x in names],n=20)[18] if len(names)>1 else None
 report['passed']=not report['missing_names'] and len(auctions)>0;assert report['passed'];print(json.dumps({k:v for k,v in report.items() if k in ['passed','paged_seconds','auction_ids','owners','names_seconds','missing_names','name_reply_ms_p95']}),flush=True);w.logout()
finally:
 if w:
  try:w.logout()
  except Exception:pass
  w.s.close()
 if aid:report['cleanup']=cleanup_owned(d,pid,account,name,aid,guid)
 (O/'report-private.json').write_text(json.dumps(report,indent=2,default=str))
