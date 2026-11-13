"""Extend the verified combined fixture with startup-discovered vehicle/pool dependencies."""
from pathlib import Path
import json,hashlib
from sf27_db import load_db
ROOT=Path(__file__).resolve().parents[2];db=load_db()
path=ROOT/'out/sf27-final-fixture-results.json';report=json.loads(path.read_text());fixture=report['fixture']
for name,digest in report['patches'].items():assert hashlib.sha256((ROOT/'sql/updates/world'/name).read_bytes()).hexdigest()==digest
db.q(f'CREATE OR REPLACE VIEW {fixture}.pool_pool AS SELECT * FROM `{db.database}`.pool_pool;')
patch=ROOT/'sql/updates/world/2026_11_06_50_world_sf27_vehicle_pool_dependencies.sql'
def run(sql):return db.q('USE '+fixture+';\n'+sql,timeout=300)
run(patch.read_text())
query="SELECT npc_entry,spell_id,cast_flags,user_type FROM npc_spellclick_spells ORDER BY npc_entry,spell_id; SELECT entry,max_limit,description FROM pool_template ORDER BY entry;"
first=run(query);run(patch.read_text());assert first==run(query)
extra="""SELECT 'heroic vehicle clicks',COUNT(*),3 FROM npc_spellclick_spells WHERE npc_entry IN (48803,48804,48913) AND spell_id=46598 AND cast_flags=1;
SELECT 'unused pool roots',COUNT(*),0 FROM pool_template WHERE entry IN (173,14176);
"""
verification=ROOT/'out/sf27-final-verification.sql';sql=verification.read_text();sql=sql if extra.strip() in sql else sql+extra
rows=run(sql).splitlines();bad=[x for x in rows if x.split('\t')[-2]!=x.split('\t')[-1]];assert not bad,bad[:15]
report['table_fingerprints_scope']='Before SQL50; all value checks and SQL50 repeat stability verified afterward'
report['checks']=len(rows);report['patches'][patch.name]=hashlib.sha256(patch.read_bytes()).hexdigest();report['startup_dependency_checks']=2
path.write_text(json.dumps(report,indent=2)+'\n');verification.write_text(sql)
print(json.dumps(dict(checks=len(rows),repeatable=True,dependency_patch=patch.name)))
