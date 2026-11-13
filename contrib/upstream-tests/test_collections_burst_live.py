"""Replay the observed18414 Shift+P catalogue burst on an owned QA client."""
import sys,json,struct,time,argparse,hashlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
sys.path[:0]=[str(ROOT/'contrib/quest-qa'),str(ROOT/'contrib/headless-client')]
from pool import Pool
import headless

def main():
 p=argparse.ArgumentParser();p.add_argument('--report',type=Path,required=True);args=p.parse_args();pool=Pool();char=pool.ensure(22,1);account=next(a for a in pool.state['accounts'] if a['id']==char['account_id']);lease=pool.lease(account);w=None
 entries=json.loads((Path(__file__).parent/'fixtures/shiftp-18414-creature-queries.json').read_text())['entries'];responses=[];report=dict(passed=False,requests=len(entries),execution_profile='normal-security-client',gm=False,source='Observed real client326-query first-open collections burst')
 try:
  pool.reset(char);w=pool.connect(account);w.char_enum();assert w.player_login(char['guid']);w.pump(1);previous=w.auto
  def observe(op,body):
   previous(op,body)
   if op==0x048b:responses.append(struct.unpack_from('<I',body)[0])
   if op==headless.SMSG_PONG:report['pong']=True
  w.auto=observe
  for entry in entries:w.send_packet(0x0842,struct.pack('<I',entry))
  w.pump(3);w.ping();w.pump(1)
  report['responses']=len(responses);report['missing']=sorted(set(entries)-set(responses))
  assert len(responses)==len(entries) and not report['missing'],'Catalogue queries were cut off'
  assert report.get('pong'),'Connection lost after collections burst'
  w.send_packet(headless.CMSG_PING,struct.pack('<II',0xFFFFFFFF,123));w.pump(.5)
  assert w.logout(),'Client could not log out after collections burst';w.s.close();w=None
  deadline=time.monotonic()+10
  while True:
   latency=int(pool.db.q('Character',f"SELECT latency FROM characters WHERE guid={char['guid']}")[0][0])
   if latency==0xFFFFFF or time.monotonic()>deadline:break
   time.sleep(.1)
  report['oversized_latency_saved']=latency
  assert latency==0xFFFFFF,'Untrusted latency exceeded the database field or save failed'
  report['passed']=True
 except Exception as e:report['error']=str(e);report['traceback']=__import__('traceback').format_exc()
 finally:
  if w:w.s.close()
  pool.release(account,lease);report['binary_sha256']=hashlib.sha256((pool.runtime/'worldserver.exe').read_bytes()).hexdigest();args.report.parent.mkdir(parents=True,exist_ok=True);args.report.write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps(report));return int(not report['passed'])
if __name__=='__main__':sys.exit(main())
