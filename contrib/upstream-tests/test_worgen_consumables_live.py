"""Controlled native Worgen-form item-use checks on owned normal-security QA bodies."""
import sys,json,struct,time,argparse,subprocess,hashlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
sys.path[:0]=[str(ROOT/'contrib/quest-qa'),str(ROOT/'contrib/headless-client')]
from pool import Pool
from qa_actions import ObservedWorld
import headless

def single_aura(body):
 r=headless.ByteReader(body);target={7:r.read_bit()};all_auras=r.read_bit();count=r.read_bits(24)
 if all_auras or count!=1:return None
 for i in (6,1,3,0,4,2,5):target[i]=r.read_bit()
 if not r.read_bit():return None
 effects=r.read_bits(22);caster=r.read_bit();g={i:r.read_bit() for i in (3,4,6,1,5,2,0,7)} if caster else {}
 r.read_bits(22);r.read_bit();r.read_bit()
 if caster:
  for i in (3,2,1,6,4,0,5,7):r.read_byte_seq(g[i])
 r.read_u8();r.read_u16();return r.read_u32()

def main():
 parser=argparse.ArgumentParser();parser.add_argument('--world-pid',type=int,required=True);parser.add_argument('--report',type=Path,required=True);a=parser.parse_args();pool=Pool();results=[]
 def console(text):subprocess.run([sys.executable,str(ROOT/'contrib/windows/Send-ServerConsoleCommand.py'),str(a.world_pid),text],check=True)
 for klass in (4,8):
  char=pool.ensure(22,klass);account=next(x for x in pool.state['accounts'] if x['id']==char['account_id']);lease=pool.lease(account);w=None
  report=dict(class_id=klass,gm=False,execution_profile='controlled health/mana and Worgen-form fixture; real CMSG_USE_ITEM',auras=[],cast_failures=[],inventory_failures=[],passed=False)
  try:
   pool.reset(char);w=pool.connect(account);w.char_enum();tracker=ObservedWorld(w);previous=w.auto
   def observe(op,body):
    previous(op,body)
    if op==0x72:
     aura=single_aura(body)
     if aura:report['auras'].append(aura)
    if op==headless.SMSG_CAST_FAILED:report['cast_failures'].append(list(struct.unpack_from('<II',body)))
    if op==0x0c1e:report['inventory_failures'].append(body.hex())
    try:tracker.handle(op,body,time.time())
    except (ValueError,KeyError,IndexError,EOFError):tracker.decode_errors+=1
   w.auto=observe;assert w.player_login(char['guid']);w.pump(1)
   for action in ('castself 97709','additem 4536 5','additem 159 5'):console(f"botscenario {char['name']} {action}")
   w.pump(.5);console('saveall');w.pump(.5)
   deadline=time.monotonic()+10
   while True:
    rows=pool.db.q('Character',f"SELECT i.itemEntry,i.guid,v.bag,v.slot FROM item_instance i JOIN character_inventory v ON v.item=i.guid WHERE i.owner_guid={char['guid']} AND i.itemEntry IN(4536,159)")
    if len(rows)==2 or time.monotonic()>deadline:break
    time.sleep(.1)
   assert len(rows)==2,'Native fixture items not saved';items={int(r[0]):r for r in rows}
   assert 97709 in report['auras'],'Worgen-form aura did not apply'
   console(f"botscenario {char['name']} sethp 30");w.pump(.25);before=tracker.health.get(w.char_guid)
   item=items[4536];assert item[2]=='0';w.use_item(255,int(item[3]),(0x400<<52)|int(item[1]));w.pump(4)
   report['health_before']=before;report['health_after']=tracker.health.get(w.char_guid)
   assert 433 in report['auras'],'Apple food aura not applied'
   assert report['health_after']>before,'Food did not restore health'
   w.send_packet(0x03e6,struct.pack('<I',0));w.pump(.25)
   if klass==8:console(f"botscenario {char['name']} setmana 0");w.pump(.25)
   item=items[159];assert item[2]=='0';w.use_item(255,int(item[3]),(0x400<<52)|int(item[1]));w.pump(3)
   report['drink_applied']=430 in report['auras']
   if klass==8:assert report['drink_applied'],'Mage drink aura not applied'
   console('saveall');w.pump(.5);assert w.logout();w.s.close();w=None
   report['passed']=True
  except Exception as e:report['error']=str(e);report['traceback']=__import__('traceback').format_exc()
  finally:
   if w:w.logout();w.s.close()
   pool.release(account,lease);results.append(report)
 a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(dict(results=results,passed=all(r['passed'] for r in results),binary_sha256=hashlib.sha256((pool.runtime/'worldserver.exe').read_bytes()).hexdigest()),indent=2)+'\n');print(json.dumps(results));return int(not all(r['passed'] for r in results))
if __name__=='__main__':sys.exit(main())
