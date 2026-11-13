"""Read-only declared dependency audit for the enabled legendary modules.
This checks references, not spell mechanics or complete quest journeys.
"""
import json,re,struct,hashlib
from pathlib import Path
from sf27_db import load_db
ROOT=Path(__file__).resolve().parents[2]

def main():
 db=load_db();refs={'SPELL':{},'NPC':{},'ITEM':{}}
 files=[ROOT/'src/server/scripts/World/tarecgosa_scripts.cpp',ROOT/'src/server/scripts/World/fangs_of_father_scripts.cpp']
 for p in files:
  for kind,name,value in re.findall(r'\b(SPELL|NPC|ITEM)_([A-Z0-9_]+)\s*=\s*(\d+)',p.read_text(encoding='utf-8-sig')):refs[kind][kind+'_'+name]=int(value)
 dbc=ROOT/'exports/dbc/Spell.dbc';b=dbc.read_bytes();magic,count,fields,size,strings=struct.unpack_from('<4s4I',b)
 assert magic==b'WDBC' and size>=4 and 20+count*size+strings<=len(b),'Invalid Spell.dbc header'
 ids={struct.unpack_from('<I',b,20+i*size)[0] for i in range(count)}
 missing={'SPELL':{n:i for n,i in refs['SPELL'].items() if i not in ids}}
 for kind,table in [('NPC','creature_template'),('ITEM','item_template')]:
  wanted=set(refs[kind].values())
  present=set(map(int,db.q('SELECT entry FROM '+table+' WHERE entry IN ('+','.join(map(str,sorted(wanted)))+')').splitlines())) if wanted else set()
  missing[kind]={n:i for n,i in refs[kind].items() if i not in present}
 report=dict(scope='Declared enum dependencies only; not runtime behavior',spell_dbc_sha256=hashlib.sha256(b).hexdigest(),source_sha256={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in files},dependencies=refs,missing=missing,passed=not any(missing.values()))
 out=ROOT/'out/legendary-native-20261108/dependencies.json';out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps(dict(passed=report['passed'],checked={k:len(v) for k,v in refs.items()},missing=missing)))
 return int(not report['passed'])
if __name__=='__main__':raise SystemExit(main())
