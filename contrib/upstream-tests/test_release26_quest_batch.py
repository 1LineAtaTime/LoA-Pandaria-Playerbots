"""Check the reviewed SQL33 batch once in a restored world-table backup."""
import pathlib,argparse,re,subprocess,os,secrets,json,hashlib,collections
from sql_batch import statements,comma_values

def main():
 ap=argparse.ArgumentParser(description=__doc__)
 for k in ('config','client','backup','report'):ap.add_argument('--'+k,type=pathlib.Path,required=True)
 a=ap.parse_args();root=pathlib.Path(__file__).resolve().parents[2]
 m=re.search(r'^WorldDatabaseInfo\s*=\s*"([^"]+)"',a.config.read_text(encoding='utf-8-sig'),re.M);host,port,user,password,source=m[1].split(';')
 cmd=[str(a.client.resolve()),'--no-defaults','--protocol=TCP','--host='+host,'--port='+port,'--user='+user,'--database='+source,'--batch','--skip-column-names','--default-character-set=utf8mb4']
 def q(sql):
  p=subprocess.run(cmd,input=sql,text=True,encoding='utf-8',capture_output=True,timeout=180,env=dict(os.environ,MYSQL_PWD=password))
  if p.returncode:raise RuntimeError(p.stderr.replace(password,'[REDACTED]')[:1800])
  return p.stdout
 schema='codex_release26_'+secrets.token_hex(5)
 def w(sql):return q('USE `'+schema+'`;'+sql)
 sql=(root/'sql/updates/world/2026_11_06_33_world_release26_quests_ai.sql').read_text(encoding='utf-8');commands=statements(sql)
 tables=sorted(set(re.findall(r'(?:UPDATE|INSERT INTO|DELETE FROM)\s+`(\w+)`',sql,re.I)))
 report={'passed':False,'gameplay_tested':False,'backup_sha256':hashlib.sha256(a.backup.read_bytes()).hexdigest(),'tables':tables}
 q('CREATE DATABASE `'+schema+'`;')
 try:
  backup=a.backup.read_text(encoding='utf-8-sig');assert not re.search(r'^\s*(USE\s|CREATE DATABASE|DROP DATABASE)',backup,re.M|re.I);w(backup)
  cols={};keys={};allowed=collections.defaultdict(set);replace=set()
  for t in tables:
   cols[t]=[x.split('\t')[0] for x in w(f'SHOW COLUMNS FROM `{t}`;').splitlines()]
   keys[t]=[x.split('\t')[4] for x in w(f'SHOW KEYS FROM `{t}` WHERE Key_name="PRIMARY";').splitlines()];assert keys[t],t
  def keyrows(t,predicate):return [tuple(x.split('\t')) for x in w('SELECT '+','.join('HEX(`'+k+'`)' for k in keys[t])+f' FROM `{t}` WHERE '+predicate+';').splitlines()]
  for s in commands:
   if s.startswith('UPDATE '):
    m=re.match(r'UPDATE `(\w+)` SET (.*?) WHERE (.*)',s,re.S);t=m[1];fields={re.match(r'`(\w+)`',x)[1].lower() for x in comma_values(m[2])}
    for key in keyrows(t,m[3]):allowed[t,key].update(fields)
   elif s.startswith('DELETE FROM '):
    m=re.match(r'DELETE FROM `(\w+)` WHERE (.*)',s,re.S);t=m[1]
    for key in keyrows(t,m[2]):replace.add((t,key))
   elif s.startswith('INSERT INTO '):
    m=re.match(r'INSERT INTO `(\w+)` \((.*?)\) VALUES\s*(.*)',s,re.S)
    if m:
     t=m[1];names=[x.strip(' `').lower() for x in comma_values(m[2])]
     for row in comma_values(m[3]):
      values=comma_values(row.strip()[1:-1]);pkvalues=[values[names.index(k.lower())] for k in keys[t]];assert all(re.fullmatch(r'\d+',v) for v in pkvalues),pkvalues
      key=tuple(w('SELECT '+','.join('HEX('+v+')' for v in pkvalues)+';').strip().split('\t'));replace.add((t,key))
    else:assert re.match(r'INSERT INTO `quest_(?:template_addon|offer_reward|request_items)` \(`ID`\) SELECT `ID` FROM `quest_template` WHERE `ID`=\d+ AND NOT EXISTS',s),s[:100]
   else:raise ValueError(s[:100])
  def snapshot():
   out={}
   for t in tables:
    col=cols[t];pk=[col.index(k) for k in keys[t]];rows={}
    body=w('SELECT '+','.join("IFNULL(HEX(`"+c+"`),'NULL')" for c in col)+f' FROM `{t}`;')
    for line in body.splitlines():
     values=line.split('\t');key=tuple(values[i] for i in pk);row=tuple(values);
     preserved=tuple(v for c,v in zip(col,values) if c.lower() not in allowed[t,key]);rows[key]=(hashlib.sha256(repr(row).encode()).hexdigest(),hashlib.sha256(repr(preserved).encode()).hexdigest())
    out[t]=rows
   return out
  before=snapshot();w(sql);after=snapshot();changes={};violations=[]
  for t in tables:
   changes[t]=sum(before[t].get(k)!=after[t].get(k) for k in set(before[t])|set(after[t]))
   for k,v in before[t].items():
    if (t,k) not in replace and (k not in after[t] or v[1]!=after[t][k][1]):violations.append((t,k))
  report['preservation_violations']=[dict(table=t,key=list(k)) for t,k in violations]
  assert not violations,violations[:5]
  desired=json.loads((root/'contrib/upstream-tests/suites/release26-quest-fields.json').read_text(encoding='utf-8'));failed=[]
  for r in desired:
   query=f"SELECT COUNT(*) FROM `{r['table']}` WHERE ID={r['id']} AND BINARY `{r['field']}`=BINARY {r['sql_value']};"
   if w(query).strip()!='1':failed.append((r['table'],r['id'],r['field']))
  assert not failed,failed
  # Link topology and authored speaker identities use the actual fixture rows.
  assert not w('SELECT s.entryorguid FROM smart_scripts s LEFT JOIN smart_scripts l ON l.entryorguid=s.entryorguid AND l.source_type=s.source_type AND l.id=s.link WHERE s.entryorguid=946 AND s.source_type=0 AND s.link<>0 AND (l.id IS NULL OR l.event_type<>61 OR s.link=s.id);').strip()
  w(sql);repeat=snapshot();assert repeat==after,'Repeat changed state'
  report.update(passed=True,changed_rows=changes,quest_fields=len(desired),repeat_stable=True,unrelated_fields_preserved=True)
 finally:
  q('DROP DATABASE `'+schema+'`;');report['fixture_removed']=True;a.report.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
 print(json.dumps(report))
if __name__=='__main__':main()
