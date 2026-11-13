"""Verify SF27 combat migration in disposable copies, never the live world."""
import argparse,hashlib,json,os,pathlib,re,secrets,subprocess

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in ('config','client','report'):p.add_argument('--'+n,type=pathlib.Path,required=True)
 a=p.parse_args();root=pathlib.Path(__file__).resolve().parents[2]
 m=re.search(r'^WorldDatabaseInfo\s*=\s*"([^"]+)"',a.config.read_text(encoding='utf-8-sig'),re.M);host,port,user,password,source=m[1].split(';');assert re.fullmatch(r'\w+',source)
 cmd=[str(a.client.resolve()),'--no-defaults','--protocol=TCP','--host='+host,'--port='+port,'--user='+user,'--batch','--skip-column-names','--default-character-set=utf8mb4']
 def q(sql):
  r=subprocess.run(cmd,input=sql,text=True,encoding='utf-8',capture_output=True,timeout=180,env=dict(os.environ,MYSQL_PWD=password))
  if r.returncode:raise RuntimeError(r.stderr.replace(password,'[REDACTED]')[:1800])
  return r.stdout
 ledger=json.loads((root/'doc/release27-combat-ledger-2026-11-06.json').read_text(encoding='utf-8'));migration=root/'sql/updates/world/2026_11_06_40_world_release27_combat.sql';sql=migration.read_text(encoding='utf-8');assert hashlib.sha256(migration.read_bytes()).hexdigest()==ledger['migration_sha256']
 schema='codex_combat_'+secrets.token_hex(5);tables=['creature_template','creature_classlevelstats'];report=dict(passed=False,gameplay_tested=False)
 q('CREATE DATABASE `'+schema+'`;')
 def w(s):return q('USE `'+schema+'`;'+s)
 def digest(table,cols,where='1'):
  pk='`entry`' if table=='creature_template' else '`level`,`class`'
  result=w('SELECT '+pk+",SHA2(CONCAT_WS('|',"+','.join("IFNULL(HEX(`"+c+"`),'NULL')" for c in cols)+'),256) FROM `'+table+'` WHERE '+where+';')
  return dict((tuple(x.split('\t')[:-1]),x.split('\t')[-1]) for x in result.splitlines())

 try:
  for t in tables:q(f'CREATE TABLE `{schema}`.`{t}` LIKE `{source}`.`{t}`; INSERT INTO `{schema}`.`{t}` SELECT * FROM `{source}`.`{t}`;')
  cols={t:[x.split('\t')[0] for x in w('SHOW COLUMNS FROM `'+t+'`;').splitlines()] for t in tables}
  keys=','.join(str(r['entry']) for r in ledger['creatures']);statkeys=','.join('('+r['level']+','+r['class']+')' for r in ledger['class_stats'])
  protected=[c for c in cols['creature_template'] if c not in ('mindmg','maxdmg','attackpower')]
  before={t:digest(t,cols[t]) for t in tables};safe=digest('creature_template',protected);outside=digest('creature_template',cols['creature_template'],'entry NOT IN ('+keys+')');extra=digest('creature_classlevelstats',cols['creature_classlevelstats'],'(`level`,`class`) NOT IN ('+statkeys+')')
  checks=[]
  for row in ledger['class_stats']:
   checks.append(dict(id='sf27-stats-'+row['level']+'-'+row['class'],query='SELECT COUNT(*) FROM creature_classlevelstats WHERE '+' AND '.join(f'ABS((`{k}`+0.0)-({v}))<=0.000002*GREATEST(1,ABS({v}))' for k,v in row.items()),expected_rows=[['1']]))
  for row in ledger['creatures']:
   checks.append(dict(id='sf27-melee-'+str(row['entry']),query='SELECT COUNT(*) FROM creature_template WHERE entry='+str(row['entry'])+' AND '+' AND '.join(f'ABS((`{k}`+0.0)-({v}))<=0.000002*GREATEST(1,ABS({v}))' for k,v in row['values'].items()),expected_rows=[['1']]))
  suite=dict(name='SF27 matched melee baseline',checks=checks);(root/'contrib/upstream-tests/suites/release27-combat.json').write_text(json.dumps(suite,indent=2)+'\n',encoding='utf-8')
  def contracts():
   out=w(';\n'.join(c['query'] for c in checks)+';').splitlines();assert len(out)==len(checks)
   return [c['id'] for c,v in zip(checks,out) if v!='1']
  initial=contracts();w(sql);assert digest('creature_template',protected)==safe,'Unrelated template fields changed';assert digest('creature_template',cols['creature_template'],'entry NOT IN ('+keys+')')==outside,'Unrelated templates changed';assert digest('creature_classlevelstats',cols['creature_classlevelstats'],'(`level`,`class`) NOT IN ('+statkeys+')')==extra,'Extra custom stat rows changed'
  failures=contracts();assert not failures,failures[:10]
  after={t:digest(t,cols[t]) for t in tables};w(sql);assert after=={t:digest(t,cols[t]) for t in tables},'Repeat changed data'
  report.update(passed=True,contracts=len(checks),initial_mismatches=len(initial),protected_fields_unchanged=True,unrelated_templates_unchanged=True,extra_stat_rows_unchanged=True,idempotent=True,changed_rows={t:sum(before[t].get(k)!=after[t].get(k) for k in set(before[t])|set(after[t])) for t in tables})
 finally:
  q('DROP DATABASE `'+schema+'`;');report['fixture_removed']=True;a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
 print(json.dumps(report))
if __name__=='__main__':main()
