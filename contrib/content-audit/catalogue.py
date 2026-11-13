"""Read-only, evidence-labelled 5.4.8 content catalogue. Presence is not behavior acceptance."""
import argparse,csv,hashlib,json,os,re,struct,subprocess,time
from pathlib import Path
from collections import Counter,defaultdict
from datetime import datetime,timezone
ROOT=Path(__file__).resolve().parents[2]

class Table:
 def __init__(self,path):
  self.path=Path(path);self.data=self.path.read_bytes();b=self.data
  if len(b)<20:raise ValueError('Short table: '+str(path))
  magic,n,f,size,strings=struct.unpack_from('<4s4I',b);self.meta=dict(file=self.path.name,magic=magic.decode('ascii',errors='replace'),records=n,fields=f,record_bytes=size,string_bytes=strings,sha256=hashlib.sha256(b).hexdigest());self.count=n;self.fields=f;self.size=size;self.start=20
  if magic==b'WDB2':
   if len(b)<48:raise ValueError('Short WDB2 header')
   tablehash,build,unknown,low,high,locale,extra=struct.unpack_from('<7I',b,20);self.start=48
   if high:
    if high<low:raise ValueError('Invalid WDB2 index range')
    self.start+=(high-low+1)*6
   self.meta.update(header_build=build,locale_mask=locale,min_id=low,max_id=high,table_hash=tablehash,header_extra=extra)
  elif magic!=b'WDBC':raise ValueError('Unsupported format '+repr(magic))
  if size<4 or self.start+n*size+strings>len(b):raise ValueError('Table bounds invalid: '+str(path))
  self.strings=b[self.start+n*size:self.start+n*size+strings]
 def value(self,row,column):
  if column<0 or column*4+4>self.size:raise ValueError('Column out of bounds')
  return struct.unpack_from('<I',self.data,self.start+row*self.size+column*4)[0]
 def text(self,row,column):
  offset=self.value(row,column)
  if offset>=len(self.strings):raise ValueError('Invalid string offset '+str(self.path))
  end=self.strings.find(b'\0',offset)
  if end<0:raise ValueError('Unterminated string')
  return self.strings[offset:end].decode('utf-8',errors='strict')
 def labels(self,column=None):
  ids={}
  for row in range(self.count):
   ident=self.value(row,0)
   if ident in ids:raise ValueError('Duplicate ID: '+str(self.path))
   ids[ident]=self.text(row,column) if column is not None else ''
  return ids

def sql_parts(text):
 quote=False;escape=False;depth=0;start=0
 for i,c in enumerate(text):
  if escape:escape=False;continue
  if quote and c=='\\':escape=True;continue
  if c=="'":quote=not quote;continue
  if quote:continue
  if c=='(':depth+=1
  elif c==')':depth-=1
  elif c==',' and depth==0:yield text[start:i];start=i+1
 if quote or depth:raise ValueError('Unbalanced SQL value list')
 yield text[start:]

def sql_text(value):
 if value=='NULL':return ''
 if not(value.startswith("'") and value.endswith("'")):return value
 value=value[1:-1].replace("''", "'")
 return re.sub(r'\\(.)',lambda m:{'n':'\n','r':'\r','t':'\t','0':'\0'}.get(m[1],m[1]),value)

def reference_tables(path,tables):
 s=Path(path).read_text(encoding='utf-8-sig');result={}
 for table,name in tables.items():
  m=re.search(r'CREATE TABLE `'+table+r'` \((.*?)\) ENGINE=',s,re.S)
  if not m:raise ValueError('Reference schema missing '+table)
  cols=re.findall(r'^  `([^`]+)` ',m[1],re.M);index=next(i for i,c in enumerate(cols) if c.lower()==name.lower());rows={}
  for m in re.finditer(r'INSERT INTO `'+table+r'` VALUES ([^\n]+)',s):
   for row in sql_parts(m[1].rstrip(';\r')):
    ident=int(row[1:row.index(',')]);parts=sql_parts(row[1:-1]);label=''
    for i,v in enumerate(parts):
     if i==index:label=sql_text(v);break
    if ident in rows:raise ValueError('Duplicate reference key')
    rows[ident]=label
  result[table]=rows
 return result

class World:
 def __init__(self,config,client):
  s=Path(config).read_text(encoding='utf-8-sig');h,port,user,self.password,db=re.search(r'^WorldDatabaseInfo\s*=\s*"([^"]+)"',s,re.M)[1].split(';');self.schema=db
  self.cmd=[str(Path(client).resolve()),'--no-defaults','--protocol=TCP','--host='+h,'--port='+port,'--user='+user,'--database='+db,'--batch','--skip-column-names','--default-character-set=utf8mb4']
 def q(self,sql):
  sql=sql.strip().rstrip(';')
  if not sql.upper().startswith('SELECT') or ';' in sql or re.search(r'\b(INTO|OUTFILE|DUMPFILE|SLEEP|BENCHMARK)\b',sql,re.I):raise ValueError('One read-only SELECT required')
  marker='__CONTENT_CATALOGUE_DONE__';p=subprocess.run(self.cmd,input=sql.rstrip(';')+"; SELECT '"+marker+"';",text=True,encoding='utf-8',capture_output=True,env=dict(os.environ,MYSQL_PWD=self.password),timeout=120)
  if p.returncode:raise RuntimeError(p.stderr.replace(self.password,'[redacted]')[-1200:])
  if not p.stdout.rstrip().endswith(marker):raise RuntimeError('Incomplete database read')
  data=p.stdout.rsplit(marker,1)[0].strip('\r\n');return [x.split('\t') for x in data.splitlines()] if data else []

def write_csv(path,rows,columns):
 temp=path.with_suffix(path.suffix+'.tmp')
 with temp.open('w',encoding='utf-8-sig',newline='') as f:
  w=csv.DictWriter(f,fieldnames=columns,extrasaction='raise',lineterminator='\n');w.writeheader()
  for row in rows:
   w.writerow({k:("'"+v if isinstance(v,str) and v.startswith(('=','+','-','@')) else v) for k,v in row.items()})

 for attempt in range(5):
  try:os.replace(temp,path);break
  except OSError:
   if attempt==4:raise
   time.sleep(.2)

class Catalogue:
 def __init__(self,args):
  self.a=args;self.out=args.output;self.out.mkdir(parents=True,exist_ok=True);self.db=World(args.config,args.client);self.sources=[];self.summary=[];self.fmt=(ROOT/'src/server/game/DataStores/DBCfmt.h').read_text();self.tables={}
 def table(self,name):
  if name not in self.tables:
   t=Table(self.a.data/name);self.tables[name]=t;self.sources.append(t.meta)
  return self.tables[name]
 def export(self,name,rows,columns,note):
  rows=list(rows);write_csv(self.out/(name+'.csv'),rows,columns);counts=Counter(r.get('comparison','not_compared') for r in rows);self.summary.append(dict(category=name,records=len(rows),comparison=dict(counts),name_difference_rows=sum(bool(r.get('name_differs')) for r in rows),note=note));print(name,len(rows),dict(counts),flush=True)
 def merge(self,name,reference,server,note,client_ids=None,extra=None,qa=None):
  rows=[]
  for ident in sorted(set(reference)|set(server)):
   present=ident in server;expected=ident in reference
   row=dict(id=ident,name=server.get(ident,reference.get(ident,'')),reference_name=reference.get(ident,''),server_name=server.get(ident,''),name_differs=bool(reference.get(ident,'') and server.get(ident,'') and reference[ident].strip()!=server[ident].strip()),reference_present=expected,server_present=present,comparison='present_in_both' if present and expected else 'reference_only_review' if expected else 'server_only_review',behavior=(qa or {}).get(ident,'not_evaluated'))
   if client_ids is not None:row['client_id_present']=ident in client_ids
   if extra is not None:row.update(extra.get(ident,dict(native_script='',registration='not_configured',spawn_count=0)))
   rows.append(row)
  cols=['id','name','reference_name','server_name','name_differs','reference_present','server_present','comparison','behavior']+(['client_id_present'] if client_ids is not None else [])+(['native_script','registration','spawn_count'] if extra is not None else [])
  self.export(name,rows,cols,note)
 def run(self):
  refs=reference_tables(self.a.reference_sql,dict(quest_template='LogTitle',creature_template='name',gameobject_template='name'))
  source=dict(file=self.a.reference_sql.name,sha256=hashlib.file_digest(self.a.reference_sql.open('rb'),'sha256').hexdigest(),kind='LoA community database peer reference; not Blizzard complete truth');
  if source['sha256']=='ab00066b89ab79649e2cd475781f4aecbd394c5520b3a9584fdcf45ae429f28a':source.update(release='v1.0.4',commit='e135742e1891d3e653d27966da2dc70e793f203e',url='https://github.com/1LineAtaTime/LoA-Pandaria-Playerbots/releases/tag/v1.0.4',verified_archive_member='world.sql')
  self.sources.append(source)
  quest={int(i):n for i,n in self.db.q('SELECT ID,LogTitle FROM quest_template')};qclient=self.table('QuestV2.dbc').labels();qa={}
  if self.a.qa_report:
   for r in json.loads(self.a.qa_report.read_text())['gameplay']:
    if r.get('quest') and r['status']=='passed':qa[r['quest']]='tested_delivery_scenario' if 'quest_accepted_and_rewarded' in r.get('checks',[]) else 'tested_acceptance_only'
  expected=dict(refs['quest_template']);expected.update({i:expected.get(i,'') for i in qclient})
  self.merge('quests',expected,quest,'Union of client QuestV2 IDs and peer LoA quest definitions. Includes obsolete/test content; not an active-quest whitelist.',qclient,qa=qa)
  items={int(i):n for i,n in self.db.q('SELECT entry,name FROM item_template')};ibase=self.table('Item.db2').labels();sparse=self.table('Item-sparse.db2').labels(100);expected={i:sparse.get(i,'') for i in set(ibase)|set(sparse)}
  self.merge('items',expected,items,'Client Item/Item-sparse union; absent IDs require build/hotfix/test-content review before import.')
  self.export('item_sparse_coverage',(dict(id=i,name=n,server_present=i in items,comparison='present_in_both' if i in items else 'reference_only_review') for i,n in sorted(sparse.items())),['id','name','server_present','comparison'],'Separate sparse item-detail subset; not all Item.db2 IDs have sparse rows.')
  missing=set(re.findall(r"ScriptName '([^']+)' exists in database, but no core script found",self.a.errors.read_text(errors='replace')))
  credit_rows=self.db.q('SELECT entry,difficulty,creditType,creditEntry,lastEncounterDungeon,comment FROM instance_encounters')
  credit_targets={int(row[3]) for row in credit_rows if int(row[2])==0}
  npc={};extra={};boss=[]
  for e,n,script,rank,flags,spawns in self.db.q("SELECT t.entry,t.name,t.ScriptName,t.rank,t.flags_extra,COUNT(c.guid) FROM creature_template t LEFT JOIN creature c ON c.id=t.entry GROUP BY t.entry"):
   i=int(e);npc[i]=n;extra[i]=dict(native_script=script,registration='reported_unregistered' if script in missing else 'not_flagged_in_log' if script else 'no_native_binding',spawn_count=int(spawns))
   criteria=[]
   if int(rank)==3:criteria.append('rank_3')
   if int(flags)&1:criteria.append('instance_bind_flag')
   if i in credit_targets:criteria.append('encounter_credit_creature')
   if script.startswith('boss_'):criteria.append('boss_script_name')
   if criteria:boss.append(dict(id=i,name=n,criteria=';'.join(criteria),spawn_count=int(spawns),native_script=script,registration=extra[i]['registration'],comparison='server_candidate_only',behavior='not_evaluated'))
  self.merge('creatures',refs['creature_template'],npc,'Peer-reference comparison. Creature.db2 is only a client subset and is not used as a universal NPC list.',extra=extra)
  self.export('boss_candidates',boss,['id','name','criteria','spawn_count','native_script','registration','comparison','behavior'],'Union of rank=3, instance-bind flag, encounter-credit creature and boss_* script candidates; not a complete or verified boss list.')
  objects={int(i):n for i,n in self.db.q('SELECT entry,name FROM gameobject_template')};self.merge('gameobjects',refs['gameobject_template'],objects,'Peer-reference templates; definition presence does not prove spawns, phases or interactions.')
  spell=self.table('Spell.dbc').labels(1);bindings=defaultdict(list)
  for sid,script in self.db.q('SELECT spell_id,ScriptName FROM spell_script_names'):bindings[abs(int(sid))].append(script)
  self.export('spells',(dict(id=i,name=n,native_binding_count=len(bindings[i]),unregistered_bindings=';'.join(s for s in bindings[i] if s in missing),comparison='client_asset_present',behavior='not_evaluated') for i,n in sorted(spell.items())),['id','name','native_binding_count','unregistered_bindings','comparison','behavior'],'Many spells are data-driven and need no C++ script. Client availability is not a mechanics test.')
  enc=self.table('DungeonEncounter.dbc');credits=defaultdict(list)
  for e,d,k,target,last,comment in credit_rows:credits[int(e)].append((d,k,target,last))
  rows=[]
  for r in range(enc.count):
   ident=enc.value(r,0);cr=credits[ident];rows.append(dict(id=ident,name=enc.text(r,5),map_id=enc.value(r,1),difficulty_id=enc.value(r,2),credit_rows=len(cr),credit_definitions=';'.join('/'.join(x) for x in cr),comparison='credit_mapping_present' if cr else 'credit_mapping_absent_review',behavior='not_evaluated'))
  self.export('dungeon_encounters',rows,['id','name','map_id','difficulty_id','credit_rows','credit_definitions','comparison','behavior'],'Direct DungeonEncounter ID to instance_encounters entry comparison. Script-controlled encounter credit may need separate inspection.')
  ji=self.table('JournalInstance.dbc');instances={ji.value(r,0):(ji.value(r,1),ji.text(r,6)) for r in range(ji.count)}
  self.export('journal_instances',(dict(id=i,name=n,map_id=m,comparison='client_journal_entry',behavior='not_evaluated') for i,(m,n) in sorted(instances.items())),['id','name','map_id','comparison','behavior'],'Client journal instance index, including historical/unused entries.')
  je=self.table('JournalEncounter.dbc');encounters={je.value(r,0):(je.value(r,6),je.text(r,9)) for r in range(je.count)}
  self.export('journal_encounters',(dict(id=i,name=n,journal_instance_id=inst,map_id=instances.get(inst,('',))[0],instance_name=instances.get(inst,('',''))[1],comparison='client_journal_entry',behavior='not_evaluated') for i,(inst,n) in sorted(encounters.items())),['id','name','journal_instance_id','map_id','instance_name','comparison','behavior'],'Journal IDs are not DungeonEncounter IDs; do not join them by numeric equality.')
  jc=self.table('JournalEncounterCreature.dbc');byname=defaultdict(list)
  for i,n in npc.items():byname[n.casefold().strip()].append(i)
  rows=[]
  for r in range(jc.count):
   label=jc.text(r,5);candidates=byname[label.casefold().strip()];method='exact_name'
   if not candidates and label in {'Dragha Shadowburner','Durumu'}:
    alias={'Dragha Shadowburner':'Drahga Shadowburner','Durumu':'Durumu the Forgotten'}[label];candidates=byname[alias.casefold().strip()];method='reviewed_name_alias'
   rows.append(dict(id=jc.value(r,0),name=label,journal_encounter_id=jc.value(r,1),creature_display_id=jc.value(r,2),candidate_method=method,creature_id_candidates=';'.join(map(str,candidates)),comparison='name_candidates_review' if candidates else 'no_exact_name_candidate_review',behavior='not_evaluated'))
  self.export('journal_boss_actors',rows,['id','name','journal_encounter_id','creature_display_id','candidate_method','creature_id_candidates','comparison','behavior'],'Display IDs are not creature template IDs. Exact names are only candidates, including phase variants; never a proof of a missing/working boss.')
  categories=[('maps','Map.dbc','MapEntryfmt'),('areas','AreaTable.dbc','AreaTableEntryfmt'),('achievements','Achievement.dbc','Achievementfmt'),('currencies','CurrencyTypes.dbc','CurrencyTypesfmt'),('factions','Faction.dbc','FactionEntryfmt'),('classes','ChrClasses.dbc','ChrClassesEntryfmt'),('races','ChrRaces.dbc','ChrRacesEntryfmt'),('specializations','ChrSpecialization.dbc','ChrSpecializationfmt'),('skill_lines','SkillLine.dbc','SkillLinefmt'),('taxi_nodes','TaxiNodes.dbc','TaxiNodesEntryfmt'),('lfg_activities','LFGDungeons.dbc','LFGDungeonEntryfmt'),('talents','Talent.dbc','TalentEntryfmt'),('battlegrounds_arenas','BattleMasterList.dbc','BattlemasterListEntryfmt'),('achievement_criteria','Achievement_Criteria.dbc','AchievementCriteriafmt'),('scenarios','Scenario.dbc','Scenariofmt'),('scenario_steps','ScenarioStep.dbc','ScenarioStepfmt'),('research_projects','ResearchProject.dbc','ResearchProjectfmt'),('research_branches','ResearchBranch.dbc','ResearchBranchfmt')]
  for cat,file,fmtname in categories:
   fmt=re.search(r'\b'+fmtname+r'\s*\[\].*?"([^"]+)"',self.fmt)[1];label={'scenarios':1,'scenario_steps':5,'research_projects':1,'research_branches':1}.get(cat,fmt.index('s') if 's' in fmt else None);t=self.table(file);names=t.labels(label)
   if cat=='talents':names={t.value(r,0):spell.get(t.value(r,4),'') for r in range(t.count)}
   self.export(cat,(dict(id=i,name=n,comparison='client_asset_present',behavior='not_evaluated') for i,n in sorted(names.items())),['id','name','comparison','behavior'],'Loaded client table inventory only; no independent mechanics/availability verdict.')
  # Relationship lists preserve their own row IDs instead of conflating linked IDs.
  for cat,file,links in [('skill_ability_links','SkillLineAbility.dbc',{'skill_line_id':1,'spell_id':2}),('taxi_paths','TaxiPath.dbc',{'from_node_id':1,'to_node_id':2,'price':3}),('mount_item_links','ItemToMountSpell.db2',{'mount_spell_id':1}),('battle_pet_species','BattlePetSpecies.db2',{'creature_id':1,'summon_spell_id':3})]:
   t=self.table(file);rows=[]
   for r in range(t.count):
    row=dict(id=t.value(r,0));row.update({k:t.value(r,c) for k,c in links.items()})
    if cat=='battle_pet_species':row['name']=npc.get(row['creature_id'],'')
    elif cat=='mount_item_links':row['name']=items.get(row['id'],sparse.get(row['id'],''))
    elif cat=='skill_ability_links':row['name']=spell.get(row['spell_id'],'')
    elif cat=='taxi_paths':row['name']=''
    row.update(comparison='client_relationship_present',behavior='not_evaluated');rows.append(row)
   self.export(cat,rows,['id',*links,'name','comparison','behavior'],'Relationship inventory, not a comprehensive playable recipe/mount/pet acceptance list.')
  skills=self.table('SkillLine.dbc');professions={skills.value(r,0):skills.text(r,2) for r in range(skills.count) if skills.value(r,1) in (9,11)}
  abilities=self.table('SkillLineAbility.dbc');rows=[]
  for r in range(abilities.count):
   skill=abilities.value(r,1);sid=abilities.value(r,2)
   if skill in professions:rows.append(dict(id=abilities.value(r,0),skill_id=skill,skill_name=professions[skill],spell_id=sid,spell_name=spell.get(sid,''),comparison='profession_ability_candidate',behavior='not_evaluated'))
  self.export('profession_recipe_candidates',rows,['id','skill_id','skill_name','spell_id','spell_name','comparison','behavior'],'Primary/secondary profession ability candidates (skill categories 9/11), not all obtainable recipe items or discoveries.')
  # Broader world systems, kept distinct from expected/reference catalogues.
  systems=['npc_vendor','npc_trainer','creature_loot_template','gameobject_loot_template','spell_loot_template','fishing_loot_template','item_loot_template','reference_loot_template','disenchant_loot_template','skinning_loot_template','gossip_menu','gossip_menu_option','conditions','smart_scripts','instance_template','areatrigger_teleport','transports','game_event','game_event_creature','battle_pet_wild_pool','battle_pay_product','scenario_poi','spell_script_names','item_script_names']
  present={r[0] for r in self.db.q('SELECT TABLE_NAME FROM information_schema.TABLES WHERE TABLE_SCHEMA=DATABASE()')};rows=[]
  for table in systems:
   exists=table in present;count=int(self.db.q('SELECT COUNT(*) FROM `'+table+'`')[0][0]) if exists else ''
   rows.append(dict(system_table=table,table_present=exists,rows=count,comparison='observed_server_metadata_only',behavior='not_evaluated'))
  self.export('world_systems',rows,['system_table','table_present','rows','comparison','behavior'],'Exact counts of supporting systems; no reference-completeness verdict. No personal/account tables read.')
  review=[]
  queries=[('placed_creature_missing_template','SELECT c.id,COUNT(*) FROM creature c LEFT JOIN creature_template t ON t.entry=c.id WHERE t.entry IS NULL GROUP BY c.id'),('placed_gameobject_missing_template','SELECT g.id,COUNT(*) FROM gameobject g LEFT JOIN gameobject_template t ON t.entry=g.id WHERE t.entry IS NULL GROUP BY g.id'),('item_objective_missing_template','SELECT o.objectId,COUNT(*) FROM quest_objective o LEFT JOIN item_template t ON t.entry=o.objectId WHERE o.type=1 AND o.objectId>0 AND t.entry IS NULL GROUP BY o.objectId'),('creature_objective_missing_template','SELECT o.objectId,COUNT(*) FROM quest_objective o LEFT JOIN creature_template t ON t.entry=o.objectId WHERE o.type IN (0,3) AND o.objectId>0 AND t.entry IS NULL GROUP BY o.objectId'),('gameobject_objective_missing_template','SELECT o.objectId,COUNT(*) FROM quest_objective o LEFT JOIN gameobject_template t ON t.entry=o.objectId WHERE o.type=2 AND o.objectId>0 AND t.entry IS NULL GROUP BY o.objectId')]
  integrity=[]
  for kind,query in queries:
   findings=self.db.q(query);integrity.append(dict(check=kind,distinct_ids=len(findings),references=sum(int(r[1]) for r in findings)))
   for ident,count in findings:review.append(dict(kind=kind,id=ident,reference_count=count,client_item_base_present=int(ident) in ibase if kind.startswith('item_') else '',client_item_sparse_present=int(ident) in sparse if kind.startswith('item_') else '',disposition='investigate_version_availability_and_behavior'))
  self.export('reference_integrity_findings',review,['kind','id','reference_count','client_item_base_present','client_item_sparse_present','disposition'],'Concrete current references needing review; not automatically an active/obtainable quest.')
  (self.out/'reference-integrity-summary.json').write_text(json.dumps(integrity,indent=2)+'\n')
  # Hash/provenance index of every client table, including unsupported/empty files.
  metas=[]
  for path in sorted(self.a.data.iterdir()):
   if path.suffix.lower() not in ('.dbc','.db2'):continue
   try:meta=Table(path).meta;meta['status']='header_validated'
   except ValueError as e:meta=dict(file=path.name,bytes=path.stat().st_size,sha256=hashlib.file_digest(path.open('rb'),'sha256').hexdigest(),status='empty_file_review' if not path.stat().st_size else 'not_parsed',reason=str(e))
   metas.append(meta)
  (self.out/'client-table-provenance.json').write_text(json.dumps(metas,indent=2)+'\n')
  report=dict(schema_version=1,generator_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),snapshot_note='Independent SELECT snapshots of public world definitions; no account/character data. Data can change if a concurrent migration runs.',generated_utc=datetime.now(timezone.utc).isoformat(),target='5.4.8 build 18414',reference_caveat='Local DB2 headers often record 18273; extraction/build provenance needs verification. No claim that local assets are a pristine 18414 master.',world_schema=self.db.schema,source_revision=subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),categories=self.summary,sources=self.sources,reference_sql=source,all_content_verified=False)
  (self.out/'summary.json').write_text(json.dumps(report,indent=2)+'\n')
  lines=['# 5.4.8 content catalogue comparison','','Read-only snapshot. Differences are review queues; presence is not a passing gameplay test.','','| Category | Rows | Comparison counts |','|---|---:|---|']
  for r in self.summary:lines.append('| ['+r['category']+']('+r['category']+'.csv) | '+str(r['records'])+' | '+str(r['comparison'])+' |')
  artifacts={p.name:dict(bytes=p.stat().st_size,sha256_lf=hashlib.sha256(p.read_bytes().replace(b'\r\n',b'\n')).hexdigest()) for p in self.out.glob('*.csv')}
  (self.out/'catalogue-files.json').write_text(json.dumps(artifacts,indent=2)+'\n')
  lines+=['','See summary.json for source hashes, provenance and category-specific limits.']
  (self.out/'README.md').write_text('\n'.join(lines)+'\n',encoding='utf-8')

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--config',type=Path,default=ROOT/'exports/worldserver.conf');p.add_argument('--client',type=Path,default=ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe');p.add_argument('--data',type=Path,default=ROOT/'exports/dbc');p.add_argument('--reference-sql',type=Path,default=ROOT/'out/loa-world-reference.sql');p.add_argument('--output',type=Path,default=ROOT/'doc/content-catalogue-548');p.add_argument('--qa-report',type=Path);p.add_argument('--errors',type=Path,default=ROOT/'exports/logs/Errors.txt');a=p.parse_args();Catalogue(a).run()
if __name__=='__main__':main()
