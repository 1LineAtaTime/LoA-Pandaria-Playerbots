"""Preservation/repeat/restore checks for the pinned, trusted Gilneas SQL batch."""
import pathlib,json,re,secrets,collections,hashlib,argparse,subprocess,os
from sql_batch import statements, comma_values


def main():
    parser=argparse.ArgumentParser(description='Test the reviewed Gilneas batch in disposable schemas; use a pre-batch backup after deployment.')
    parser.add_argument('--config',type=pathlib.Path,required=True)
    parser.add_argument('--client',type=pathlib.Path,required=True)
    parser.add_argument('--report',type=pathlib.Path,required=True)
    parser.add_argument('--backup',type=pathlib.Path)
    parser.add_argument('--batch',choices=('gilneas','crowley'),default='gilneas')
    args=parser.parse_args()
    root=pathlib.Path(__file__).resolve().parents[2]
    match=re.search(r'^WorldDatabaseInfo\s*=\s*"([^"]+)"',args.config.read_text(encoding='utf-8-sig'),re.M)
    if not match:raise ValueError('WorldDatabaseInfo missing')
    host,port,user,password,source=match[1].split(';')
    assert re.fullmatch('[A-Za-z0-9_]+',source)
    command=[str(args.client.resolve()),'--no-defaults','--protocol=TCP','--host='+host,'--port='+port,'--user='+user,'--database='+source,'--batch','--skip-column-names','--default-character-set=utf8mb4']
    def q(sql):
     p=subprocess.run(command,input=sql,text=True,encoding='utf-8',capture_output=True,timeout=180,env=dict(os.environ,MYSQL_PWD=password))
     if p.returncode:raise RuntimeError(p.stderr.replace(password,'[REDACTED]')[:1500] if password else p.stderr[:1500])
     return p.stdout
    stmts=statements;csv=comma_values
    names=['2026_11_05_30_world_gilneas_progression.sql','2026_11_05_31_world_gilneas_backend_alignment.sql'] if args.batch=='gilneas' else ['2026_11_05_32_world_crowley_ride.sql']
    sql='\n'.join((root/'sql/updates/world'/name).read_text(encoding='utf-8') for name in names);commands=stmts(sql)

    base='codex_gilneas_base_'+secrets.token_hex(4);work='codex_gilneas_work_'+secrets.token_hex(4)
    assert base != source and work != source
    mutations=[];tables=set()
    for s in commands:
     m=re.match(r'(?:UPDATE|INSERT INTO|ALTER TABLE)\s+`?(\w+)`?\b',s,re.I)
     if not m:m=re.match(r'DELETE\s+(?:`?\w+`?\s+)?FROM\s+`?(\w+)`?',s,re.I)
     assert m,s[:100]
     table=m[1];tables.add(table);mutations.append((table,s))
    if args.batch=='crowley':tables.update(['waypoints','creature_text','quest_objective','creature_queststarter','creature_questender'])
    tables=sorted(tables)
    def w(sql):return q('USE `'+work+'`;'+sql)
    def b(sql):return q('USE `'+base+'`;'+sql)
    report={'passed':False,'gameplay_tested':False,'tables':tables,'execution':'Reviewed local SQL in disposable schemas using the configured administrative account; not an untrusted-SQL sandbox'}
    created=[]
    try:
     q('CREATE DATABASE `'+base+'`;');created.append(base)
     q('CREATE DATABASE `'+work+'`;');created.append(work)
     if args.backup:
      body=args.backup.read_text(encoding='utf-8-sig')
      assert not re.search(r'^\s*(USE\s|CREATE DATABASE|DROP DATABASE)',body,re.M|re.I)
      b(body);report['actual_backup_sha256']=hashlib.sha256(args.backup.read_bytes()).hexdigest()
     for t in tables:
      if not args.backup:q(f'CREATE TABLE `{base}`.`{t}` LIKE `{source}`.`{t}`; INSERT INTO `{base}`.`{t}` SELECT * FROM `{source}`.`{t}`;')
      q(f'CREATE TABLE `{work}`.`{t}` LIKE `{base}`.`{t}`; INSERT INTO `{work}`.`{t}` SELECT * FROM `{base}`.`{t}`;')
     columns={t:[r.split('\t')[0] for r in w('SHOW COLUMNS FROM `'+t+'`;').splitlines()] for t in tables}
     keys={}
     for t in tables:
      rows=w("SELECT COLUMN_NAME FROM information_schema.KEY_COLUMN_USAGE WHERE TABLE_SCHEMA='"+work+"' AND TABLE_NAME='"+t+"' AND CONSTRAINT_NAME='PRIMARY' ORDER BY ORDINAL_POSITION;").splitlines()
      keys[t]=rows or [c for c in columns[t] if c.lower() not in ('comment','scriptname')]
     def snapshot(run):
      result={}
      for t in tables:
       body="CONCAT_WS('|',"+','.join("IFNULL(HEX(CAST(`"+c+"` AS BINARY)),'NULL')" for c in columns[t])+')'
       select=','.join('`'+c+'`' for c in keys[t])+',SHA2('+body+',256)'
       result[t]=collections.Counter(tuple(row.split('\t')) for row in run('SELECT '+select+' FROM `'+t+'`;').rstrip('\n').split('\n') if row)
      return result
     def literal(raw):
      raw=raw.strip()
      if re.fullmatch(r'-?\d+',raw):return str(int(raw))
      if raw.startswith("'") and raw.endswith("'") and '\\' not in raw:return raw[1:-1].replace("''", "'")
      if raw.upper()=='NULL':return 'NULL'
      raise ValueError('Unsupported identity literal '+raw)
     def scopes(selected=None):
      allowed={t:set() for t in tables}
      for t,s in (mutations if selected is None else selected):
       insert=re.fullmatch(r'INSERT INTO\s+`?\w+`?\s*\((.*?)\)\s*VALUES\s*(.*)',s,re.S|re.I)
       if insert:
        names=[c.strip('`').lower() for c in csv(insert[1])]
        for row in csv(insert[2]):
         fields=csv(row[1:-1]);values=dict(zip(names,fields));assert len(fields)==len(names)
         allowed[t].add(tuple(literal(values[c.lower()]) for c in keys[t]))
        continue
       update=re.match(r'UPDATE\s+`?\w+`?\s+SET\b.*?\bWHERE\b(.*)',s,re.S|re.I)
       if update:query='SELECT '+','.join('`'+c+'`' for c in keys[t])+' FROM `'+t+'` WHERE '+update[1]
       else:
        delete=re.match(r'DELETE\s+(?:(`?\w+`?)\s+)?FROM\s+(.*)',s,re.S|re.I)
        if not delete:continue
        prefix=(delete[1]+'.') if delete[1] else ''
        query='SELECT '+','.join(prefix+'`'+c+'`' for c in keys[t])+' FROM '+delete[2]
       for row in w(query+';').rstrip('\n').split('\n'):
        if row:allowed[t].add(tuple(row.split('\t')))
      return allowed
     # Reject reserved-ID collisions and text that is cross-owned by two real NPCs.
     if args.batch=='gilneas':
      assert not b('SELECT entry FROM gameobject_template WHERE entry=800100;').strip()
      assert not b('SELECT guid FROM gameobject WHERE guid BETWEEN 4100001 AND 4100004;').strip()
     actors=[37067,37953,38218,38469,43566,43749,43767]; actor_ids=','.join(map(str,actors))
     assert not b(f'SELECT entry FROM creature_text WHERE entry>0 AND CreatureID>0 AND entry<>CreatureID AND (entry IN ({actor_ids}) OR CreatureID IN ({actor_ids}));').strip()
     before=snapshot(b);allowed={t:set() for t in tables}
     # Backend changes can enable a later guarded delete. Capture each statement's
     # real pre/post identities, including intermediate states, before proceeding.
     for t,s in mutations:
      prior=scopes([(t,s)]);w(s+';');following=scopes([(t,s)])
      for name in tables:allowed[name]|=prior[name]|following[name]
     after=snapshot(w);later=scopes()
     for t in tables:
      allowed[t]|=later[t]
      removed=before[t]-after[t];added=after[t]-before[t]
      assert all(row[:-1] in allowed[t] for row in removed),('Unplanned removal/update',t,[r[:-1] for r in removed if r[:-1] not in allowed[t]][:5])
      assert all(row[:-1] in allowed[t] for row in added),('Unplanned addition/update',t,[r[:-1] for r in added if r[:-1] not in allowed[t]][:5])
     w(sql);assert snapshot(w)==after,'Second application changed rows'
     assert w("SELECT IS_NULLABLE FROM information_schema.COLUMNS WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME='waypoints' AND COLUMN_NAME='orientation';").strip()=='YES'
     # Actual postconditions, on fixture only, with the same tracked SELECT suites.
     suite_name='gilneas-progression.json' if args.batch=='gilneas' else 'crowley-ride.json'
     suite=json.loads((root/'contrib/upstream-tests/suites'/suite_name).read_text(encoding='utf-8'));passed=0
     for case in suite['checks']:
      actual=[r.split('\t') for r in w(case['query']).rstrip('\n').split('\n') if r]
      expected=[[str(v) for v in row] for row in case['expected_rows']]
      assert actual==expected,(case['id'],actual,expected);passed+=1
     # Test restoration of data and the nullable-column DDL from protected baseline.
     for t in tables:w(f'DROP TABLE `{t}`; CREATE TABLE `{t}` LIKE `{base}`.`{t}`; INSERT INTO `{t}` SELECT * FROM `{base}`.`{t}`;')
     assert snapshot(w)==before,'Baseline restoration changed data'
     assert w("SELECT IS_NULLABLE FROM information_schema.COLUMNS WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME='waypoints' AND COLUMN_NAME='orientation';").strip()==b("SELECT IS_NULLABLE FROM information_schema.COLUMNS WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME='waypoints' AND COLUMN_NAME='orientation';").strip()
     report.update(passed=True,preservation_outside_declared_sql_key_scopes=True,idempotent=True,full_table_restore=True,postconditions=passed,
      row_changes={t:{'removed_or_replaced':sum((before[t]-after[t]).values()),'added_or_replaced':sum((after[t]-before[t]).values()),'key_scope':len(allowed[t])} for t in tables})
    except Exception as error:
     detail=str(error)
     report['failure']=type(error).__name__+': '+(detail.replace(password,'[REDACTED]') if password else detail)
     raise
    finally:
     for name in reversed(created):
      assert name.startswith('codex_gilneas_')
      q('DROP DATABASE IF EXISTS `'+name+'`;')
     report['fixtures_removed']=True;args.report.parent.mkdir(parents=True,exist_ok=True);args.report.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))


if __name__=='__main__':
    main()
