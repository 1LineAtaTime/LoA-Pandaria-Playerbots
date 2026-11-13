"""Compile remaining pinned SF27 content through an isolated public-content compatibility schema.

No live writes. Export only scoped active-schema differences; preserve custom keys.
"""
from pathlib import Path
import json,re,collections,hashlib,ast,math,subprocess
from sql_batch import statements,comma_values
from sf27_db import load_db
from assemble_release26_quest_batch import MAP,split_where
ROOT=Path(__file__).resolve().parents[2];db=load_db();live=db.database;fixture='codex_sf27_content_fixture_20261106';ref='codex_sf27_reference_20261106'
base=ROOT/'out/skyfire27/sql/old/5.4.8/world/SFDB_release_26.001_to_27.001'
names=['2026_08_17_world_05.sql','2026_08_20_world_01.sql','2026_08_24_world_03.sql','2026_08_25_world_02.sql','2026_08_25_world_03.sql','2026_08_26_world_05.sql','2026_08_27_world_00.sql','2026_08_27_world_01.sql','2026_08_31_world_02.sql','2026_09_02_world_00.sql','2026_09_10_world_00.sql','2026_09_21_world_03.sql','2026_09_21_world_05.sql','2026_09_23_world_06.sql','2026_10_07_world_00.sql','2026_10_07_world_01.sql','2026_10_22_world_01.sql','2026_10_24_world_00.sql','2026_10_24_world_02.sql','2026_10_25_world_00.sql','2026_10_25_world_05.sql']
names += ['2026_08_26_world_02.sql','2026_09_02_world_01.sql','2026_10_24_world_01.sql']
names.sort()
paths=[base/n for n in names]+[ROOT/'out/sf27-new-updates'/n for n in ['2026_11_06_world_06.sql','2026_11_06_world_07.sql']];sources=[];commands=[];variables={}
def number(s):
 s=s.strip();assert re.fullmatch(r'[0-9+\-(). *]+',s),s
 node=ast.parse(s,mode='eval')
 for part in ast.walk(node):assert isinstance(part,(ast.Expression,ast.Constant,ast.BinOp,ast.UnaryOp,ast.Add,ast.Sub,ast.Mult,ast.USub,ast.UAdd,ast.Load)),s
 return int(eval(compile(node,'<numeric SQL>','eval'),{'__builtins__':{}},{}))
for p in paths:
 if not p.exists() and p.parent.name=='sf27-new-updates':
  p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(subprocess.check_output(['git','show','a69eda918b5bd416078fd8d832560d0fd4fb9023:sql/updates/world/'+p.name],cwd=ROOT))
 raw=p.read_bytes();sources.append(dict(file=p.name,sha256=hashlib.sha256(raw).hexdigest()))
 for cmd in statements(raw.decode('utf-8-sig')):
  if cmd.upper().startswith('SET '):
   m=re.fullmatch(r'SET\s+(@\w+)\s*:?=\s*(.*)',cmd,re.I|re.S);assert m,cmd
   value=re.sub(r'@\w+',lambda m:str(variables[m[0].lower()]),m[2]);variables[m[1].lower()]=number(value);continue
  cmd=re.sub(r'@\w+',lambda m:str(variables[m[0].lower()]),cmd)
  commands.append((p.name,cmd))
# Public world tables only. Never copy auth, characters, mail or shop data.
tables=set()
for source,cmd in commands:
 cmd=cmd.replace('`npc_rank`','`rank`').replace('`WDBVerified`','`VerifiedBuild`')
 m=re.match(r'(?:UPDATE|DELETE FROM|INSERT(?: IGNORE)? INTO)\s+`?(\w+)`?',cmd,re.I);assert m,cmd[:100];tables.add(m[1])
tables.update(['quest_template_addon','quest_offer_reward','quest_request_items','creature_template_model','creature_template_movement','gameobject_template_addon'])
assert not tables & {'account','characters','mail','mail_items'}
info={t:[line.split('\t') for line in db.q('SHOW COLUMNS FROM `'+t+'`;').splitlines()] for t in tables}
original_columns={t:[x[0] for x in rows] for t,rows in info.items()}
keys={t:[x[0] for x in rows if x[3]=='PRI'] for t,rows in info.items()}
# Content rows without a primary key use full-row identity, avoiding deletion of unrelated conditions.
for t in tables:
 if not keys[t]:keys[t]=original_columns[t]
setup='CREATE DATABASE IF NOT EXISTS '+fixture+';\n'
for t in sorted(tables):setup+=f'DROP TABLE IF EXISTS {fixture}.`{t}`; CREATE TABLE {fixture}.`{t}` LIKE `{live}`.`{t}`; INSERT INTO {fixture}.`{t}` SELECT * FROM `{live}`.`{t}`;\n'
db.q(setup,timeout=180)
def q(sql):
 for attempt in range(3):
  result=db.q('USE '+fixture+';\n'+sql+"\nSELECT '__SF27_COMPLETE__';",timeout=180)
  if result.rstrip().endswith('__SF27_COMPLETE__'):return result.rsplit('__SF27_COMPLETE__',1)[0].rstrip('\r\n')
 raise RuntimeError('Incomplete fixture SQL response')
# Compatibility aliases are initialized from active local storage, then only changed values are mirrored back.
fields=dict(MAP,RequiredRaces=('quest_template','AllowableRaces'),RequiredClasses=('quest_template_addon','AllowableClasses'),SpecialFlags=('quest_template_addon','SpecialFlags'))
for old,(t,new) in fields.items():
 if old.lower() not in {c.lower() for c in original_columns['quest_template']}:
  typ='TEXT NULL' if old in ('Title','Details','Objectives','EndText','OfferRewardText','RequestItemsText') else 'BIGINT NOT NULL DEFAULT 0'
  q('ALTER TABLE quest_template ADD COLUMN `'+old+'` '+typ+';')
 if t=='quest_template':q('UPDATE quest_template SET `'+old+'`=`'+new+'`;')
 else:q(f'UPDATE quest_template qt LEFT JOIN `{t}` src ON qt.ID=src.ID SET qt.`{old}`=COALESCE(src.`{new}`,'+("''" if old.endswith('Text') else '0')+');')
q('ALTER TABLE gameobject_template ADD COLUMN flags INT UNSIGNED NOT NULL DEFAULT 0; UPDATE gameobject_template g LEFT JOIN gameobject_template_addon a ON a.entry=g.entry SET g.flags=COALESCE(a.flags,0);')
q('ALTER TABLE gameobject_template ADD COLUMN faction INT UNSIGNED NOT NULL DEFAULT 0; UPDATE gameobject_template g LEFT JOIN gameobject_template_addon a ON a.entry=g.entry SET g.faction=COALESCE(a.faction,0);')
q('ALTER TABLE creature_template ADD COLUMN ModLevel TINYINT NOT NULL DEFAULT 0, ADD COLUMN detection_range FLOAT NOT NULL DEFAULT 20;')
q('UPDATE creature_template SET faction_A=faction,faction_H=faction;')
q('UPDATE creature c SET spawndist=wander_distance;')
# Snapshot after alias initialization: untouched legacy compatibility values are never exported.
def snapshot(t):
 cols=original_columns[t];sql='SELECT '+','.join("IF(`"+c+"` IS NULL,'N',CONCAT('H',HEX(CAST(`"+c+'` AS BINARY))))' for c in cols)+' FROM `'+t+'`;'
 rows={}
 for line in q(sql).splitlines():
  vals=line.split('\t');assert len(vals)==len(cols),(t,len(vals),len(cols));data=dict(zip(cols,vals));key=tuple(data[c] for c in keys[t]);assert key not in rows,(t,key);rows[key]=data
 return rows
# Always compare the freshly cloned active world; a cached pre-migration snapshot can be stale.
before={t:snapshot(t) for t in tables}
# Resolve foreign spawn IDs by identity and location, allocating only genuinely missing spawns.
entity={};spatial={};reference={};mapping={'creature':{},'gameobject':{},'waypoint':{},'pool':{}};alloc={};notes=[]
for t in ('creature','gameobject'):
 cols='guid,id,map,position_x,position_y,position_z,phaseid,phasegroup'
 rows=[x.split('\t') for x in q('SELECT '+cols+',phaseMask FROM '+t+';').splitlines()];entity[t]={int(x[0]):x for x in rows};spatial[t]=collections.defaultdict(list)
 for row in rows:spatial[t][(int(row[1]),int(row[2]))].append(row)
 rows=[x.split('\t') for x in db.q('SELECT '+cols+',1 FROM '+ref+'.'+t+';').splitlines()];reference[t]={int(x[0]):x for x in rows};alloc[t]=max(entity[t],default=0)+1000
alloc['waypoint']=int(q('SELECT COALESCE(MAX(id),0)+1000 FROM waypoint_data;').strip());alloc['pool']=int(q('SELECT COALESCE(MAX(entry),0)+1000 FROM pool_template;').strip())
def map_spawn(t,source,row=None):
 source=int(source)
 if source==0:return 0
 if source in mapping[t]:return mapping[t][source]
 expected=row or reference[t].get(source)
 if not expected:raise RuntimeError('Missing source identity for '+t+' '+str(source))
 def matches(x):return x[1:3]==expected[1:3] and x[6:9]==(expected[6:9] if len(expected)>8 else expected[6:8]+['1']) and sum((float(x[i])-float(expected[i]))**2 for i in (3,4,5))<4
 same=entity[t].get(source)
 if same and matches(same):chosen=source
 else:
  candidates=[x for x in spatial[t].get((int(expected[1]),int(expected[2])),[]) if matches(x)]
  if candidates:chosen=int(min(candidates,key=lambda x:sum((float(x[i])-float(expected[i]))**2 for i in (3,4,5)))[0])
  else:chosen=alloc[t];alloc[t]+=1
 mapping[t][source]=chosen;return chosen
def map_owned(kind,value):
 value=number(value)
 if not value:return 0
 if value not in mapping[kind]:mapping[kind][value]=alloc[kind];alloc[kind]+=1
 return mapping[kind][value]
def valnum(x):return number(x.strip("'"))
def rewrite_rows(t,cols,rows):
 out=[]
 for vals in rows:
  row=dict(zip(cols,vals));low={k.lower():k for k in cols}
  def get(k,default='0'):return row.get(low.get(k.lower(),''),default)
  def put(k,v):row[low[k.lower()]]=str(v)
  if t in ('creature','gameobject') and 'guid' in low:
   source=valnum(get('guid'));expected=[str(source),str(valnum(get('id'))),str(valnum(get('map'))),*[str(float(get(k))) for k in ('position_x','position_y','position_z')],str(valnum(get('phaseid'))),str(valnum(get('phasegroup'))),str(valnum(get('phaseMask','1')))]
   put('guid',map_spawn(t,source,expected))
  elif t in ('creature_addon','gameobject_addon','pool_creature','pool_gameobject','game_event_creature','game_event_gameobject') and 'guid' in low:
   put('guid',map_spawn('gameobject' if 'gameobject' in t else 'creature',valnum(get('guid'))))
  if t=='linked_respawn':
   typ=valnum(get('linkType'));assert typ==0,'unmapped non-creature linked_respawn';put('guid',map_spawn('creature',valnum(get('guid'))));put('linkedGuid',map_spawn('creature',valnum(get('linkedGuid'))))
  if t=='waypoint_data':put('id',map_owned('waypoint',get('id')))
  if t in ('creature_addon','creature_template_addon') and 'path_id' in low:put('path_id',map_owned('waypoint',get('path_id')))
  if t=='pool_template':put('entry',map_owned('pool',get('entry')))
  if t.startswith('pool_') and 'pool_entry' in low:put('pool_entry',map_owned('pool',get('pool_entry')))
  if t=='smart_scripts':
   entry=valnum(get('entryorguid'));st=valnum(get('source_type'))
   if entry<0:put('entryorguid',-map_spawn('gameobject' if st==1 else 'creature',-entry))
   target=valnum(get('target_type'))
   if target in (10,14) and valnum(get('target_param1')):put('target_param1',map_spawn('creature' if target==10 else 'gameobject',valnum(get('target_param1'))))
   if valnum(get('action_type'))==53:put('action_param2',map_owned('waypoint',get('action_param2')))
   if valnum(get('action_type'))==41 and valnum(get('action_param2'))==0:put('action_param2',get('action_param1'))
  out.append([row[k] for k in cols])
 return out
# Execute adapted published statements only inside the fixture. Every omission is recorded, never silently accepted.
applied=[];skipped=[];template_inputs=collections.defaultdict(set)
for source,cmd in commands:
 cmd=cmd.replace('`npc_rank`','`rank`').replace('`WDBVerified`','`VerifiedBuild`')
 m=re.match(r'(UPDATE|DELETE FROM|INSERT(?: IGNORE)? INTO)\s+`?(\w+)`?',cmd,re.I);op,t=m.groups();op=op.upper()
 # Keyed upserts replace broad delete/reinsert patterns without resetting unmentioned custom fields/rows.
 if op=='DELETE FROM' and t=='smart_scripts':
  cmd=re.sub(r'(entryorguid`?\s*=\s*)-(\d+)',lambda m:m[1]+str(-map_spawn('creature',int(m[2]))),cmd,flags=re.I)
 allowed_delete=(source=='2026_11_06_world_06.sql' and t in ('spell_area','creature_queststarter','creature_questender','gameobject_queststarter')) or t in ('conditions','smart_scripts','quest_objective','quest_poi','quest_poi_points') or (source=='2026_08_26_world_02.sql' and t=='creature_loot_template') or (source=='2026_08_27_world_00.sql' and t in ('spell_script_names','spell_target_position')) or (source=='2026_08_24_world_03.sql' and t=='creature_template_addon')
 if op=='DELETE FROM' and not allowed_delete:
  skipped.append(dict(source=source,reason='retain local keys; incoming rows use scoped upserts',statement=cmd));continue
 ins=re.match(r'INSERT(?: IGNORE)? INTO\s+`?(\w+)`?\s*\((.*?)\)\s*VALUES\s*(.*)',cmd,re.I|re.S)
 if ins:
  cols=[x.strip(' `\r\n\t') for x in comma_values(ins[2])];rows=[comma_values(x.strip()[1:-1]) for x in comma_values(ins[3])]
  rows=rewrite_rows(t,cols,rows)
  if t=='creature_template':
   for row in rows:template_inputs[valnum(row[cols.index('entry')])].update(x.lower() for x in cols)
  if t=='creature_text':
   groups=collections.defaultdict(set)
   for row in rows:groups[valnum(row[cols.index('entry')])].add(valnum(row[cols.index('groupid')]))
   for entry,gids in groups.items():applied.append(dict(source=source,statement=f'DELETE FROM creature_text WHERE entry={entry} AND groupid IN ('+','.join(map(str,sorted(gids)))+')'))
  if t=='creature_template':cols=['rank' if x=='npc_rank' else 'VerifiedBuild' if x=='WDBVerified' else x for x in cols]
  if t=='spell_script_names':
   # Current spell-module bindings own these names; dungeon-only names are retained.
   active={n for p in (ROOT/'doc').glob('sf27-*-native-port.json') for n in json.loads(p.read_text(encoding='utf-8'))['upstream_names']}
   rows=[r for r in rows if r[cols.index('ScriptName')].strip("'") not in active]
  if not rows:continue
  actual={c.lower():c for c in original_columns[t]};cols=[actual.get(c.lower(),c) for c in cols]
  nonkeys=[c for c in cols if c not in keys[t]]
  cmd='INSERT '+('' if nonkeys else 'IGNORE ')+'INTO `'+t+'` ('+','.join('`'+x+'`' for x in cols)+') VALUES '+','.join('('+','.join(row)+')' for row in rows)
  if nonkeys:cmd+=' ON DUPLICATE KEY UPDATE '+','.join('`'+c+'`=VALUES(`'+c+'`)' for c in nonkeys)
 elif op=='UPDATE' and t=='quest_template':
  body,where=split_where(re.sub(r'^UPDATE\s+`?quest_template`?\s+SET\s+','',cmd,flags=re.I));groups=collections.defaultdict(list)
  for assignment in comma_values(body):
   mm=re.fullmatch(r'`?(\w+)`?\s*=\s*(.*)',assignment,re.S);assert mm,assignment;old,value=mm.groups()
   groups[old].append(value)
  # Aliases in this fixture emulate the old table exactly. Missing alias columns fail visibly.
 elif op=='UPDATE' and t in ('creature','gameobject') and re.search(r'WHERE\s+`?guid`?',cmd,re.I):
  body,where=split_where(re.sub(r'^UPDATE\s+`?'+t+r'`?\s+SET\s+','',cmd,flags=re.I))
  guard=''
  if not re.search(r'\bBETWEEN\b',where,re.I):
   parts=re.split(r'\s+AND\s+',where,maxsplit=1,flags=re.I);where=parts[0].strip();guard=parts[1] if len(parts)>1 else ''
  match=re.fullmatch(r'`?guid`?\s*(?:=\s*([0-9+\- ()]+)|IN\s*\((.*?)\)|BETWEEN\s+(.+?)\s+AND\s+(.+))',where,re.I|re.S);assert match,(source,where)
  if match[1]:ids=[number(match[1])]
  elif match[2]:ids=[number(x) for x in comma_values(match[2])]
  else:ids=list(range(number(match[3]),number(match[4])+1))
  omitted=[i for i in ids if i not in mapping[t] and i not in reference[t]]
  if omitted:notes.append(dict(source=source,unused_range_guids=omitted))
  ids=[i for i in ids if i in mapping[t] or i in reference[t]]
  if not ids:continue
  for i in ids:
   if i not in mapping[t] and i in reference[t]:
    expected=reference[t][i]
    candidates=[x for x in spatial[t].get((int(expected[1]),int(expected[2])),[]) if sum((float(x[j])-float(expected[j]))**2 for j in (3,4,5))<4]
    if candidates:mapping[t][i]=int(min(candidates,key=lambda x:sum((float(x[j])-float(expected[j]))**2 for j in (3,4,5)))[0])
  cmd='UPDATE `'+t+'` SET '+body+' WHERE guid IN ('+','.join(str(map_spawn(t,i)) for i in ids)+')'+(' AND '+guard if guard else '')
 if op=='UPDATE' and t=='creature_template':
  body,where=split_where(re.sub(r'^UPDATE\s+`?creature_template`?\s+SET\s+','',cmd,flags=re.I))
  assigned={re.match(r'`?(\w+)',a)[1].lower() for a in comma_values(body)}
  if assigned & {'modelid1','modelid2','modelid3','modelid4','inhabittype'}:
   for line in q('SELECT entry FROM creature_template WHERE '+where+';').splitlines():template_inputs[int(line)].update(assigned)
 try:applied.append(dict(source=source,statement=cmd))
 except Exception as exc:
  (ROOT/'out/sf27-content-compile-error.json').write_text(json.dumps(dict(source=source,statement=cmd,error=str(exc)),indent=2),encoding='utf-8');raise
q('ALTER TABLE gameobject AUTO_INCREMENT='+str(alloc['gameobject']+1000)+';')
q('\n'.join(x['statement']+';' for x in applied))
# Local gossip loader consumes option_id; retain its compatibility alias too.
q('UPDATE gossip_menu_option SET option_id=1,OptionType=1 WHERE menu_id=10410 AND id=0;')

# Mirror old quest fields into active split storage without creating unrelated empty rows.
for old,(t,new) in fields.items():
 if t=='quest_template':q(f'UPDATE quest_template SET `{new}`=`{old}` WHERE NOT (`{new}` <=> `{old}`);')
 else:
  default="''" if old.endswith('Text') else '0'
  q(f'INSERT INTO `{t}` (ID,`{new}`) SELECT qt.ID,qt.`{old}` FROM quest_template qt LEFT JOIN `{t}` dst ON dst.ID=qt.ID WHERE NOT (qt.`{old}` <=> COALESCE(dst.`{new}`,{default})) ON DUPLICATE KEY UPDATE `{new}`=VALUES(`{new}`);')
for col in ('flags','faction'):
 q(f'INSERT INTO gameobject_template_addon (entry,`{col}`) SELECT g.entry,g.`{col}` FROM gameobject_template g LEFT JOIN gameobject_template_addon a ON a.entry=g.entry WHERE NOT (g.`{col}` <=> COALESCE(a.`{col}`,0)) ON DUPLICATE KEY UPDATE `{col}`=VALUES(`{col}`);')
q('UPDATE creature_template SET faction=faction_A WHERE faction_A=faction_H AND faction<>faction_A;')
q('UPDATE creature SET wander_distance=spawndist WHERE wander_distance<>spawndist;')
def decode(value):return None if value=='N' else bytes.fromhex(value[1:]).decode('utf-8')
def literal(value):return 'NULL' if value=='N' else "CONVERT(X'"+value[1:]+"' USING utf8mb4)"
def where_key(t,row):return ' AND '.join('`'+c+'` <=> '+literal(row[c]) for c in keys[t])
# Source text/POI/addon inserts use legacy columns; only source-changed rows receive active aliases.
mirrors={'creature_text':{'entry':'CreatureID'},'quest_poi':{'id':'Idx1','objIndex':'ObjectiveIndex','FloorId':'Floor','unk3':'Priority','unk4':'Flags'},'quest_poi_points':{'id':'Idx1','idx':'Idx2'}}
synchronize=[]
for t,pairs in mirrors.items():
 if t not in tables:continue
 for key,row in snapshot(t).items():
  if before[t].get(key)==row:continue
  assignments=['`'+new+'`='+literal(row[old]) for old,new in pairs.items() if old in row and new in row]
  if assignments:synchronize.append('UPDATE `'+t+'` SET '+','.join(assignments)+' WHERE '+where_key(t,row)+';')
for t in ('creature_addon','creature_template_addon'):
 for key,row in snapshot(t).items():
  old=before[t].get(key)
  if old==row:continue
  assignments=[]
  for byte,parts in [('bytes1',[('StandState',0),('VisFlags',16),('AnimTier',24)]),('bytes2',[('SheathState',0),('PvPFlags',8)])]:
   if old and old[byte]==row[byte]:continue
   value=int(decode(row[byte]))
   assignments.extend('`'+col+'`='+str((value>>shift)&255) for col,shift in parts)
  if assignments:synchronize.append('UPDATE `'+t+'` SET '+','.join(assignments)+' WHERE '+where_key(t,row)+';')
if synchronize:q('\n'.join(synchronize))
synchronize=[]
# Source-assigned model slots use the active model table; other slots are preserved.
ct=snapshot('creature_template')
for entry,assigned in template_inputs.items():
 row=ct.get(('H'+str(entry).encode().hex().upper(),))
 assert row,entry
 slots=[i for i in range(4) if 'modelid'+str(i+1) in assigned]
 if slots:
  synchronize.append('DELETE FROM creature_template_model WHERE CreatureID='+str(entry)+' AND Idx IN ('+','.join(map(str,slots))+');')
  models=[];seen=set()
  for i in slots:
   display=int(decode(row['modelid'+str(i+1)]))
   if display and display not in seen:models.append((i,display));seen.add(display)
  if models:synchronize.append('INSERT INTO creature_template_model (CreatureID,Idx,CreatureDisplayID,DisplayScale,Probability,VerifiedBuild) VALUES '+','.join(f'({entry},{i},{display},1,{1/len(models)},0)' for i,display in models)+' ON DUPLICATE KEY UPDATE Idx=VALUES(Idx),Probability=VALUES(Probability);')
 if 'inhabittype' in assigned:
  inhabit=int(decode(row['InhabitType']))
  synchronize.append(f'INSERT INTO creature_template_movement (CreatureId,Ground,Swim,Flight) VALUES ({entry},{1 if inhabit&1 else 0},{1 if inhabit&2 else 0},{1 if inhabit&4 else 0}) ON DUPLICATE KEY UPDATE Ground=VALUES(Ground),Swim=VALUES(Swim),Flight=VALUES(Flight);')
if synchronize:q('\n'.join(synchronize))
# A published deletion removes the ambient worgen; only its verified identity near the summoned encounter is eligible.
q('DELETE FROM creature WHERE guid=192110 AND id=43799 AND map=0 AND POW(position_x+11127.5,2)+POW(position_y+518.424,2)<100;')
# Copy missing canonical path geometry for remapped addon paths that were not in the incoming VALUES batch.
for old,new in mapping['waypoint'].items():
 if not int(q('SELECT COUNT(*) FROM waypoint_data WHERE id='+str(new)+';').strip()):
  cols=[line.split('\t')[0] for line in db.q('SHOW COLUMNS FROM waypoint_data;').splitlines()]
  q('INSERT INTO waypoint_data ('+','.join('`'+c+'`' for c in cols)+') SELECT '+','.join(str(new) if c=='id' else '`'+c+'`' for c in cols)+' FROM '+ref+'.waypoint_data WHERE id='+str(old)+';')
  if not int(q('SELECT COUNT(*) FROM waypoint_data WHERE id='+str(new)+';').strip()):notes.append(dict(unresolved_source_waypoint=old,mapped_waypoint=new))
# Add only canonical spawn dependencies explicitly referenced by imported addons/events/pools.
# Never leave a newly remapped addon pointing at a nonexistent creature.
for kind,links in [('creature',['creature_addon','pool_creature','game_event_creature']),('gameobject',['pool_gameobject','game_event_gameobject'])]:
 missing=set()
 for table in links:
  if table in tables:
   missing.update(int(line) for line in q('SELECT DISTINCT l.guid FROM '+table+' l LEFT JOIN '+kind+' s ON s.guid=l.guid WHERE s.guid IS NULL;').splitlines())
 reverse={new:old for old,new in mapping[kind].items()}
 additions=[]
 refcols={line.split('\t')[0].lower():line.split('\t')[0] for line in db.q('SHOW COLUMNS FROM '+ref+'.'+kind+';').splitlines()}
 for guid in sorted(missing & set(reverse)):
  old=reverse[guid];cols=[];expr=[]
  for c in original_columns[kind]:
   source=refcols.get(c.lower())
   if c.lower()=='guid':cols.append(c);expr.append(str(guid))
   elif c.lower()=='wander_distance' and 'spawndist' in refcols:cols.append(c);expr.append('`'+refcols['spawndist']+'`')
   elif source:cols.append(c);expr.append('`'+source+'`')
  additions.append('INSERT INTO '+kind+' ('+','.join('`'+c+'`' for c in cols)+') SELECT '+','.join(expr)+' FROM '+ref+'.'+kind+' WHERE guid='+str(old)+';')
  notes.append(dict(canonical_spawn_dependency=kind,source_guid=old,mapped_guid=guid))
 if additions:q('\n'.join(additions))
# The heroic source file contains explicit item drops with dangling reference IDs50021-50048.
# Neither the pinned public package nor this fork defines those groups. Keep each authored
# item/chance/maxcount and restore a one-item minimum instead of inventing a reference pool.
repairs=[]
canonical_references={int(x) for x in db.q('SELECT DISTINCT entry FROM '+ref+'.reference_loot_template;').splitlines()}
valid_items={int(x) for x in db.q('SELECT entry FROM item_template;').splitlines()}
for t in sorted(x for x in tables if x.endswith('_loot_template')):
 for key,row in snapshot(t).items():
  if before[t].get(key)==row:continue
  minimum=int(decode(row['mincountOrRef']))
  if minimum>=0:continue
  reference_id=-minimum
  if reference_id in canonical_references:
   raise RuntimeError('New canonical loot-reference dependency requires an isolated group mapping: '+str(reference_id))
  item=int(decode(row['item']))
  assert item>0 and int(decode(row['maxcount']))==1
  assert item in valid_items
  repairs.append('UPDATE `'+t+'` SET mincountOrRef=1 WHERE '+where_key(t,row)+';')
  notes.append(dict(source_defect='explicit item drop had undefined loot reference',table=t,entry=decode(row['entry']),item=item,missing_reference=reference_id,minimum_count=1))
if repairs:q('\n'.join(repairs))
# Export scoped active-schema changes. No compatibility-only columns leave the fixture.
output=['-- Remaining SF27 published content, adapted at a69eda918b5bd416078fd8d832560d0fd4fb9023.', '-- Requires the matched native modules and SQL44-47. Back up the listed public world tables first.']
changes=[];counts={}
for t in sorted(tables):
 after=snapshot(t);oldrows=before[t];added=updated=removed=0
 for key,oldrow in oldrows.items():
  if key not in after:
   output.append('DELETE FROM `'+t+'` WHERE '+where_key(t,oldrow)+';');changes.append(dict(table=t,operation='delete',before=oldrow));removed+=1
 for key,row in after.items():
  oldrow=oldrows.get(key)
  if oldrow==row:continue
  if oldrow is None:
   cols=list(row);output.append('INSERT INTO `'+t+'` ('+','.join('`'+c+'`' for c in cols)+') SELECT '+','.join(literal(row[c]) for c in cols)+' WHERE NOT EXISTS (SELECT 1 FROM `'+t+'` WHERE '+where_key(t,row)+');');added+=1
  else:
   changed=[c for c in row if row[c]!=oldrow[c]]
   output.append('UPDATE `'+t+'` SET '+','.join('`'+c+'`='+literal(row[c]) for c in changed)+' WHERE '+where_key(t,oldrow)+';');updated+=1
  changes.append(dict(table=t,operation='insert' if oldrow is None else 'update',keys=keys[t],before=oldrow,after=row))
 counts[t]=dict(inserted=added,updated=updated,removed=removed)
 if before[t] is not ct:before[t]={} # Release the large baseline progressively.
path=ROOT/'sql/updates/world/2026_11_06_48_world_sf27_remaining_content.sql';path.write_text('\n'.join(output)+'\n',encoding='utf-8')
report=dict(fixture=fixture,pin='a69eda918b5bd416078fd8d832560d0fd4fb9023',sources=sources,applied_source_statements=len(applied),preservation_adaptations=skipped,notes=notes,mapping=mapping,tables=sorted(tables),counts=counts,changes=changes,sql_sha256=hashlib.sha256(path.read_bytes()).hexdigest(),status='prepared; not deployed; requires dependency and repeat validation')
(ROOT/'doc/sf27-remaining-content-data.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
print(json.dumps(dict(source_files=len(sources),source_statements=len(applied),scoped_statements=len(output)-2,counts=counts,note_count=len(notes))))
