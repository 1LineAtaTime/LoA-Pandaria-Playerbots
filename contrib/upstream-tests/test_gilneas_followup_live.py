"""Controlled normal-security Worgen fixtures: Cynthia rescue and the actual Manor vehicle route."""
import sys,json,time,argparse,hashlib,subprocess,struct
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
sys.path[:0]=[str(ROOT/'contrib/quest-qa'),str(ROOT/'contrib/headless-client'),str(Path(__file__).parent)]
from pool import Pool
from qa_actions import ObservedWorld
from test_gilneas_cage_live import phase_packet,teleport_packet,journal_packet
import headless

def main():
 p=argparse.ArgumentParser();p.add_argument('--out',type=Path,required=True);p.add_argument('--case',choices=('cynthia','ashley','james','manor','breath','gate'),required=True);args=p.parse_args();args.out.parent.mkdir(parents=True,exist_ok=True)
 pool=Pool();c=pool.ensure(22,1 if args.case=='gate' else 4);account=next(a for a in pool.state['accounts'] if a['id']==c['account_id']);lease=pool.lease(account);w=None
 report=dict(case=args.case,execution_profile='controlled-prerequisite-fixture',gm=False,quest_credit_injected=False,passed=False,phases=[],journals=[],mirror_timers=[])
 try:
  pool.reset(c);guid=c['guid'];rewards=[14078,14159,14293,14221,14222,14375,14367]
  if args.case=='gate':
   quest=14386;actor=pool.db.q('World','SELECT guid,id,position_x,position_y,position_z FROM creature WHERE id=36290 AND map=654 AND phaseMask&4096 LIMIT 1')[0]
  elif args.case in ('cynthia','ashley','james'):
   child={'cynthia':36287,'ashley':36288,'james':36289}[args.case]
   quest=14368;actor=pool.db.q('World',f'SELECT guid,id,position_x,position_y,position_z FROM creature WHERE id={child} AND map=654 AND phaseMask=4096 LIMIT 1')[0]
  else:
   quest=14465;actor=pool.db.q('World','SELECT guid,id,position_x,position_y,position_z FROM creature WHERE id=36452 AND map=654 LIMIT 1')[0];rewards += [14368,14369,14386,14396,14395,14397,14402,14405,14463]
  spawn,entry,x,y,z=actor
  if args.case=='breath':x,y,z=-1987.47,2508.71,-5
  report['fixture_level']=20 if args.case in ('cynthia','ashley','james','breath') else 8
  pool.db.q('Character',f"UPDATE characters SET level={report['fixture_level']},map=654,zone=4714,position_x={float(x)+1},position_y={y},position_z={z},health=200 WHERE guid={guid}")
  pool.db.q('Character','INSERT INTO character_queststatus_rewarded(guid,quest,active) VALUES '+','.join(f'({guid},{q},1)' for q in rewards))
  if args.case not in ('breath','gate'):pool.db.q('Character',f'INSERT INTO character_queststatus(guid,quest,status) VALUES({guid},{quest},3)')
  w=pool.connect(account);w.char_enum();tracker=ObservedWorld(w);prior=w.auto
  def observe(op,body):
   prior(op,body)
   if op==headless.SMSG_MOVE_TELEPORT:w.pos,w.orientation=teleport_packet(body)
   if op==0x02A2:report['phases'].append(phase_packet(body))
   if op==0x1542:report['journals'].append(journal_packet(body))
   if op==0x1A0F:report['lock_acquired']=len(body)==0
   if op==headless.SMSG_PONG:report['pong']=True
   if op==headless.SMSG_LOGOUT_RESPONSE:report.setdefault('logout_reasons',[]).append(struct.unpack_from('<I',body)[0])
   if op==0x0E12:report['mirror_timers'].append(body.hex())
   if op==0x1026:report['mirror_stopped']=True
   try:tracker.handle(op,body,time.time())
   except (ValueError,KeyError,IndexError,EOFError):tracker.decode_errors+=1
  w.auto=observe;assert w.player_login(guid);w.pump(3)
  w.move_goto(w.pos[0]+0.1,w.pos[1],w.pos[2]);w.pump(.5) # stationary client heartbeat clears a login's pending fall state
  report['terrain']=report['phases'][-1]['terrains'];assert report['terrain']==([638] if args.case in ('cynthia','ashley','james','gate') else [655])
  raw=(0xF13<<52)|(int(entry)<<32)|int(spawn)
  if args.case=='gate':
   assert raw in tracker.visible,'Godfrey not visible'
   w.quest_hello(raw);w.pump(.5);w.quest_accept(14369,raw);w.pump(1)
   assert 14369 in tracker.quest_slots.values(),'Positive control quest14369 not accepted'
   w.quest_accept(14386,raw);w.pump(1)
   assert 14386 not in tracker.quest_slots.values(),'Leader of the Pack skipped the children prerequisite'
   report['eligible_quest_accepted']=14369;report['premature_quest_rejected']=14386
   assert w.logout();w.s.close();w=None
  elif args.case in ('cynthia','ashley','james'):
   assert raw in tracker.visible,f'{args.case} not visible';w.spellclick(raw);w.pump(3)
   w.send_packet(0x0A23,b'');w.send_packet(0x05E1,b'');w.pump(2)
   w.ping();w.pump(1);assert report.get('pong'),'Connection lost after journal requests'
   report['graceful_logout']=w.logout();w.s.close();w=None
   deadline=time.monotonic()+95
   while pool.db.q('Character',f'SELECT online FROM characters WHERE guid={guid}')[0][0]!='0':
    assert time.monotonic()<deadline,'QA disconnect linger did not finish';time.sleep(1)
   columns=[r[0] for r in pool.db.q('Character',"SELECT COLUMN_NAME FROM information_schema.columns WHERE table_schema=DATABASE() AND table_name='character_queststatus_objective'")]
   deadline=time.monotonic()+10
   while True:
    rows=pool.db.q('Character',f'SELECT * FROM character_queststatus_objective WHERE guid={guid}')
    if rows or time.monotonic()>deadline:break
    time.sleep(.1)
   report['objective_columns']=columns;report['objective_rows']=rows
   objectives=pool.db.q('World',f'SELECT id FROM quest_objective WHERE questId=14368 AND objectId={child}');assert len(objectives)==1
   expected=int(objectives[0][0]);data=[dict(zip(columns,row)) for row in rows]
   report['rescued_child']=child;report['child_objective']=expected
   assert any(int(r.get('objective',r.get('objectiveId',r.get('id',-1))))==expected and int(r.get('counter',r.get('data',r.get('amount',0))))>=1 for r in data),f'{args.case} rescue counter not persisted'
   assert report['journals'],'Journal/lock request pair disconnected'
  elif args.case=='manor':
   assert raw in tracker.visible,'Gwen not visible';w.gossip_hello(raw);w.pump(1)
   options=[o for o in (w.gossip or {}).get('options',[]) if 'ride to greymane manor' in o['text'].lower()];assert len(options)==1,'Ride retry option missing'
   w.gossip_select(options[0]['id']);deadline=time.monotonic()+150;heights=[]
   while time.monotonic()<deadline:
    w.pump(2);horses=tracker.actors(38765)
    if horses:
     pos=horses[0][1];heights.append(pos)
     if abs(pos[0]+1669.61)<8 and abs(pos[1]-2519.56)<8:break
   report['ride_samples']=heights;assert len(heights)>3,'No moving manor horse observed'
   assert any(abs(pos[0]+1669.61)<8 and abs(pos[1]-2519.56)<8 for pos in heights),'Horse did not finish original Manor route'
   w.pump(3);assert w.logout();w.s.close();w=None
  else:
   w.pump(4);assert report['mirror_timers'],'No breath timer while submerged'
   w.move_goto(w.pos[0]+0.1,w.pos[1],0.1);w.pump(3);assert report.get('mirror_stopped'),'Breath timer did not clear above water'
   w.move_goto(w.pos[0]+0.1,w.pos[1],-5);w.pump(.5)
   report['graceful_logout']=w.logout();w.s.close();w=None
   assert report['graceful_logout'],'Underwater fixture logout refused'
  report['passed']=True
 except Exception as e:report['error']=str(e);report['traceback']=__import__('traceback').format_exc()
 finally:
  if w:w.logout();w.s.close()
  pool.release(account,lease);report['binary_sha256']=hashlib.sha256((pool.runtime/'worldserver.exe').read_bytes()).hexdigest();args.out.write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps({k:v for k,v in report.items() if k in ('case','passed','error','terrain','mirror_timers')}));return int(not report['passed'])
if __name__=='__main__':sys.exit(main())
