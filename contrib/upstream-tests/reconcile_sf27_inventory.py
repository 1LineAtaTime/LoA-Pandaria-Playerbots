"""Reconcile published SQL inputs with explicit adapted/equivalent dispositions."""
from pathlib import Path
import csv,json,argparse,subprocess,hashlib
ROOT=Path(__file__).resolve().parents[2]
parser=argparse.ArgumentParser();parser.add_argument('--deployed',action='store_true');args=parser.parse_args()
path=ROOT/'doc/skyfire27-integration-inventory-2026-11-06.csv'
rows=list(csv.DictReader(path.open(encoding='utf-8-sig')))
modules={}
for name,batch in [('sf27-native-spell-data.json','SQL44'),('sf27-interaction-data.json','SQL45'),('sf27-typed-loot-data.json','SQL46'),('sf27-remaining-content-data.json','SQL48'),('sf27-authored-damage-data.json','SQL49')]:
 d=json.loads((ROOT/'doc'/name).read_text())
 for s in d['sources']:modules.setdefault(s.get('file',s.get('patch')),[]).append(batch)
for s in json.loads((ROOT/'doc/release27-services-ledger-2026-11-06.json').read_text())['sources']:modules.setdefault(s['file'],[]).append('SQL40')
world=json.loads((ROOT/'doc/release27-world-ledger-2026-11-06.json').read_text())
for s in world['sources']:
 if s['selected_statements']:modules.setdefault(s['file'],[]).append('SQL42')
modules.setdefault('2026_08_17_world_00.sql',[]).append('SQL41')
modules['2026_08_17_world_04.sql']=['SQL47']
equivalent={
 '2026_08_20_world_00.sql':'SQL42 updates the active lfg_dungeon_template; unused legacy lfg_entrances mirror is not recreated.',
 '2026_08_20_world_05.sql':'SQL42 keyed ooze loot/model changes preserve unrelated local rows instead of broad deletion.',
 '2026_08_22_world_00.sql':'Active quest_template.AllowableRaces already uses unsigned 32-bit storage; RequiredRaces is a legacy field.',
 '2026_08_26_world_00.sql':'SQL44 and the native hunter module own the canonical Stampede/Glaive Toss bindings.',
 '2026_08_26_world_04.sql':'SQL42 provides pouch loot through keyed rows; unrelated custom loot retained.',
 '2026_08_26_world_06.sql':'SQL42 provides Timberling loot/harvesting flags; unrelated loot retained.',
 '2026_09_08_world_00.sql':'Not applicable: published help requires an email-only account-creation signature. Preserve this fork\'s username-compatible authentication and matching command help.',
 '2026_09_21_world_12.sql':'SQL44 owns the current canonical Drain Life binding and native implementation.',
 '2026_10_04_world_01.sql':'SQL42 adds canonical riding turn-in links without deleting unrelated links.',
 '2026_10_04_world_03.sql':'SQL42 adds canonical missing turn-in links without deleting unrelated links.',
 '2026_10_25_world_15.sql':'Equivalent: this core learns starter spells through DBC skill/race/class/level gates, not the absent playercreateinfo_spell table; do not recreate that table.'}
pin='a69eda918b5bd416078fd8d832560d0fd4fb9023'
existing={Path(r['path']).name for r in rows}
for name in ['2026_11_06_world_06.sql','2026_11_06_world_07.sql']:
 if name not in existing:
  source='sql/updates/world/'+name
  raw=subprocess.check_output(['git','show',pin+':'+source],cwd=ROOT)
  rows.append(dict(path=source,sha256=hashlib.sha256(raw).hexdigest(),bytes=len(raw),title='Latest Teldrassil content and quest repair' if name.endswith('06.sql') else 'Clear NPC-hiding creature type flag',module='world quests and interactions',status=''))
dispositions=[]
for row in rows:
 name=Path(row['path']).name;batches=modules.get(name,[])
 assert batches or name in equivalent,'Unaccounted source: '+name
 if name in equivalent:status=equivalent[name]
 else:status='Adapted in '+', '.join(sorted(set(batches)))+'; custom unmentioned fields/identities retained.'
 row['status']=('deployed/reconciled: ' if args.deployed else 'prepared/reconciled: ')+status
 dispositions.append(dict(file=name,batches=sorted(set(batches)),disposition=status))
with path.open('w',encoding='utf-8',newline='') as f:
 writer=csv.DictWriter(f,fieldnames=['path','sha256','bytes','title','module','status']);writer.writeheader();writer.writerows(rows)
report=dict(upstream_pin=pin,published_world_sql_inputs=len(rows),deployed=args.deployed,scope='Release26-to27 published world SQL plus subsequent updates through the pin; not an assertion of whole-tree equivalence or exhaustive gameplay.',sources=dispositions)
(ROOT/'doc/sf27-final-source-dispositions.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(dict(inputs=len(rows),deployed=args.deployed,unaccounted=0)))
