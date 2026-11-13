"""Assemble pinned SF27 spell data against this fork's active schema, without live writes."""
import pathlib,sys,json,re,hashlib,struct
from sql_batch import statements,comma_values
ROOT=pathlib.Path(__file__).resolve().parents[2]
from sf27_db import load_db
db=load_db();RUNTIME=db.runtime_root
PIN='df19329b209fa1d1ae80444b43f6201d5dc124f0'
base=ROOT/'out/skyfire27/sql/old/5.4.8/world/SFDB_release_26.001_to_27.001'
sources=[base/f'2026_10_25_world_{i:02}.sql' for i in [1,2,3,4,6,7,8,9,10,11,12,13,14,16,17]]+[base/'2026_10_29_world_00.sql',ROOT/'out/skyfire27/sql/updates/world/2026_11_06_world_00.sql']
metadata=[json.loads(p.read_text()) for p in (ROOT/'doc').glob('sf27-*-native-port.json')]
active={x for m in metadata for x in m['upstream_names']}
owned=active|{x for m in metadata for x in m.get('local_original_registrations',[])}
b=(RUNTIME/'exports/dbc/Spell.dbc').read_bytes();_,n,f,size,ss=struct.unpack_from('<4s4I',b);clientids={struct.unpack_from('<I',b,20+i*size)[0] for i in range(n)}
ref='codex_sf27_reference_20261106'
canonical=[(int(x.split('\t')[0]),x.split('\t')[1]) for x in db.q(f'SELECT spell_id,ScriptName FROM {ref}.spell_script_names ORDER BY spell_id,ScriptName;').splitlines()]
canonical=[(i,n) for i,n in canonical if n in active and abs(i) in clientids]
assert len(canonical)>1400
# The local loader expands negative first-rank bindings. Collapse only complete,
# exactly matching chains, preserving the same set of affected spell ranks.
chains={}
for line in db.q('SELECT first_spell_id,spell_id FROM spell_ranks ORDER BY first_spell_id,rank;').splitlines():
 first,sid=map(int,line.split('\t'));chains.setdefault(first,set()).add(sid)
collapsed=[]
for name in sorted({name for sid,name in canonical}):
 ids={sid for sid,n in canonical if n==name}
 for first,chain in chains.items():
  if len(chain)>1 and chain<=ids:
   canonical=[(sid,n) for sid,n in canonical if n!=name or sid not in chain]
   canonical.append((-first,name));collapsed.append(dict(first=first,name=name,ranks=sorted(chain)))
canonical.sort()

baseline=[(int(x.split('\t')[0]),x.split('\t')[1]) for x in db.q('SELECT spell_id,ScriptName FROM spell_script_names ORDER BY spell_id,ScriptName;').splitlines()]
canonids={i for i,n in canonical};canonnames={n for i,n in canonical};canonset=set(canonical)
# Remove precisely captured obsolete owned bindings; local-only spell variants remain intact.
remove=[(i,n) for i,n in baseline if abs(i) in clientids and n in owned and (i in canonids or n in canonnames) and (i,n) not in canonset]
lines=['-- Project SkyFire SF27 spell modules and matched data; source '+PIN, '-- Scoped upserts preserve unmentioned local fields, custom systems and non-client spell variants.']
for i,n in remove:lines.append(f"DELETE FROM spell_script_names WHERE spell_id={i} AND ScriptName='{n}';")
lines.append('INSERT IGNORE INTO spell_script_names (spell_id,ScriptName) VALUES\n'+',\n'.join(f"({i},'{n}')" for i,n in canonical)+';')
schema={};ledger=[];desired=[];tables={'spell_script_names'};superseded=[]
def info(t):
 if t not in schema:schema[t]=[x.split('\t') for x in db.q('SHOW COLUMNS FROM `'+t+'`;').splitlines()]
 return schema[t]
def upsert(t,cols,rows):
 actual={x[0].lower():x[0] for x in info(t)};pk={x[0] for x in info(t) if x[3]=='PRI'}
 cols=[{'WDBVerified':'VerifiedBuild','spell_rank':'rank'}.get(x,x) for x in cols];cols=[actual[c.lower()] for c in cols]
 data=[dict(zip(cols,r)) for r in rows]
 if t=='spell_group' and 'comment' not in cols:
  cols.append('comment')
  for x in data:x['comment']="''"
 if t=='creature_template' and 'faction_A' in cols:
  assert all(x['faction_A']==x['faction_H'] for x in data)
  cols.append('faction')
  for x in data:x['faction']=x['faction_A']
 if t=='creature_template_addon':
  for old,fields in [('bytes1',[('StandState',0),('VisFlags',16),('AnimTier',24)]),('bytes2',[('SheathState',0),('PvPFlags',8)])]:
   if old in cols:
    for name,shift in fields:
     cols.append(name)
     for x in data:x[name]=str((int(x[old])>>shift)&255)
 nonkeys=[c for c in cols if c not in pk]
 result='INSERT '+('' if nonkeys else 'IGNORE ')+'INTO `'+t+'` ('+','.join('`'+c+'`' for c in cols)+') VALUES\n'+',\n'.join('('+','.join(x[c] for c in cols)+')' for x in data)
 if nonkeys:result+='\nON DUPLICATE KEY UPDATE '+','.join('`'+c+'`=VALUES(`'+c+'`)' for c in nonkeys)
 lines.append(result+';');tables.add(t)
 desired.extend(dict(table=t,values=x,keys=sorted(pk)) for x in data)
 if t=='creature_template':
  for x in data:
   models=[(i,int(x.get('modelid'+str(i+1),'0'))) for i in range(4) if int(x.get('modelid'+str(i+1),'0'))]
   if models:
    lines.append('DELETE FROM creature_template_model WHERE CreatureID='+x['entry']+' AND Idx IN ('+','.join(str(i) for i,m in models)+');')
    upsert('creature_template_model',['CreatureID','Idx','CreatureDisplayID','DisplayScale','Probability','VerifiedBuild'],[[x['entry'],str(i),str(m),'1',str(1/len(models)),x.get('VerifiedBuild','0')] for i,m in models])
for p in sources:
 lines.append('\n-- '+p.name);start=len(lines);skipped=[]
 for cmd in statements(p.read_text(encoding='utf-8-sig')):
  if cmd.upper().startswith('CREATE TABLE'):
   assert 'spell_areatrigger_template' in cmd;skipped.append('existing active local area-trigger schema retained');continue
  match=re.match(r'(?:INSERT(?: IGNORE)? INTO|DELETE FROM|UPDATE)\s+`?(\w+)`?',cmd,re.I);assert match,cmd[:100]
  t=match[1]
  if t=='spell_script_names':skipped.append('bindings resolved by current active native ownership');continue
  if cmd.upper().startswith('DELETE') and t in ('creature_template','creature_template_addon'):
   skipped.append('retain unmentioned local template/addon fields');continue
  ins=re.match(r'INSERT(?: IGNORE)? INTO\s+`?(\w+)`?\s*\((.*?)\)\s*VALUES\s*(.*)',cmd,re.I|re.S)
  if ins:
   cols=[x.strip(' `\n\r\t') for x in comma_values(ins[2])];rows=[comma_values(x.strip()[1:-1]) for x in comma_values(ins[3])];assert all(len(r)==len(cols) for r in rows)
   upsert(t,cols,rows)
  else:
   if t=='spell_ranks':cmd=cmd.replace('`spell_rank`','`rank`')
   assert cmd.upper().startswith(('DELETE','UPDATE'))
   lines.append(cmd+';');tables.add(t)
 ledger.append(dict(patch=p.name,sha256=hashlib.sha256(p.read_bytes()).hexdigest(),output_statements=len(lines)-start,adaptations=sorted(set(skipped))))
out=ROOT/'sql/updates/world/2026_11_06_44_world_sf27_native_spells.sql';out.write_text('\n'.join(lines)+'\n',encoding='utf-8')
report=dict(pin=PIN,sources=ledger,tables=sorted(tables),canonical_bindings=len(canonical),canonical_spell_bindings=canonical,rank_chain_adaptations=collapsed,removed_bindings=remove,desired=desired,local_nonclient_preserved=[x for x in baseline if x[1] in owned and abs(x[0]) not in clientids],sql_sha256=hashlib.sha256(out.read_bytes()).hexdigest())
(ROOT/'doc/sf27-native-spell-data.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
print(json.dumps(dict(statements=len(statements(out.read_text())),bindings=len(canonical),remove=len(remove),desired_rows=len(desired),tables=sorted(tables))))
