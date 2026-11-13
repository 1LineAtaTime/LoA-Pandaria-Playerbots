"""Check trusted SQL36/37/38 in disposable world-table copies; no live mutation."""
import pathlib,argparse,re,subprocess,os,secrets,json,hashlib,collections,sys
from sql_batch import statements,comma_values
from assemble_release26_quest_batch import split_where

def main():
 ap=argparse.ArgumentParser(description=__doc__)
 for k in ('config','client','report'):ap.add_argument('--'+k,type=pathlib.Path,required=True)
 a=ap.parse_args();root=pathlib.Path(__file__).resolve().parents[2];m=re.search(r'^WorldDatabaseInfo\s*=\s*"([^"]+)"',a.config.read_text(encoding='utf-8-sig'),re.M);host,port,user,password,source=m[1].split(';')
 cmd=[str(a.client.resolve()),'--no-defaults','--protocol=TCP','--host='+host,'--port='+port,'--user='+user,'--database='+source,'--batch','--skip-column-names','--default-character-set=utf8mb4']
 def q(sql):
  p=subprocess.run(cmd,input=sql,text=True,encoding='utf-8',capture_output=True,timeout=180,env=dict(os.environ,MYSQL_PWD=password))
  if p.returncode:raise RuntimeError(p.stderr.replace(password,'[REDACTED]')[:1600])
  return p.stdout
 names=['2026_11_06_38_world_release26_loot_references.sql','2026_11_06_36_world_release26_loot_services.sql','2026_11_06_37_world_release26_class_spells.sql','2026_11_06_39_world_release26_spare_parts_alignment.sql'];sql='\n'.join((root/'sql/updates/world'/n).read_text(encoding='utf-8') for n in names);commands=statements(sql);tables=sorted(set(re.findall(r'(?m)^(?:UPDATE|INSERT(?: IGNORE)? INTO|DELETE FROM)\s+`?(\w+)`?',sql,re.I)))
 if 'gameobject_template' not in tables:tables.append('gameobject_template')
 schema='codex_services_'+secrets.token_hex(5)
 def w(sql):return q('USE `'+schema+'`;'+sql)
 report=dict(passed=False,gameplay_tested=False,tables=tables,migrations=names);q('CREATE DATABASE `'+schema+'`;')
 try:
  for t in tables:q(f'CREATE TABLE `{schema}`.`{t}` LIKE `{source}`.`{t}`; INSERT INTO `{schema}`.`{t}` SELECT * FROM `{source}`.`{t}`;')
  cols={};types={};keys={};allowed=collections.defaultdict(set);replace=set()
  for t in tables:
   info=[x.split('\t') for x in w(f'SHOW COLUMNS FROM `{t}`;').splitlines()];cols[t]=[r[0] for r in info];types[t]={r[0].lower():r[1] for r in info};keys[t]=[x.split('\t')[4] for x in w(f'SHOW KEYS FROM `{t}` WHERE Key_name="PRIMARY";').splitlines()]
   if not keys[t]:
    unique=[x.split('\t') for x in w(f'SHOW KEYS FROM `{t}` WHERE Non_unique=0;').splitlines()];assert unique,t
    keys[t]=[x[4] for x in unique if x[2]==unique[0][2]]
  def expr(t,k,s=None):
   s='`'+k+'`' if s is None else '('+s+')'
   return 'HEX('+s+('+0' if types[t][k.lower()].startswith(('set(','enum(','tinyint','smallint','mediumint','int','bigint')) else '')+')'
  def keyrows(t,predicate):return [tuple(x.split('\t')) for x in w('SELECT '+','.join(expr(t,k) for k in keys[t])+f' FROM `{t}` WHERE '+predicate+';').splitlines()]
  for s in commands:
   m=re.match(r'UPDATE\s+`?(\w+)`?\s+SET\s+(.*)',s,re.S|re.I)
   if m:
    t=m[1];body,where=split_where(m[2]);fields={re.match(r'\s*`?(\w+)',x)[1].lower() for x in comma_values(body)}
    for key in keyrows(t,where):allowed[t,key].update(fields)
    continue
   m=re.match(r'DELETE FROM\s+`?(\w+)`?\s+WHERE\s+(.*)',s,re.S|re.I)
   if m:
    for key in keyrows(m[1],m[2]):replace.add((m[1],key))
    continue
   m=re.match(r'INSERT(?: IGNORE)? INTO\s+`?(\w+)`?\s*\((.*?)\)\s*VALUES\s*(.*)',s,re.S|re.I);assert m,s[:100];t=m[1];names=[x.strip(' `\n\r\t').lower() for x in comma_values(m[2])];valuespart=re.split(r'\nON DUPLICATE KEY UPDATE',m[3],maxsplit=1)[0]
   rows=[comma_values(x.strip()[1:-1]) for x in comma_values(valuespart)];queries=[]
   for r in rows:queries.append('SELECT '+','.join(expr(t,k,r[names.index(k.lower())]) for k in keys[t])+';')
   for row in w('\n'.join(queries)).splitlines():replace.add((t,tuple(row.split('\t'))))
  def snapshot():
   out={}
   for t in tables:
    pk=','.join(expr(t,k) for k in keys[t]);allhash="SHA2(CONCAT_WS('|',"+','.join("IFNULL(HEX(`"+c+"`),'NULL')" for c in cols[t])+'),256)';rows={}
    for line in w('SELECT '+pk+','+allhash+f' FROM `{t}`;').splitlines():
     fields=line.split('\t');rows[tuple(fields[:-1])]=fields[-1]
    out[t]=rows
   return out
  before=snapshot();partial={}
  for (t,key),fields in allowed.items():
   remaining=[c for c in cols[t] if c.lower() not in fields];where=' AND '.join(expr(t,k)+"='"+v+"'" for k,v in zip(keys[t],key));query='SELECT '+','.join('IFNULL(HEX(`'+c+"`),'NULL')" for c in remaining)+f' FROM `{t}` WHERE '+where+';';partial[t,key]=(query,w(query))
  # Independent desired-value checks. Duplicate source keys resolve in archive order.
  ledger=json.loads((root/'doc/release26-services-ledger-2026-11-06.json').read_text(encoding='utf-8'));latest={}
  for r in ledger['desired']:
   v=r['values'];key=tuple((k.lower(),v[k]) for k in r['keys']);latest[r['table'],key]=r
  checks=[]
  for i,r in enumerate(latest.values()):
   t=r['table'];conditions=[]
   for c,v in r['values'].items():
    ty=types[t][c.lower()]
    if ty.startswith(('float','double')):condition=f'ABS(`{c}`-({v}))<=0.000001*GREATEST(1,ABS({v}))'
    elif ty.startswith(('set(','enum(')) and not v.startswith("'"):condition=f'(`{c}`+0)<=>({v})'
    else:condition=f'BINARY `{c}`<=>BINARY ({v})' if v.startswith("'") else f'`{c}`<=>({v})'
    conditions.append('('+condition+')')
   checks.append(dict(id=f'services-{i:04d}',query=f'SELECT COUNT(*) FROM `{t}` WHERE '+' AND '.join(conditions),expected_rows=[['1']]))
  checks.extend([
   dict(id='spare-parts-active-quest-drops',query='SELECT COUNT(*) FROM gameobject_loot_template WHERE entry IN (203443,203964,203965,203966,203967,203968) AND item=57764 AND lootmode+0=1 AND ChanceOrQuestChance=-100',expected_rows=[['6']]),
   dict(id='spare-parts-disabled-masks-repaired',query='SELECT COUNT(*) FROM gameobject_loot_template WHERE entry IN (203443,203964,203965,203966,203967,203968) AND item=57764 AND lootmode+0=0',expected_rows=[['0']]),
   dict(id='spare-parts-no-orphan-template',query='SELECT COUNT(*) FROM gameobject_loot_template WHERE entry=30020 AND item=57764',expected_rows=[['0']]),
   dict(id='spare-parts-real-chest-owners',query='SELECT COUNT(*) FROM gameobject_template WHERE entry IN (203443,203964,203965,203966,203967,203968) AND type=3 AND data1=entry',expected_rows=[['6']])])
  suite={'name':'release26 services and class spell data','checks':checks};(root/'contrib/upstream-tests/suites/release26-services.json').write_text(json.dumps(suite,indent=2)+'\n',encoding='utf-8')
  def contracts():
   out=w(';\n'.join(c['query'] for c in checks)+';').splitlines();assert len(out)==len(checks)
   return [c['id'] for c,x in zip(checks,out) if [x]!=c['expected_rows'][0]]
  initial=contracts();w(sql);after=snapshot();violations=[]
  for t in tables:
   for k,v in before[t].items():
    if (t,k) not in replace and k not in after[t]:violations.append((t,k,'deleted'))
    elif (t,k) not in replace and (t,k) not in allowed and after[t][k]!=v:violations.append((t,k,'changed'))
  for (t,k),(query,value) in partial.items():
   if (t,k) not in replace and w(query)!=value:violations.append((t,k,'unrelated fields'))
  assert not violations,violations[:5]
  failures=contracts();assert not failures,failures[:10]
  # Reject newly invalid loot distributions or dangling reference edges.
  loot=[t for t in tables if 'loot_template' in t];bad=[]
  for t in loot:
   rows=w(f'SELECT entry,lootmode+0,groupid,SUM(GREATEST(ChanceOrQuestChance,0)) FROM `{t}` WHERE groupid<>0 GROUP BY entry,lootmode,groupid HAVING SUM(GREATEST(ChanceOrQuestChance,0))>100.01;').splitlines()
   bad.extend((t,x) for x in rows)
  report['overfull_groups']=bad
  # Existing unrelated groups may already be invalid; compare their identities to live.
  newbad=[]
  for t,line in bad:
   entry,mode,group,total=line.split('\t');old=q(f'SELECT COALESCE(SUM(GREATEST(ChanceOrQuestChance,0)),0) FROM `{t}` WHERE entry={entry} AND lootmode+0={mode} AND groupid={group};').strip()
   if float(old)<=100.01 or float(total)>float(old)+0.01:newbad.append((t,line))
  assert not newbad,newbad[:5]
  w(sql);repeat=snapshot();assert repeat==after,'Repeat changed state'
  report.update(passed=True,total_contracts=len(checks),initial_mismatches=len(initial),repeat_stable=True,unrelated_rows_fields_preserved=True,changed_rows={t:sum(before[t].get(k)!=after[t].get(k) for k in set(before[t])|set(after[t])) for t in tables})
 finally:
  q('DROP DATABASE `'+schema+'`;');report['fixture_removed']=True;a.report.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
 print(json.dumps({k:v for k,v in report.items() if k!='overfull_groups'}))
if __name__=='__main__':main()
