"""Apply the SF27 typed loot/credit migration twice in a public-content fixture only."""
import pathlib,sys,json,re,hashlib
ROOT=pathlib.Path(__file__).resolve().parents[2]
from sf27_db import load_db
db=load_db()
report=json.loads((ROOT/'doc/sf27-typed-loot-data.json').read_text())
sql=(ROOT/'sql/updates/world/2026_11_06_46_world_sf27_typed_loot_credit_spawns.sql').read_text()
fixture='codex_sf27_typed_loot_fixture_20261106'
tables=report['tables'];live=db.database
setup='CREATE DATABASE IF NOT EXISTS '+fixture+';\n'
for t in tables:setup+=f'DROP TABLE IF EXISTS {fixture}.`{t}`; CREATE TABLE {fixture}.`{t}` LIKE `{live}`.`{t}`; INSERT INTO {fixture}.`{t}` SELECT * FROM `{live}`.`{t}`;\n'
db.q(setup,timeout=180)
db.q(f'CREATE OR REPLACE VIEW {fixture}.gameobject_template AS SELECT entry,type,data1 FROM `{live}`.gameobject_template;')

def run(q):
 r=db.q('USE '+fixture+';\n'+q+'\nSELECT \'__completed__\';',timeout=180)
 assert r.rstrip().endswith('__completed__'), 'incomplete SQL response'
 return r.rsplit('__completed__',1)[0].strip()
run(sql)
def hashes():
 return run('\n'.join(f"SELECT '{t}',COALESCE(BIT_XOR(CRC32(CONCAT_WS('|',"+','.join('COALESCE(HEX(`'+c.split('\t')[0]+"`),'NULL')" for c in db.q('SHOW COLUMNS FROM '+t+';').splitlines())+f"))),0),COUNT(*) FROM `{t}`;" for t in tables))
first=hashes();run(sql);assert first==hashes(),'migration not repeatable'
checks=[]
for index,row in enumerate(report['desired']):
 where=' AND '.join('`'+k+'` <=> '+row['values'][k] for k in row['keys']) or '1'
 predicates=[]
 for k,v in row['values'].items():
  if re.fullmatch(r'-?\d+\.\d+',v):predicates.append('ABS(`'+k+'` - '+v+') < 0.001')
  else:predicates.append('`'+k+'` <=> '+v)
 checks.append(f"SELECT 'row{index}',COUNT(*) FROM `{row['table']}` WHERE {where} AND "+' AND '.join(predicates)+';')
result=run('\n'.join(checks));failed=[x for x in result.splitlines() if x.split('\t')[-1]!='1'];assert not failed,failed[:15]
print(json.dumps(dict(fixture=fixture,checks=len(checks),repeatable=True,table_checksums=first.splitlines())))
