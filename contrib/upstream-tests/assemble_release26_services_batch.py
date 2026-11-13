"""Adapt pinned release-26 loot/vendor/trainer and class spell data, preserving unrelated rows."""
import argparse,pathlib,csv,re,json,hashlib,collections,subprocess,os,struct
from sql_batch import statements,comma_values

def main():
 ap=argparse.ArgumentParser(description=__doc__)
 for k in ('config','client','patches','report','baseline'):ap.add_argument('--'+k,type=pathlib.Path,required=True)
 a=ap.parse_args();root=pathlib.Path(__file__).resolve().parents[2]
 m=re.search(r'^WorldDatabaseInfo\s*=\s*"([^"]+)"',a.config.read_text(encoding='utf-8-sig'),re.M);host,port,user,password,db=m[1].split(';')
 baseline=json.loads(a.baseline.read_text(encoding='utf-8'));binding_baseline=baseline.get('binding_baseline',baseline.get('bindings'));assert binding_baseline is not None
 command=[str(a.client.resolve()),'--no-defaults','--protocol=TCP','--host='+host,'--port='+port,'--user='+user,'--database='+db,'--batch','--skip-column-names','--default-character-set=utf8mb4']
 def q(sql):
  p=subprocess.run(command,input=sql,text=True,encoding='utf-8',capture_output=True,timeout=60,env=dict(os.environ,MYSQL_PWD=password))
  if p.returncode:raise RuntimeError(p.stderr.replace(password,'[REDACTED]')[:1000])
  return p.stdout
 b=(root/'exports/dbc/Spell.dbc').read_bytes();_,n,f,size,ss=struct.unpack_from('<4s4I',b);clientspells={struct.unpack_from('<I',b,20+i*size)[0] for i in range(n)}
 registry=set()
 for p in (root/'src/server/scripts').rglob('*.cpp'):
  s=re.sub(r'/\*.*?\*/|//[^\n]*','',p.read_text(encoding='utf-8-sig',errors='replace'),flags=re.S)
  registry.update(re.findall(r'new\s+(?:spell_script|aura_script|creature_script)\s*<[^>]+>\s*\(\s*"([^"]+)"',s))
  for cls,name in re.findall(r'(\w+)\s*\(\s*\)\s*:\s*(?:SpellScriptLoader|CreatureScript)\s*\(\s*"([^"]+)"',s):
   if re.search(r'new\s+'+re.escape(cls)+r'\s*\(',s):registry.add(name)
 lootnames={'2026_07_19_world_01.sql','2026_07_20_world_04.sql','2026_07_24_world_00.sql','2026_07_24_world_01.sql','2026_07_24_world_03.sql','2026_07_28_world_01.sql','2026_07_28_world_02.sql','2026_07_29_world_03.sql','2026_08_02_world_02.sql','2026_08_02_world_04.sql','2026_08_03_world_09.sql','2026_08_03_world_12.sql','2026_08_07_world_06.sql','2026_08_07_world_11.sql','2026_08_07_world_14.sql','2026_08_11_world_04.sql','2026_08_05_world_00.sql','2026_08_05_world_04.sql','2026_07_25_world_06.sql','2026_07_27_world_35.sql','2026_07_27_world_36.sql'}
 proconly={'2026_08_03_world_14.sql','2026_08_04_world_11.sql','2026_08_06_world_08.sql','2026_08_06_world_09.sql','2026_08_04_world_16.sql'}
 allow={'spell_script_names','spell_proc_event','spell_bonus_data','spell_linked_spell','npc_trainer','creature_template','conditions'}
 outputs={'loot':['-- Published SkyFire release26 services data, adapted to this fork. GPL-3.0 upstream contributors.','-- Merge incoming loot/vendor/trainer keys; preserve unmentioned local content.'], 'spells':['-- Published SkyFire release26 class spell data, adapted to registered local handlers.','-- Preserve custom non-client spell bindings and unrelated proc/bonus/link entries.']};ledger=[];desired=[];used_binding_names=set()
 schemas={}
 def upsert(t,cols,rows,defaults=False):
  if t not in schemas:schemas[t]=[x.split('\t') for x in q('SHOW COLUMNS FROM `'+t+'`;').splitlines()]
  info=schemas[t];actual={x[0].lower():x for x in info};cols=[actual[c.lower()][0] for c in cols];pk={x[0].lower() for x in info if x[3]=='PRI'}
  data=[dict(zip(cols,r)) for r in rows]
  if 'loot_template' in t and 'lootmode' not in {c.lower() for c in cols}:
   cols.append('lootmode')
   for r in data:r['lootmode']='1'
  if defaults:
   for x in info:
    if x[0].lower() not in {c.lower() for c in cols}:
     cols.append(x[0]);v='NULL' if x[4]=='NULL' else "'"+x[4].replace("'","''")+"'"
     for r in data:r[x[0]]=v
  if 'loot_template' in t and 'lootmode' not in {c.lower() for c in cols}:
   cols.append('lootmode')
   for r in data:r['lootmode']='1'
  if t=='gameobject_loot_template':
   expanded=[]
   for r in data:
    if r['entry']=='30020' and r['item']=='57764':expanded.extend(dict(r,entry=str(i)) for i in (203443,203964,203965,203966,203967,203968))
    else:expanded.append(r)
   data=expanded
  if 'loot_template' in t:
   for r in data:
    c=next(c for c in cols if c.lower()=='mincountorref');v=int(r[c].strip("'"))
    if -v in (24036,24116,24161,24736,44011):r[c]=str(-(6000000-v))
  for r in data:desired.append(dict(table=t,values=r,keys=[c for c in cols if c.lower() in pk]))
  return 'INSERT INTO `'+t+'` ('+','.join('`'+c+'`' for c in cols)+') VALUES\n'+',\n'.join('('+','.join(r[c] for c in cols)+')' for r in data)+'\nON DUPLICATE KEY UPDATE '+','.join('`'+c+'`=VALUES(`'+c+'`)' for c in cols if c.lower() not in pk)+';'
 for item in csv.DictReader((root/'doc/published-sql-compatibility-2026-11-05.csv').open(encoding='utf-8-sig')):
  p=a.patches/item['patch'];raw=p.read_bytes();assert hashlib.sha256(raw).hexdigest()==item['sha256'];s=raw.decode('utf-8-sig');tables=set(item['referenced_tables'].split(';'));names=set(re.findall(r"'(spell_\w+|npc_\w+)'",s));kind=None;reason='outside services/class subset'
  if item['patch'] in lootnames:kind='loot'
  elif item['patch'] in proconly:kind='spells'
  elif 'spell_script_names' in tables and tables<=allow and not item['patch'].startswith('2026_07_27_'):
   if names<=registry:kind='spells'
   else:reason='missing registered native handlers: '+','.join(sorted(names-registry))
  if not kind:ledger.append(dict(patch=item['patch'],status='not selected',reason=reason,sha256=item['sha256'],source=item['source']));continue
  if kind=='spells':used_binding_names.update(names)
  selected=[];excluded=[]
  for cmd in statements(s):
   ins=re.match(r'INSERT(?: IGNORE)? INTO\s+`?(\w+)`?\s*\((.*?)\)\s*VALUES\s*(.*)',cmd,re.I|re.S)
   if ins:
    t=ins[1];cols=[x.strip(' `\n\r\t') for x in comma_values(ins[2])];rows=[comma_values(row.strip()[1:-1]) for row in comma_values(ins[3])]
    if 'loot_template' in t or t in ('npc_vendor','npc_trainer','spell_script_names','spell_proc_event','spell_bonus_data','spell_linked_spell'):
     if t=='spell_script_names':
      # The all-key binding table needs INSERT IGNORE, not an empty UPDATE list.
      for row in rows:desired.append(dict(table=t,values=dict(zip(cols,row)),keys=cols))
      selected.append('INSERT IGNORE INTO `'+t+'` ('+','.join('`'+c+'`' for c in cols)+') VALUES\n'+',\n'.join('('+','.join(r)+')' for r in rows)+';')
     else:selected.append(upsert(t,cols,rows,True))
    else:excluded.append('insert '+t)
   elif re.match(r'DELETE FROM `?spell_script_names`?',cmd,re.I):
    where=re.split(r'\bWHERE\b',cmd,maxsplit=1,flags=re.I)[1]
    names_in_where=set(re.findall(r"'([^']+)'",where));candidates=[(sid,name) for sid,name in binding_baseline if name in names_in_where]
    virtual=' UNION ALL '.join(f"SELECT {int(sid)} AS spell_id,'{name}' AS ScriptName" for sid,name in candidates)
    matches=q('SELECT spell_id,ScriptName FROM ('+virtual+') captured WHERE '+where+';').splitlines() if candidates else []
    for line in matches:
     sid,name=line.split('\t')
     if abs(int(sid)) in clientspells:selected.append(f"DELETE FROM `spell_script_names` WHERE `spell_id`={sid} AND `ScriptName`='{name}';")
   elif re.match(r'DELETE FROM `?(spell_proc_event|spell_bonus_data|npc_trainer)`?',cmd,re.I):
    # Exact obsolete trainer spell IDs need removal; incoming rules are upserted.
    if cmd.lower().startswith('delete from `npc_trainer`') and re.search(r'\bspell`?\s+IN',cmd,re.I):selected.append(cmd+';')
   elif re.match(r'DELETE FROM `?spell_linked_spell`?',cmd,re.I):selected.append(cmd+';')
   elif re.match(r'DELETE FROM `?(\w+loot_template)`?',cmd,re.I):
    if re.search(r'\bitem`?\s*=',cmd,re.I):selected.append(cmd+';')
    elif item['patch']=='2026_08_05_world_00.sql':selected.append('DELETE FROM `skinning_loot_template` WHERE entry IN (217,422);')
   elif re.match(r'UPDATE\s+`?creature_template`?\s+SET',cmd,re.I):selected.append(cmd+';')
   elif item['patch']=='2026_08_05_world_04.sql' and re.match(r'UPDATE\s+`?gossip_menu_option`?',cmd,re.I):selected.append(cmd+';')
   else:excluded.append(cmd.split()[0]+' '+(re.search(r'`(\w+)`',cmd)[1] if re.search(r'`(\w+)`',cmd) else 'metadata'))
  if selected:outputs[kind]+=['\n-- '+item['patch']+' | '+item['source'],*selected]
  ledger.append(dict(patch=item['patch'],status='selected '+kind,sha256=item['sha256'],source=item['source'],output_statements=len(selected),excluded=excluded))
 for kind,n in [('loot','2026_11_06_36_world_release26_loot_services.sql'),('spells','2026_11_06_37_world_release26_class_spells.sql')]:
  (root/'sql/updates/world'/n).write_text('\n'.join(outputs[kind])+'\n',encoding='utf-8')
 a.report.write_text(json.dumps(dict(sources=ledger,desired=desired,binding_baseline=[(sid,n) for sid,n in binding_baseline if n in used_binding_names],native_ports=[{'source': 'https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/src/server/scripts/Spells/spell_warrior.cpp', 'sha256': '81d6baa16e240db358ac8b6ff66bd79fa2581b94232d9a1d674b24005284e741', 'handlers': ['spell_warr_last_stand', 'spell_warr_shattering_throw']}, {'source': 'https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/src/server/scripts/Spells/spell_druid.cpp', 'sha256': 'cf7c47082c9507843149183b5da2b41f6aa357897104f13748a87d9a3707eded', 'handlers': ['spell_dru_skull_bash']}, {'source': 'https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/src/server/scripts/Spells/spell_dk.cpp', 'sha256': '51b18ff38007770b5cc1157efb141be5e6ad03723be44e3fdc2153b9443c649b', 'handlers': ['spell_dk_runic_corruption']}]),indent=2)+'\n',encoding='utf-8');print(collections.Counter(x['status'] for x in ledger),'desired rows',len(desired))
if __name__=='__main__':main()
