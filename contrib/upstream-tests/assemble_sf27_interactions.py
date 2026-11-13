"""Adapt a large batch of published SF27 interaction, starter and profession repairs."""
from pathlib import Path
import json,re,hashlib
from sql_batch import statements,comma_values
from sf27_db import load_db
ROOT=Path(__file__).resolve().parents[2];db=load_db()
base=ROOT/'out/skyfire27/sql/old/5.4.8/world/SFDB_release_26.001_to_27.001'
names=['2026_08_17_world_01.sql','2026_08_24_world_02.sql','2026_08_25_world_00.sql','2026_08_25_world_01.sql','2026_08_31_world_01.sql','2026_09_01_world_01.sql','2026_09_01_world_02.sql','2026_09_06_world_00.sql','2026_09_06_world_01.sql','2026_09_14_world_00.sql','2026_09_21_world_04.sql','2026_09_21_world_09.sql','2026_10_04_world_02.sql','2026_10_22_world_00.sql']
paths=[base/n for n in names]+[ROOT/'out/skyfire27/sql/updates/world'/f'2026_11_06_world_{i:02}.sql' for i in [1,2,3]]
lines=['-- Project SkyFire SF27 interaction/starter/profession fixes at df19329b209fa1d1ae80444b43f6201d5dc124f0.', '-- Preserve unmentioned local rows/fields and active schema aliases.'];schemas={};tables=set();desired=[];ledger=[]
def info(t):
 if t not in schemas:schemas[t]=[x.split('\t') for x in db.q('SHOW COLUMNS FROM `'+t+'`;').splitlines()]
 return schemas[t]
def add(sql,t):lines.append(sql.rstrip(';')+';');tables.add(t)
def upsert(t,cols,rows):
 actual={x[0].lower():x[0] for x in info(t)};pk={x[0] for x in info(t) if x[3]=='PRI'}
 data=[dict(zip(cols,row)) for row in rows]
 aliases={'gossip_menu':{'MenuID':'entry','TextID':'text_id'},'gossip_menu_option':{'MenuID':'menu_id','OptionID':'id','OptionIcon':'option_icon','OptionText':'option_text','OptionType':'option_id','OptionNpcflag':'npc_option_npcflag','ActionMenuID':'action_menu_id','ActionPoiID':'action_poi_id','BoxCoded':'box_coded','BoxMoney':'box_money','BoxText':'box_text'}}
 for src,dst in aliases.get(t,{}).items():
  if src in cols:
   for row in data:row[dst]=row[src]
   if dst not in cols:cols.append(dst)
 if t=='creature_template_addon':
  for old,fields in [('bytes1',[('StandState',0),('VisFlags',16),('AnimTier',24)]),('bytes2',[('SheathState',0),('PvPFlags',8)])]:
   if old in cols:
    for name,shift in fields:
     if name not in cols:cols.append(name)
     for x in data:x[name]=str((int(x[old])>>shift)&255)
 if t=='smart_scripts':
  for row in data:
   if row['action_type']=='41' and row['action_param2']=='0':row['action_param2']=row['action_param1']
 normalized=[]
 for col in cols:
  if col.lower() not in actual:
   assert col=='VerifiedBuild' and t in ('gossip_menu','gossip_menu_option'),(t,col)
   continue
  normalized.append((col,actual[col.lower()]))
 cols=[b for a,b in normalized];data=[{b:x[a] for a,b in normalized} for x in data]
 nonkeys=[c for c in cols if c not in pk]
 stmt='INSERT '+('' if nonkeys else 'IGNORE ')+'INTO `'+t+'` ('+','.join('`'+c+'`' for c in cols)+') VALUES\n'+',\n'.join('('+','.join(x[c] for c in cols)+')' for x in data)
 if nonkeys:stmt+='\nON DUPLICATE KEY UPDATE '+','.join('`'+c+'`=VALUES(`'+c+'`)' for c in nonkeys)
 add(stmt,t);desired.extend(dict(table=t,values=x,keys=sorted(pk)) for x in data)
for path in paths:
 raw=path.read_bytes();cmds=statements(raw.decode('utf-8-sig'));start=len(lines);lines.append('\n-- '+path.name);adaptations=[]
 for cmd in cmds:
  cmd=cmd.replace('`skyfire_string`','`trinity_string`')
  m=re.match(r'(INSERT(?: IGNORE)? INTO|DELETE FROM|UPDATE)\s+`?(\w+)`?',cmd,re.I);assert m,cmd[:100]
  operation,t=m.groups()
  if t=='playercreateinfo_spell':
   adaptations.append('equivalent: unused legacy create-spell table; local DBC skill/race/class and SpellLevel gates remain active');continue
  # Preserve keyed custom rows: replacement inserts below own their declared fields.
  if operation.upper()=='DELETE FROM' and t in ('creature_template_addon','gossip_menu','gossip_menu_option','npc_trainer','creature_queststarter','creature_questender','npc_spellclick_spells','trinity_string'):
   # Only the empty trainer-menu placeholders are intentionally obsolete.
   if t=='gossip_menu_option' and 'OptionType' in cmd and "Train me in %" in cmd:add(cmd,t)
   else:adaptations.append('keyed upserts retain unrelated '+t+' content')
   continue
  if t=='smart_scripts' and operation.upper()=='DELETE FROM':
   if 'source_type' not in cmd:
    ids=re.findall(r'entryorguid`?\s*=\s*(\d+)',cmd);assert len(ids)==1
    scope=9 if len(ids[0])>5 else 0;cmd+=' AND source_type='+str(scope)
  if t=='gameobject_template' and '`flags`' in cmd:
   entry=re.search(r'`entry`\s*=\s*(\d+)',cmd)[1]
   # INTERACT_COND is the authored bit; leave other unrelated local flags intact.
   value='4' if entry=='195074' else '0'
   add(f'INSERT INTO gameobject_template_addon (entry,flags) VALUES ({entry},{value}) ON DUPLICATE KEY UPDATE flags='+('flags | 4' if value=='4' else 'flags & ~4'),'gameobject_template_addon')
   adaptations.append('gameobject flags use active addon table and preserve other bits');continue
  if t=='creature_template' and '`faction_A`' in cmd:
   faction=re.search(r'`faction_A`\s*=\s*(\d+)',cmd)[1];cmd=cmd.replace('SET ', 'SET `faction`='+faction+', ',1)
  if path.name=='2026_09_21_world_09.sql' and t=='creature_template':
   cmd+=' AND VehicleId=0 AND EXISTS (SELECT 1 FROM creature c WHERE c.id=creature_template.entry)'
  ins=re.match(r'INSERT(?: IGNORE)? INTO\s+`?(\w+)`?\s*\((.*?)\)\s*VALUES\s*(.*)',cmd,re.I|re.S)
  if ins:
   cols=[x.strip(' `\n\r\t') for x in comma_values(ins[2])];rows=[comma_values(x.strip()[1:-1]) for x in comma_values(ins[3])];assert all(len(x)==len(cols) for x in rows)
   upsert(t,cols,rows)
  else:
   assert operation.upper() in ('UPDATE','DELETE FROM');add(cmd,t)
   if t=='quest_template' and '`SpecialFlags`' in cmd:
    assert path.name=='2026_08_25_world_01.sql'
    add('INSERT INTO quest_template_addon (ID,SpecialFlags) VALUES (26383,2),(26385,2) ON DUPLICATE KEY UPDATE SpecialFlags=SpecialFlags | 2','quest_template_addon');adaptations.append('mirror exploration/event flag into active quest addon')
 ledger.append(dict(file=path.name,sha256=hashlib.sha256(raw).hexdigest(),source_statements=len(cmds),output_statements=len(lines)-start-1,adaptations=sorted(set(adaptations))))
p=ROOT/'sql/updates/world/2026_11_06_45_world_sf27_interactions.sql';p.write_text('\n'.join(lines)+'\n',encoding='utf-8')
report=dict(pin='df19329b209fa1d1ae80444b43f6201d5dc124f0',sources=ledger,tables=sorted(tables),desired=desired,sql_sha256=hashlib.sha256(p.read_bytes()).hexdigest())
(ROOT/'doc/sf27-interaction-data.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(dict(sources=len(ledger),statements=len(statements(p.read_text())),desired=len(desired),tables=sorted(tables))))
