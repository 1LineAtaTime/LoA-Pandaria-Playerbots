"""Compare trusted numeric UPDATE migrations with an independent row model in table copies."""
import argparse,collections,json,os,pathlib,re,secrets,subprocess
from sql_batch import statements,comma_values
from assemble_release26_quest_batch import split_where

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in ('config','client','migration','report','suite'):p.add_argument('--'+n,type=pathlib.Path,required=True)
 a=p.parse_args();m=re.search(r'^WorldDatabaseInfo\s*=\s*"([^"]+)"',a.config.read_text(encoding='utf-8-sig'),re.M);host,port,user,password,source=m[1].split(';');assert re.fullmatch(r'\w+',source)
 cmd=[str(a.client.resolve()),'--no-defaults','--protocol=TCP','--host='+host,'--port='+port,'--user='+user,'--batch','--skip-column-names','--default-character-set=utf8mb4']
 def q(sql):
  p=subprocess.run(cmd,input=sql,text=True,encoding='utf-8',capture_output=True,timeout=180,env=dict(os.environ,MYSQL_PWD=password))
  if p.returncode:raise RuntimeError(p.stderr.replace(password,'[REDACTED]')[:1500])
  return p.stdout
 updates=[]
 for s in statements(a.migration.read_text(encoding='utf-8')):
  m=re.fullmatch(r'UPDATE `(\w+)`\s+SET\s+(.*)',s,re.S|re.I);assert m,s[:120];body,where=split_where(m[2]);assignments={}
  for x in comma_values(body):
   v=re.fullmatch(r'\s*`(\w+)`\s*=\s*(\d+|`\w+`\s*\|\s*\d+)\s*',x);assert v,x;assignments[v[1]]=v[2]
  updates.append((m[1],assignments,where))
 schema='codex_numeric_batch_'+secrets.token_hex(5);tables=sorted(set(u[0] for u in updates));report=dict(passed=False,gameplay_tested=False);q('CREATE DATABASE `'+schema+'`;')
 def w(s):return q('USE `'+schema+'`;'+s)
 try:
  for t in tables:q(f'CREATE TABLE `{schema}`.`{t}` LIKE `{source}`.`{t}`;INSERT INTO `{schema}`.`{t}` SELECT * FROM `{source}`.`{t}`;')
  cols={t:[x.split('\t')[0] for x in w('SHOW COLUMNS FROM `'+t+'`;').splitlines()] for t in tables};indexes={t:{c.lower():i for i,c in enumerate(cols[t])} for t in tables}; changed={t:{c.lower() for u,vs,_ in updates if u==t for c in vs} for t in tables}; keys={t:{x.split('\t')[4].lower() for x in w('SHOW KEYS FROM `'+t+'` WHERE Key_name="PRIMARY";').splitlines()} for t in tables}
  def rows(t,where='1'):
   return [tuple(x.split('\t')) for x in w('SELECT '+','.join("IFNULL(HEX(CAST(`"+c+"` AS BINARY)),'NULL')" for c in cols[t])+f' FROM `{t}` WHERE '+where+';').splitlines()]
  before={t:collections.Counter(rows(t)) for t in tables};expected={t:v.copy() for t,v in before.items()};checks=[];matched=[]
  for n,(t,values,where) in enumerate(updates):
   selected=collections.Counter(rows(t,where));matched.append(sum(selected.values()));new=[]
   for row,count in selected.items():
    out=list(row)
    for col,expr in values.items():
     if expr.isdigit():value=int(expr)
     else:
      m=re.fullmatch(r'`(\w+)`\s*\|\s*(\d+)',expr);value=int(bytes.fromhex(row[indexes[t][m[1].lower()]]).decode())|int(m[2])
     out[indexes[t][col.lower()]]=str(value).encode().hex().upper()
    out=tuple(out);expected[t][row]-=count;expected[t][out]+=count
    # Contracts pin keys/changed fields; the independent Counter model checks every field.
    pred=' AND '.join("IFNULL(HEX(CAST(`"+c+"` AS BINARY)),'NULL')='"+v+"'" for c,v in zip(cols[t],out) if not keys[t] or c.lower() in keys[t]|changed[t])
    checks.append(dict(id=f'sf27-service-{n}-{len(checks)}',query='SELECT COUNT(*) FROM `'+t+'` WHERE '+pred,expected_rows=[[str(count)]]))
   setsql=','.join('`'+c+'`='+v for c,v in values.items());w('UPDATE `'+t+'` SET '+setsql+' WHERE '+where+';')
  after={t:collections.Counter(rows(t)) for t in tables};assert all(+expected[t]==after[t] for t in tables),'Unexpected full-row mutation'
  sql=a.migration.read_text(encoding='utf-8');w(sql);assert after=={t:collections.Counter(rows(t)) for t in tables},'Not idempotent'
  # Final rows can be touched by more than one source update. Test only final full rows.
  final=w(';\n'.join(c['query'] for c in checks)+';').splitlines();assert len(final)==len(checks)
  checks=[c for c,value in zip(checks,final) if value==c['expected_rows'][0][0]]
  a.suite.write_text(json.dumps(dict(name='SF27 service flags and quest gossip gates',checks=checks),indent=2)+'\n',encoding='utf-8')
  report.update(passed=True,full_row_preservation=True,idempotent=True,source_updates=len(updates),matched_by_update=matched,contracts=len(checks),changed_rows={t:sum((before[t]-after[t]).values()) for t in tables})
 finally:
  q('DROP DATABASE `'+schema+'`;');report['fixture_removed']=True;a.report.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
 print(json.dumps(report))
if __name__=='__main__':main()
