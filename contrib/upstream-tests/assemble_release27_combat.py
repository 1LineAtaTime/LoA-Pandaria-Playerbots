"""Assemble the pinned SF27 melee baseline without replacing existing tables.

The source directory must be exported from UPSTREAM. No database writes occur.
--names is a two-column entry/name TSV used solely to exclude repurposed IDs.
"""
import argparse,hashlib,json,pathlib,re
from sql_batch import statements,comma_values
UPSTREAM='df19329b209fa1d1ae80444b43f6201d5dc124f0'
FILES=['2026_08_17_world_00.sql','2026_08_17_world_03.sql','2026_08_17_world_06.sql']
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in ('patches','names','output','ledger'):p.add_argument('--'+n,type=pathlib.Path,required=True)
 a=p.parse_args();names={int(x.split('\t',1)[0]):x.split('\t',1)[1] for x in a.names.read_text(encoding='utf-8').splitlines()}
 texts={n:(a.patches/n).read_text(encoding='utf-8-sig') for n in FILES};stats={};npcs={}
 insert=next(s for s in statements(texts[FILES[0]]) if s.startswith('INSERT INTO'))
 m=re.fullmatch(r'INSERT INTO `creature_classlevelstats`\s*\((.*?)\)\s*VALUES\s*(.*)',insert,re.S);assert m
 cols=[x.strip(' `') for x in comma_values(m[1])]
 for row in comma_values(m[2]):
  vals=comma_values(row.strip()[1:-1]);stats[tuple(vals[:2])]=dict(zip(cols,vals))
 for s in statements(texts[FILES[2]]):
  m=re.fullmatch(r'UPDATE `creature_classlevelstats` SET `damage_exp4`=([\d.]+) WHERE `level`=(\d+) AND `class`=(\d+)',s)
  if m:stats[m[2],m[3]]['damage_exp4']=m[1]
 for n in FILES[1:]:
  for line in texts[n].splitlines():
   if not line.startswith('UPDATE `creature_template`'):continue
   m=re.fullmatch(r'UPDATE `creature_template` SET (.*?) WHERE `entry`=(\d+); -- (.*)',line);assert m,line
   values={k:v for k,v in re.findall(r'`(\w+)`=([\d.]+)',m[1])}
   assert set(values)=={'mindmg','maxdmg','attackpower','dmg_multiplier'}
   npcs[int(m[2])]=dict(name=m[3],source=n,values={k:values[k] for k in ('mindmg','maxdmg','attackpower')})
 skipped=[dict(entry=i,upstream_name=v['name'],local_name=names.get(i),reason='missing or different NPC identity') for i,v in npcs.items() if names.get(i)!=v['name']]
 npcs={i:v for i,v in npcs.items() if names.get(i)==v['name']}
 out=['-- SF27 melee baseline adapted to the LoA schema. SkyFire contributors, GPL-3.0.',f'-- Upstream {UPSTREAM}; exact input hashes and deferred multiplier policy in paired ledger.', '-- Retain unrelated template fields, extra class/level rows and all custom character/shop data.']
 out.append('INSERT INTO `creature_classlevelstats` ('+','.join('`'+c+'`' for c in cols)+') VALUES\n'+',\n'.join('('+','.join(v[c] for c in cols)+')' for v in stats.values())+'\nON DUPLICATE KEY UPDATE '+','.join('`'+c+'`=VALUES(`'+c+'`)' for c in cols[2:])+';')
 for i,v in sorted(npcs.items()):
  name=v['name'].replace('\\','\\\\').replace("'","''")
  out.append('UPDATE `creature_template` SET '+','.join('`'+k+'`='+x for k,x in v['values'].items())+f" WHERE `entry`={i} AND BINARY `name`=BINARY '{name}';")
 a.output.write_text('\n\n'.join(out)+'\n',encoding='utf-8')
 ledger=dict(upstream=UPSTREAM,release='27.001',source_root='sql/old/5.4.8/world/SFDB_release_26.001_to_27.001',sources=[dict(file=n,sha256=hashlib.sha256((a.patches/n).read_bytes()).hexdigest()) for n in FILES],class_stats=list(stats.values()),creatures=[dict(entry=i,**v) for i,v in sorted(npcs.items())],skipped=skipped,policy='Port authored melee ranges/AP and class/level curves. Preserve LoA BG/AutoBalance/ranged policies. Upstream per-template damage multipliers remain deferred: LoA uses its existing runtime/event multiplier and rank scaling. No DROP, blanket DELETE, account or character mutation.',migration_sha256=hashlib.sha256(a.output.read_bytes()).hexdigest())
 a.ledger.write_text(json.dumps(ledger,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
 print(json.dumps(dict(class_stats=len(stats),creatures=len(npcs),skipped=len(skipped),output=str(a.output))))
if __name__=='__main__':main()
