"""SF27 typed-enchantment, quest-credit and duplicate-spawn compatibility batch."""
from pathlib import Path
import re,json,hashlib,struct,collections
from sql_batch import statements,comma_values
from sf27_db import load_db
ROOT=Path(__file__).resolve().parents[2];db=load_db();base=ROOT/'out/skyfire27/sql/old/5.4.8/world/SFDB_release_26.001_to_27.001';updates=ROOT/'out/skyfire27/sql/updates/world'
paths=[base/n for n in ['2026_08_20_world_07.sql','2026_08_20_world_08.sql','2026_08_26_world_03.sql','2026_09_21_world_08.sql','2026_09_21_world_11.sql']]+[updates/n for n in ['2026_11_05_world_00.sql','2026_11_06_world_04.sql','2026_11_06_world_05.sql']]
lines=['-- Published SF27 compatibility batch at df19329b209fa1d1ae80444b43f6201d5dc124f0.', '-- Typed suffix pools preserve property pools; duplicate cleanup requires an actual retained sibling.'];tables=set();desired=[];ledger=[];schemas={}
def emit(sql,t):lines.append(sql.rstrip(';')+';');tables.add(t)
def upsert(t,cols,rows):
 if t not in schemas:schemas[t]=[x.split('\t') for x in db.q('SHOW COLUMNS FROM `'+t+'`;').splitlines()]
 pk={x[0] for x in schemas[t] if x[3]=='PRI'};values=[dict(zip(cols,row)) for row in rows];nonkeys=[c for c in cols if c not in pk]
 sql='INSERT '+('' if nonkeys else 'IGNORE ')+'INTO `'+t+'` ('+','.join('`'+c+'`' for c in cols)+') VALUES\n'+',\n'.join('('+','.join(row[c] for c in cols)+')' for row in values)
 if nonkeys:sql+='\nON DUPLICATE KEY UPDATE '+','.join('`'+c+'`=VALUES(`'+c+'`)' for c in nonkeys)
 emit(sql,t);desired.extend(dict(table=t,values=row,keys=sorted(pk)) for row in values)
b=(db.runtime_root/'exports/dbc/ItemRandomSuffix.dbc').read_bytes();_,n,f,size,ss=struct.unpack_from('<4s4I',b);suffixids={struct.unpack_from('<I',b,20+i*size)[0] for i in range(n)}
for path in paths:
 raw=path.read_bytes();start=len(lines);lines.append('\n-- '+path.name);notes=[]
 for cmd in statements(raw.decode('utf-8-sig')):
  t=re.match(r'(?:UPDATE|DELETE FROM|INSERT INTO)\s+`?(\w+)`?',cmd,re.I)[1]
  if t=='item_enchantment_template' and cmd.upper().startswith('DELETE'):
   if path.name.endswith('_08.sql'):
    entryclause=re.search(r'WHERE\s+`entry`\s+IN\s*\((.*?)\)',cmd,re.S|re.I)[1]
    emit('DELETE FROM item_enchantment_template WHERE type=1 AND entry IN ('+entryclause+') AND ench NOT IN ('+','.join(map(str,sorted(suffixids)))+')',t);notes.append('filter only suffix-type rows against actual client DBC; retain property-type rows')
   else:emit(cmd+' AND type=1',t)
   continue
  if t in ('gameobject','creature') and cmd.upper().startswith('DELETE'):
   alias='g';guard='keeper.id=g.id AND keeper.map=g.map AND keeper.spawnMask=g.spawnMask AND keeper.phaseMask=g.phaseMask AND keeper.phaseId=g.phaseId AND keeper.phaseGroup=g.phaseGroup'
   distance='ABS(keeper.position_x-g.position_x)<0.01 AND ABS(keeper.position_y-g.position_y)<0.01 AND ABS(keeper.position_z-g.position_z)<0.01'
   if t=='creature':distance='POW(keeper.position_x-g.position_x,2)+POW(keeper.position_y-g.position_y,2)+POW(keeper.position_z-g.position_z,2)<9'
   where=re.split(r'\bWHERE\b',cmd,maxsplit=1,flags=re.I)[1];where=re.sub(r'`(guid|id)`',r'g.`\1`',where)
   emit(f'DELETE g FROM `{t}` g JOIN `{t}` keeper ON '+guard+' AND '+distance+' AND keeper.guid<g.guid WHERE '+where,t);notes.append('remove only when lower-GUID same-phase sibling actually exists');continue
  ins=re.match(r'INSERT INTO\s+`?(\w+)`?\s*\((.*?)\)\s*VALUES\s*(.*)',cmd,re.I|re.S)
  if ins:
   cols=[x.strip(' `\n\r\t') for x in comma_values(ins[2])];rows=[comma_values(x.strip()[1:-1]) for x in comma_values(ins[3])]
   if t=='item_enchantment_template':cols.append('type');rows=[row+['1'] for row in rows];assert all(int(row[1]) in suffixids for row in rows);notes.append('source pools are suffix pools; set local type=1 explicitly')
   if t=='smart_scripts':
    for row in rows:
     data=dict(zip(cols,row))
     if data['action_type']=='41' and data['action_param2']=='0':row[cols.index('action_param2')]=data['action_param1']
   upsert(t,cols,rows)
  else:emit(cmd,t)
 ledger.append(dict(file=path.name,sha256=hashlib.sha256(raw).hexdigest(),output_statements=len(lines)-start-1,adaptations=sorted(set(notes))))
# SQL45's old upstream loot owner differs from the active local template owner.
emit('DELETE FROM gameobject_loot_template WHERE entry=14980 AND item=16332 AND ChanceOrQuestChance=100 AND mincountOrRef=1 AND maxcount=1 AND NOT EXISTS (SELECT 1 FROM gameobject_template WHERE data1=14980)','gameobject_loot_template')
emit('INSERT INTO gameobject_loot_template (entry,item,ChanceOrQuestChance,lootmode,groupid,mincountOrRef,maxcount) SELECT data1,16332,100,1,0,1,1 FROM gameobject_template g WHERE g.entry=178087 AND g.type=3 AND NOT EXISTS (SELECT 1 FROM gameobject_loot_template l WHERE l.entry=g.data1 AND l.item=16332)','gameobject_loot_template')
# The current published reference contains both the collection cast and visual cast.
rows=[line.split('\t') for line in db.q('SELECT npc_entry,spell_id,cast_flags,user_type FROM codex_sf27_reference_20261106.npc_spellclick_spells WHERE npc_entry=53561 ORDER BY spell_id;').splitlines()];assert rows==[['53561','99471','1','0'],['53561','99473','2','0']]
upsert('npc_spellclick_spells',['npc_entry','spell_id','cast_flags','user_type'],rows)
emit('UPDATE creature_template SET npcflag=npcflag | 16777216 WHERE entry=53561','creature_template')
p=ROOT/'sql/updates/world/2026_11_06_46_world_sf27_typed_loot_credit_spawns.sql';p.write_text('\n'.join(lines)+'\n',encoding='utf-8')
report=dict(pin='df19329b209fa1d1ae80444b43f6201d5dc124f0',sources=ledger,tables=sorted(tables),desired=desired,sql_sha256=hashlib.sha256(p.read_bytes()).hexdigest(),compatibility_fixes=['Thazzril uses actual template loot owner and retains existing quest-only item row','Stonebull Crayfish needs both published spellclick rows before static flag'])
(ROOT/'doc/sf27-typed-loot-data.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(dict(sources=len(ledger),desired=len(desired),statements=len(statements(p.read_text())),tables=sorted(tables))))
