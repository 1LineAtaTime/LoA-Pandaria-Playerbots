"""Check trusted SQL42 in disposable world-table copies; no live mutation."""
import pathlib,argparse,re,subprocess,os,secrets,json,hashlib,collections,sys
from sql_batch import statements,comma_values
from assemble_release26_quest_batch import split_where

def main():
 ap=argparse.ArgumentParser(description=__doc__)
 for k in ('config','client','report'):ap.add_argument('--'+k,type=pathlib.Path,required=True)
 a=ap.parse_args();root=pathlib.Path(__file__).resolve().parents[2];m=re.search(r'^WorldDatabaseInfo\s*=\s*"([^"]+)"',a.config.read_text(encoding='utf-8-sig'),re.M);host,port,user,password,source=m[1].split(';')
 cmd=[str(a.client.resolve()),'--no-defaults','--protocol=TCP','--host='+host,'--port='+port,'--user='+user,'--database='+source,'--batch','--skip-column-names','--default-character-set=utf8mb4']
 def q(sql):
  p=subprocess.run(cmd,input=sql.rstrip().rstrip(';')+";\nSELECT '__SF27_QUERY_DONE__';\n",text=True,encoding='utf-8',capture_output=True,timeout=180,env=dict(os.environ,MYSQL_PWD=password))
  if p.returncode:raise RuntimeError(p.stderr.replace(password,'[REDACTED]')[:1600])
  assert p.stdout.endswith('__SF27_QUERY_DONE__\n'),'Incomplete client response'
  return p.stdout[:-len('__SF27_QUERY_DONE__\n')]
 names=['2026_11_06_42_world_release27_quests_loot_travel.sql','2026_11_06_43_world_dragonmaw_loot_ownership.sql'];sql='\n'.join((root/'sql/updates/world'/n).read_text(encoding='utf-8') for n in names);commands=statements(sql);tables=sorted(set(re.findall(r'(?m)^(?:UPDATE|INSERT(?: IGNORE)? INTO|DELETE FROM)\s+`?(\w+)`?',sql,re.I)))
 assert commands and tables,'Empty migration is not a valid deployment batch'
 schema='codex_sf27_world_'+secrets.token_hex(5)
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
   ensured=re.fullmatch(r'INSERT IGNORE INTO `(quest_template_addon|quest_offer_reward|quest_request_items)` \(`ID`\) SELECT `ID` FROM `quest_template` WHERE `ID`=(\d+)',s)
   if ensured:
    for k in w('SELECT HEX('+ensured[2]+');').splitlines():replace.add((ensured[1],(k,)))
    continue
   m=re.match(r'INSERT(?: IGNORE)? INTO\s+`?(\w+)`?\s*\((.*?)\)\s*VALUES\s*(.*)',s,re.S|re.I);assert m,s[:100];t=m[1];names=[x.strip(' `\n\r\t').lower() for x in comma_values(m[2])];valuespart=re.split(r'\nON DUPLICATE KEY UPDATE',m[3],maxsplit=1)[0]
   rows=[comma_values(x.strip()[1:-1]) for x in comma_values(valuespart)];queries=[]
   for r in rows:queries.append('SELECT '+','.join(expr(t,k,r[names.index(k.lower())]) for k in keys[t])+';')
   for row in w('\n'.join(queries)).splitlines():replace.add((t,tuple(row.split('\t'))))
  def snapshot():
   out={}
   for t in tables:
    pk=','.join(expr(t,k) for k in keys[t]);allhash="SHA2(CONCAT_WS('|',"+','.join("IFNULL(HEX(CAST(`"+c+"` AS BINARY)),'NULL')" for c in cols[t])+'),256)';rows={}
    for line in w('SELECT '+pk+','+allhash+f' FROM `{t}`;').splitlines():
     fields=line.split('\t');rows[tuple(fields[:-1])]=fields[-1]
    out[t]=rows
   return out
  def loot_query(t):return f'SELECT entry,lootmode+0,groupid,SUM(GREATEST(ChanceOrQuestChance,0)) FROM `{t}` WHERE groupid<>0 GROUP BY entry,lootmode,groupid HAVING SUM(GREATEST(ChanceOrQuestChance,0))>100.01;'
  baseline_loot={}
  for t in tables:
   if 'loot_template' in t:baseline_loot[t]={tuple(x.split('\t')[:-1]):float(x.split('\t')[-1]) for x in w(loot_query(t)).splitlines()}
  before=snapshot();partial={}
  for (t,key),fields in allowed.items():
   remaining=[c for c in cols[t] if c.lower() not in fields];where=' AND '.join(expr(t,k)+"='"+v+"'" for k,v in zip(keys[t],key));query='SELECT '+','.join('IFNULL(HEX(CAST(`'+c+"` AS BINARY)),'NULL')" for c in remaining)+f' FROM `{t}` WHERE '+where+';';partial[t,key]=(query,w(query))
  # Independent desired-value checks. Duplicate source keys resolve in archive order.
  ledger=json.loads((root/'doc/release27-world-ledger-2026-11-06.json').read_text(encoding='utf-8'));latest={}
  for r in ledger['desired']:
   v=r['values'];key=tuple((k.lower(),v[k]) for k in r['keys']);
   if (r['table'],key) in latest:latest[r['table'],key]['values'].update(v)
   else:latest[r['table'],key]=dict(r,values=dict(v))
  checks=[]
  for i,r in enumerate(latest.values()):
   t=r['table'];conditions=[]
   for c,v in r['values'].items():
    ty=types[t][c.lower()]
    if ty.startswith(('float','double')):condition=f'ABS((`{c}`+0.0)-({v}))<=0.000001*GREATEST(1,ABS({v}))'
    elif ty.startswith(('set(','enum(')) and not v.startswith("'"):condition=f'(`{c}`+0)<=>({v})'
    else:condition=f'BINARY `{c}`<=>BINARY ({v})' if v.startswith("'") else f'`{c}`<=>({v})'
    conditions.append('('+condition+')')
   checks.append(dict(id=f'services-{i:04d}',query=f'SELECT COUNT(*) FROM `{t}` WHERE '+' AND '.join(conditions),expected_rows=[['1']]))
  for entry in (41072,41080,42107):checks.append(dict(id='dragonmaw-retain-populated-owner-'+str(entry),query=f'SELECT COUNT(*) FROM creature_template ct WHERE entry={entry} AND lootid={entry} AND EXISTS (SELECT 1 FROM creature_loot_template l WHERE l.entry=ct.lootid)',expected_rows=[['1']]))
  suite={'name':'SF27 quest text, chains, loot and travel','checks':checks};(root/'contrib/upstream-tests/suites/release27-world.json').write_text(json.dumps(suite,indent=2)+'\n',encoding='utf-8')
  def contracts():
   out=w(';\n'.join(c['query'] for c in checks)+';').splitlines();assert len(out)==len(checks)
   return [c['id'] for c,x in zip(checks,out) if [x]!=c['expected_rows'][0]]
  initial=contracts();w(sql);after=snapshot();violations=[]
  for t in tables:
   for k,v in before[t].items():
    if (t,k) not in replace and k not in after[t]:violations.append((t,k,'deleted'))
    elif (t,k) not in replace and (t,k) not in allowed and after[t][k]!=v:violations.append((t,k,'changed'))
  for (t,k),(query,value) in partial.items():
   if (t,k) not in replace and w(query)!=value:violations.append((t,k,'unrelated fields',value,w(query)))
  assert not violations,violations[:5]
  failures=contracts();assert not failures,failures[:10]
  # Retained quest/shop/bot/data contracts run against the same migrated table copies.
  # Tables outside this batch stay read-only in the live schema.
  retained=[]
  for name in ('starting-training','training-chain-links','gilneas-progression','crowley-ride','existing-quest-ports','release26-quests','release26-services','release27-combat','release27-services'):
   for c in json.loads((root/'contrib/upstream-tests/suites'/(name+'.json')).read_text(encoding='utf-8'))['checks']:retained.append(dict(c,id=name+'-'+c['id']))
  # Qualify unmutated tables into source, without copying sensitive schemas.
  known=set(q('SHOW TABLES;').splitlines());untouched=known-set(tables)
  statements_read=[]
  for i,c in enumerate(retained):
   query=c['query'].replace('DATABASE()',"'"+source+"'")
   query=re.sub(r'(?i)(FROM|JOIN)\s+`?(\w+)`?',lambda m:m[1]+' `'+source+'`.`'+m[2]+'`' if m[2] in untouched else m[0],query)
   statements_read.extend([query.rstrip(';')+';',f"SELECT '__KEEP_{i}__';"])
  result=w('\n'.join(statements_read)).splitlines();cursor=0;regressions=[]
  for i,c in enumerate(retained):
   actual=[]
   while cursor<len(result) and result[cursor]!=f'__KEEP_{i}__':actual.append(result[cursor].split('\t'));cursor+=1
   assert cursor<len(result);cursor+=1
   if actual!=[[str(v) for v in row] for row in c['expected_rows']]:regressions.append(c['id'])
  report['retained_contracts']=len(retained);report['retained_conflicts']=regressions
  assert not regressions,regressions[:15]

  # Compare grouped distributions with the pre-migration fixture, in four batch reads.
  newbad=[]
  for t in baseline_loot:
   for line in w(loot_query(t)).splitlines():
    *key,total=line.split('\t');old=baseline_loot[t].get(tuple(key))
    if old is None or float(total)>old+0.01:newbad.append((t,line))
  report['new_overfull_groups']=newbad
  assert not newbad,newbad[:5]
  w(sql);repeat=snapshot();assert repeat==after,'Repeat changed state'
  report.update(passed=True,total_contracts=len(checks),initial_mismatches=len(initial),repeat_stable=True,unrelated_rows_fields_preserved=True,changed_rows={t:sum(before[t].get(k)!=after[t].get(k) for k in set(before[t])|set(after[t])) for t in tables})
 finally:
  q('DROP DATABASE `'+schema+'`;');report['fixture_removed']=True;a.report.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
 print(json.dumps({k:v for k,v in report.items() if k!='overfull_groups'}))
if __name__=='__main__':main()
