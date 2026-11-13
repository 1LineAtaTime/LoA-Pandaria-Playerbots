"""Owned QA-schema Worgen transition regression. Fixtures are NOT full natural playthroughs.

For every ordinary Worgen class: seed only the prior checkpoint, log in below
pillory, inspect terrain packets and NPC visibility, accept/reward14375 through
real client packets, request/fully decode the pet journal, logout/re-enter.
Also inventory every Gilneas quest/class combination, retaining unexecuted
objectives explicitly. No personal character or account mutation is possible.
"""
import argparse,csv,json,sys,time,hashlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
sys.path[:0]=[str(ROOT/'contrib/quest-qa'),str(ROOT/'contrib/headless-client')]
from pool import Pool
import headless
from qa_actions import ObservedWorld
CLASSES=(1,3,4,5,8,9,11)

def teleport_packet(body):
 r=headless.ByteReader(body);g={i:r.read_bit() for i in (0,6,5,7,2)};transport=r.read_bit();g[4]=r.read_bit()
 tg={i:r.read_bit() for i in (1,3,6,4,5,2,0,7)} if transport else {}
 g[3]=r.read_bit();g[1]=r.read_bit();assert not r.read_bit()
 if transport:
  for i in (4,3,7,1,6,0,2,5):r.read_byte_seq(tg[i])
 for i in (4,7):r.read_byte_seq(g[i])
 z=r.read_f32();y=r.read_f32()
 for i in (2,3,5):r.read_byte_seq(g[i])
 x=r.read_f32();r.read_u32()
 for i in (0,6,1):r.read_byte_seq(g[i])
 orientation=r.read_f32();assert r.pos==len(body)
 return (x,y,z),orientation

def phase_packet(body):
 r=headless.ByteReader(body);mask={i:r.read_bit() for i in (0,3,1,4,6,2,7,5)}
 def guid(seq):
  for i in seq:r.read_byte_seq(mask[i])
 def values():
  size=r.read_u32();assert size%2==0 and size<4096
  return [r.read_u16() for _ in range(size//2)]
 guid((4,3,2));phases=values();guid((0,6));inactive=values();guid((1,7));areas=values();terrains=values();guid((5,));flags=r.read_u32()
 assert r.pos==len(body)
 return dict(phases=phases,terrains=terrains,areas=areas,inactive=inactive,flags=flags)

def journal_packet(body):
 r=headless.ByteReader(body);count=r.read_bits(19);assert count<1001;metadata=[]
 for _ in range(count):
  flags=not r.read_bit();g={3:r.read_bit(),7:r.read_bit()};length=r.read_bits(7);account_bound=r.read_bit()
  for i in (0,2,6):g[i]=r.read_bit()
  unknown=r.read_bit()
  for i in (1,5):g[i]=r.read_bit()
  breed=not r.read_bit();g[4]=r.read_bit();quality=not r.read_bit()
  assert not account_bound and not unknown
  metadata.append((g,length,flags,breed,quality))
 enabled=r.read_bit();slots=r.read_bits(25);assert slots==3;slotmeta=[]
 for _ in range(slots):
  locked=r.read_bit();unknown=r.read_bit();has_index=not r.read_bit();fake=r.read_bit()
  g={i:r.read_bit() for i in (0,1,7,6,4,2,5,3)}
  assert unknown and has_index and not fake
  slotmeta.append((g,locked))
 indexes=[]
 for g,locked in slotmeta:
  for i in (5,1,7,2,3,0,4,6):r.read_byte_seq(g[i])
  indexes.append(r.read_u8())
 assert indexes==[0,1,2]
 species=[]
 for g,length,flags,breed,quality in metadata:
  if quality:r.read_u8()
  r.read_u32();r.read_byte_seq(g[7]);r.read_u16();r.read_u32()
  if breed:r.read_u16()
  species.append(r.read_u32());r.read_byte_seq(g[2])
  if flags:r.read_u16()
  r.read_u32();display=r.read_u32();assert display>0;r.read_u32();r.read_string(length)
  for i in (6,5):r.read_byte_seq(g[i])
  r.read_u32();r.read_byte_seq(g[4]);r.read_u16()
  for i in (0,1,3):r.read_byte_seq(g[i])
 unknown=r.read_u16();assert r.pos==len(body)
 return dict(count=count,slots=slots,species=species,enabled=enabled,bytes=len(body))

def catalogue(pool,out):
 rows=pool.db.q('World',"SELECT q.ID,q.LogTitle,q.QuestSortID,q.RequiredClasses,q.AllowableRaces,a.AllowableClasses,COALESCE(a.PrevQuestID,0),q.RewardSpell FROM quest_template q LEFT JOIN quest_template_addon a ON a.ID=q.ID WHERE q.QuestSortID IN(4714,4755) OR q.ID IN(SELECT r.quest FROM creature_queststarter r JOIN creature c ON c.id=r.id WHERE c.map=654) ORDER BY q.ID")
 result=[]
 for qid,title,zone,legacy,races,classes,previous,reward in rows:
  for klass in CLASSES:
   classmask=int(classes) if classes not in ('NULL','0') else int(legacy)
   eligible=(not classmask or bool(classmask&(1<<(klass-1)))) and (not int(races) or bool(int(races)&2097152))
   result.append(dict(quest=int(qid),name=title,class_id=klass,eligible=eligible,verification='NOT_GAMEPLAY_TESTED' if eligible else 'NOT_APPLICABLE',reason='Catalogue entry; actions not executed by cage regression',reward_spell=int(reward)))
 with (out/'quest-class-matrix.csv').open('w',newline='',encoding='utf-8') as f:
  writer=csv.DictWriter(f,fieldnames=result[0]);writer.writeheader();writer.writerows(result)
 return result

def run_class(pool,klass,out):
 c=pool.ensure(22,klass);a=next(a for a in pool.state['accounts'] if a['id']==c['account_id']);lease=pool.lease(a);w=None
 result=dict(class_id=klass,character_guid=c['guid'],execution_profile='controlled-prerequisite-fixture',quest=14375,quest_credit_injected=False,gm=False,passed=False,phases=[],journals=[])
 try:
  pool.reset(c);guid=c['guid']
  pool.db.q('Character',f"UPDATE characters SET level=5,map=654,zone=4714,position_x=-1818.41,position_y=2294.27,position_z=0.5196,orientation=3.16,health=200 WHERE guid={guid}")
  pool.db.q('Character',f"INSERT INTO character_queststatus_rewarded(guid,quest,active) VALUES "+','.join(f'({guid},{q},1)' for q in (14078,14159,14293,14221,14222)))
  w=pool.connect(a);w.char_enum();tracker=ObservedWorld(w);previous=w.auto
  def observe(op,body):
   previous(op,body)
   if op==headless.SMSG_MOVE_TELEPORT:w.pos,w.orientation=teleport_packet(body)
   if op==0x02A2:result['phases'].append(phase_packet(body))
   if op==0x1542:result['journals'].append(journal_packet(body))
   try:tracker.handle(op,body,time.time())
   except (ValueError,KeyError,IndexError,EOFError):tracker.decode_errors+=1
  w.auto=observe;assert w.player_login(guid);w.pump(5)
  result['cage_position']=w.pos
  assert w.pos and abs(w.pos[2]-42.21)<3,'Pillory terrain/teleport did not recover height'
  assert result['phases'] and result['phases'][-1]['terrains']==[638],result['phases']
  genn=(0xF13<<52)|(36332<<32)|375757
  assert genn in tracker.visible,'Genn not visible in the live client view'
  w.quest_hello(genn);w.pump(.5);w.quest_accept(14375,genn);w.pump(1)
  assert 14375 in tracker.quest_slots.values(),'14375 was not accepted through questgiver'
  w.quest_complete(14375,genn);w.pump(.5);w.quest_reward(14375,0,genn);w.pump(3)
  for _ in range(2):w.send_packet(0x0A23,b'');w.pump(1)
  assert len(result['journals'])>=2,'No pet journal response'
  assert w.logout(),'Post-release logout refused';w.s.close();w=None
  assert pool.db.q('Character',f'SELECT quest FROM character_queststatus_rewarded WHERE guid={guid} AND quest=14375'),'14375 reward not persisted'
  auras=[int(r[0]) for r in pool.db.q('Character',f'SELECT spell FROM character_aura WHERE guid={guid}')]
  assert not set(auras)&{68630,68631,72872,76642,59073},auras
  result['release_auras']=auras;result['persisted_position']=pool.db.q('Character',f'SELECT map,position_x,position_y,position_z FROM characters WHERE guid={guid}')[0]
  w=pool.connect(a);w.char_enum();assert w.player_login(guid);w.pump(2);assert w.logout();w.s.close();w=None
  result['relogin']=True;result['passed']=True
 except Exception as e:result['error']=str(e);result['traceback']=__import__('traceback').format_exc()
 finally:
  if w:w.logout();w.s.close()
  pool.release(a,lease)
  (out/f'class-{klass}.json').write_text(json.dumps(result,indent=2)+'\n')
 return result

def main():
 p=argparse.ArgumentParser();p.add_argument('--out',type=Path,required=True);p.add_argument('--classes',default=','.join(map(str,CLASSES)));args=p.parse_args();args.out.mkdir(parents=True,exist_ok=True)
 pool=Pool();matrix=catalogue(pool,args.out);results=[]
 for klass in map(int,args.classes.split(',')):
  assert klass in CLASSES
  result=run_class(pool,klass,args.out);results.append(result);print(json.dumps(dict(class_id=klass,passed=result['passed'],error=result.get('error'))),flush=True)
 report=dict(binary_sha256=hashlib.sha256((pool.runtime/'worldserver.exe').read_bytes()).hexdigest(),catalogue_quests=len({r['quest'] for r in matrix}),eligible_quest_class_pairs=sum(r['eligible'] for r in matrix),gameplay_scope='14375 controlled transition only; full quest matrix is NOT gameplay certified',results=results,passed=all(r['passed'] for r in results),death_knight='Separate Acherus start; not a Gilneas class campaign')
 (args.out/'summary.json').write_text(json.dumps(report,indent=2)+'\n');return int(not report['passed'])
if __name__=='__main__':sys.exit(main())
