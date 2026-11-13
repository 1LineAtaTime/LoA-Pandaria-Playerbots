"""Exercise SQL46-49 together on public world tables, never account/player data."""
from pathlib import Path
import hashlib,json,re
from sf27_db import load_db
ROOT=Path(__file__).resolve().parents[2]
db=load_db(); live=db.database
fixture='codex_sf27_final_fixture_20261106'
content=json.loads((ROOT/'doc/sf27-remaining-content-data.json').read_text())
loot=json.loads((ROOT/'doc/sf27-typed-loot-data.json').read_text())
damage=json.loads((ROOT/'doc/sf27-authored-damage-data.json').read_text())
tables=sorted(set(content['tables'])|set(loot['tables']))
assert not set(tables)&{'account','characters','mail','mail_items'}
db.q('CREATE DATABASE IF NOT EXISTS '+fixture+';\n'+'\n'.join(f'DROP TABLE IF EXISTS {fixture}.`{t}`; CREATE TABLE {fixture}.`{t}` LIKE `{live}`.`{t}`; INSERT INTO {fixture}.`{t}` SELECT * FROM `{live}`.`{t}`;' for t in tables),timeout=300)
def run(sql):return db.q('USE '+fixture+';\n'+sql,timeout=300)
db.q(f'CREATE OR REPLACE VIEW {fixture}.pool_pool AS SELECT * FROM `{live}`.pool_pool;')
patches=[]
for number,meta in [(46,loot),(48,content),(47,None),(49,damage),(50,None)]:
 p=next((ROOT/'sql/updates/world').glob(f'2026_11_06_{number}_*.sql'))
 if meta:assert hashlib.sha256(p.read_bytes()).hexdigest()==meta['sql_sha256']
 patches.append(p)
 run(p.read_text(encoding='utf-8'))
print('Applied four migrations to public-content fixture',flush=True)
def fingerprints():
 statements=[]
 for t in tables:
  cols=[x.split('\t')[0] for x in db.q('SHOW COLUMNS FROM '+fixture+'.`'+t+'`;').splitlines()]
  values=','.join('COALESCE(HEX(`'+c+"`),'NULL')" for c in cols)
  statements.append(f"SELECT '{t}',COUNT(*),COALESCE(BIT_XOR(CRC32(CONCAT_WS('|',{values}))),0) FROM `{t}`;")
 return run('\n'.join(statements))
first=fingerprints()
for p in patches:run(p.read_text(encoding='utf-8'))
assert first==fingerprints(),'combined migrations changed data on second application'
def literal(v):return 'NULL' if v=='N' else "CONVERT(X'"+v[1:]+"' USING utf8mb4)"
types={}
for t in tables:types[t]={x.split('\t')[0]:x.split('\t')[1] for x in db.q('SHOW COLUMNS FROM '+fixture+'.`'+t+'`;').splitlines()}
def equal(t,c,v):
 if v!='N' and any(k in types[t][c].lower() for k in ('float','double')):return 'ABS(`'+c+'` - '+literal(v)+') <= GREATEST(0.00001,ABS('+literal(v)+')*0.000001)'
 return '`'+c+'` <=> '+literal(v)
checks=[]
for n,change in enumerate(content['changes']):
 before=change.get('before');after=change.get('after');row=after or before
 keys=change.get('keys') or [x.split('\t')[0] for x in db.q('SHOW COLUMNS FROM '+fixture+'.`'+change['table']+'`;').splitlines() if x.split('\t')[3]=='PRI'] or list(row)
 where=' AND '.join('`'+c+'` <=> '+literal(row[c]) for c in keys)
 if after:
  cols=list(after) if before is None else [c for c in after if after[c]!=before[c]]
  where+=' AND '+' AND '.join(equal(change['table'],c,after[c]) for c in cols)
 checks.append(f"SELECT 'content{n}',COUNT(*),{0 if after is None else 1} FROM `{change['table']}` WHERE {where};")
# Published values must survive the combined batch, not just each migration alone.
for n,row in enumerate(loot['desired']):
 pred=[]
 for k,v in row['values'].items():
  pred.append('ABS(`'+k+'` - '+v+') < 0.001' if re.fullmatch(r'-?\d+\.\d+',v) else '`'+k+'` <=> '+v)
 checks.append(f"SELECT 'typed{n}',COUNT(*),1 FROM `{row['table']}` WHERE "+' AND '.join(pred)+';')
checks.append("SELECT 'detection',COUNT(*),0 FROM creature_template WHERE detection_range <> IF(maxlevel<=6,10,20);")
for n,row in enumerate(damage['values']):
 entry=row['entry'];value=row.get('multiplier',row.get('value'))
 if value is None:raise ValueError(row)
 checks.append(f"SELECT 'damage{n}',COUNT(*),1 FROM creature_template WHERE entry={entry} AND ABS(sf27_damage_multiplier-{value})<0.0001;")
# Verify imported records have real dependencies, independently of their value ledger.
def decode(v):return bytes.fromhex(v[1:]).decode() if v!='N' else None
for t in ['creature','gameobject','creature_addon','game_event_creature','game_event_gameobject','pool_gameobject','pool_creature']:
 added=[c['after'] for c in content['changes'] if c['table']==t and c['operation']=='insert']
 if not added:continue
 ids=','.join(decode(row['guid']) for row in added)
 if t in ['creature','gameobject']:
  checks.append(f"SELECT '{t} template dependencies',COUNT(*),0 FROM `{t}` s LEFT JOIN `{t}_template` p ON p.entry=s.id WHERE s.guid IN ({ids}) AND p.entry IS NULL;")
 else:
  target='creature' if t in ['creature_addon','game_event_creature','pool_creature'] else 'gameobject'
  checks.append(f"SELECT '{t} spawn dependencies',COUNT(*),0 FROM `{t}` s LEFT JOIN `{target}` p ON p.guid=s.guid WHERE s.guid IN ({ids}) AND p.guid IS NULL;")
for t in ['creature_loot_template','pickpocketing_loot_template','skinning_loot_template']:
 entries=sorted({decode(c['after']['entry']) for c in content['changes'] if c['table']==t and c['operation']=='insert'})
 if entries:checks.append(f"SELECT '{t} items',COUNT(*),0 FROM `{t}` l LEFT JOIN `{live}`.item_template i ON i.entry=l.item WHERE l.entry IN ("+','.join(entries)+") AND l.mincountOrRef>=0 AND i.entry IS NULL;")
checks.extend(["SELECT 'heroic vehicle clicks',COUNT(*),3 FROM npc_spellclick_spells WHERE npc_entry IN (48803,48804,48913) AND spell_id=46598 AND cast_flags=1;","SELECT 'unused pool roots',COUNT(*),0 FROM pool_template WHERE entry IN (173,14176);"])
(ROOT/'out/sf27-final-verification.sql').write_text('\n'.join(checks)+'\n')
results=run('\n'.join(checks));bad=[x for x in results.splitlines() if x.split('\t')[-2]!=x.split('\t')[-1]]
assert not bad,bad[:30]
report=dict(fixture=fixture,checks=len(checks),repeatable=True,tables=tables,table_fingerprints=first.splitlines(),patches={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in patches})
(ROOT/'out/sf27-final-fixture-results.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(dict(fixture=fixture,checks=len(checks),repeatable=True,tables=len(tables))))
