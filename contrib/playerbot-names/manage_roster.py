"""Import, plan, rename and roll back positively registered bots on one existing realm.
All writes require --apply. Rename/rollback/backup require worldserver offline.
Private plans, backups and character fingerprints stay under ignored out/.
"""
import argparse,csv,gzip,hashlib,json,math,os,random,re,shutil,subprocess,sys,time
from pathlib import Path
R=Path(__file__).resolve().parents[2];sys.path.insert(0,str(R/'contrib/headless-client'))
from starter_campaign import Database
A=argparse.ArgumentParser();A.add_argument('mode',choices=['backup','import','plan','rename','rollback','verify']);A.add_argument('--apply',action='store_true');A.add_argument('--config',type=Path,default=R/'exports/worldserver.conf');A.add_argument('--client',type=Path,default=R/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe');A.add_argument('--csv',type=Path,default=R/'modules/mod_playerbots/data/names/playerbot_names.csv');A.add_argument('--out',type=Path,default=R/'out/playerbot-naming-20261112');A.add_argument('--prefix',default='mop_');a=A.parse_args();a.out.mkdir(parents=True,exist_ok=True);assert re.fullmatch(r'[A-Za-z0-9_]+',a.prefix)
d=Database(a.config,a.client);P=a.prefix;sha=hashlib.sha256(a.csv.read_bytes()).hexdigest()
CLASSES={'Warrior':1,'Paladin':2,'Hunter':3,'Rogue':4,'Priest':5,'Death Knight':6,'Shaman':7,'Mage':8,'Warlock':9,'Druid':11,'Monk':10}
# Class IDs in this core follow the client: Druid11, Monk10.
RACES={'Human':1,'Orc':2,'Dwarf':3,'Night Elf':4,'Undead':5,'Tauren':6,'Gnome':7,'Troll':8,'Goblin':9,'Blood Elf':10,'Draenei':11,'Worgen':22,'Pandaren':24}
def q(s):return d.q('Character',s)
def lit(s):return "'"+str(s).replace('\\','\\\\').replace("'","\\'")+"'"
def offline():
 out=(subprocess.check_output(['powershell','-NoProfile','-Command',"Get-CimInstance Win32_Process | Where-Object Name -eq 'worldserver.exe' | Select-Object -ExpandProperty ProcessId"],text=True) if os.name=='nt' else subprocess.run(['pgrep','-x','worldserver'],capture_output=True,text=True).stdout)
 if out.strip():raise RuntimeError('Stop worldserver gracefully before this operation; no live rename is supported')
def owned_accounts():
 return q(f"SELECT a.id,a.username,t.account_type FROM {P}auth.account a JOIN {P}playerbots.playerbots_account_type t ON t.account_id=a.id WHERE a.username REGEXP '^RNDBOT[0-9]+$' AND t.account_type IN(0,1,2) AND NOT EXISTS(SELECT 1 FROM {P}auth.account_access x WHERE x.id=a.id AND x.gmlevel>0) ORDER BY a.id")
def pool_rows():
 rows=list(csv.DictReader(a.csv.open(encoding='utf-8',newline='')));seen=set()
 for x in rows:
  assert re.fullmatch('[A-Z][a-z]{1,11}',x['name']) and x['name'].lower() not in seen;seen.add(x['name'].lower());assert 1<=int(x['weight'])<=1000 and x['tier'] in 'ABC'
  x['gender_id']={'neutral':0,'feminine':1,'masculine':2}[x['gender_hint']];x['class_id']=CLASSES.get(x['class_hint'],0);x['race_id']=RACES.get(x['race_hint'],0)
 return rows
def digest(rows):return hashlib.sha256(json.dumps(rows,separators=(',',':')).encode()).hexdigest()
def fingerprint():
 columns=[x[0] for x in q('SHOW COLUMNS FROM characters')];ni=columns.index('name');ai=columns.index('account');owned={x[0] for x in owned_accounts()};rows=q('SELECT * FROM characters ORDER BY guid');bots=[r[:ni]+r[ni+1:] for r in rows if r[ai] in owned];humans=[r for r in rows if r[ai] not in owned]
 tables=['character_inventory','item_instance','character_spell','character_queststatus','character_queststatus_rewarded','auctionhouse','mail','guild_member','group_member']
 checks=q('CHECKSUM TABLE '+','.join('`'+t+'`' for t in tables))
 return dict(bot_rows_except_name=digest(bots),protected_character_rows=digest(humans),dependent_checksums=checks,bot_count=len(bots),protected_count=len(humans))
def backup():
 offline();dest=a.out/'before-naming-databases.sql.gz'
 if dest.exists():raise RuntimeError('Backup already exists; preserve it and choose a new --out for another backup')
 h,port,user,password,_=d.connections['World'];client=a.client.with_name('mariadb-dump.exe' if a.client.suffix.lower()=='.exe' else 'mariadb-dump');env=dict(os.environ,MYSQL_PWD=password)
 invalid=[];definitions=[]
 for schema,view in q(f"SELECT TABLE_SCHEMA,TABLE_NAME FROM information_schema.VIEWS WHERE TABLE_SCHEMA IN('{P}auth','{P}characters','{P}playerbots')"):
  assert re.fullmatch(r'[A-Za-z0-9_]+',view)
  try:q(f'SHOW FIELDS FROM `{schema}`.`{view}`')
  except RuntimeError:
   definition=q(f'SHOW CREATE VIEW `{schema}`.`{view}`')[0][1];definitions.append('USE `'+schema+'`;\n'+definition+';');invalid.append(schema+'.'+view)
 (a.out/'invalid-view-definitions.sql').write_text('\n'.join(definitions)+'\n',encoding='utf-8')
 args=[str(client),'--no-defaults','--protocol=TCP','--host='+h,'--port='+port,'--user='+user,'--single-transaction','--skip-lock-tables','--routines','--events','--triggers','--hex-blob']+['--ignore-table='+v for v in invalid]+['--databases',P+'auth',P+'characters',P+'playerbots']
 with gzip.open(dest,'wb') as f:
  proc=subprocess.Popen(args,stdout=subprocess.PIPE,stderr=subprocess.PIPE,env=env);shutil.copyfileobj(proc.stdout,f);err=proc.stderr.read();code=proc.wait()
 if code:raise RuntimeError(err.decode(errors='replace').replace(password,'[redacted]')[:300])
 for n in ['worldserver.conf','authserver.conf','playerbots.conf']:
  shutil.copy2(R/'exports'/n,a.out/('before-'+n))
 (a.out/'backup.json').write_text(json.dumps(dict(file=dest.name,bytes=dest.stat().st_size,sha256=hashlib.sha256(dest.read_bytes()).hexdigest(),invalid_views_preserved_separately=invalid),indent=2));print('Consistent offline database backup saved',dest.stat().st_size,'compressed bytes')
def import_pool():
 rows=pool_rows();reserved={x[0].lower() for x in d.q('Character','SELECT name FROM reserved_name')};disabled=[]
 q((R/'sql/updates/characters/2026_11_12_00_characters_curated_playerbot_names.sql').read_text())
 for i in range(0,len(rows),400):
  values=[]
  for x in rows[i:i+400]:
   enabled=not (re.search(r'(.)\1\1',x['name'].lower()) or x['name'].lower() in reserved or x['category']=='soft_fantasy')
   if not enabled:disabled.append(x['name'])
   values.append('('+','.join([lit(x['name']),lit(x['category']),lit(x['tier']),str(x['gender_id']),str(x['class_id']),str(x['race_id']),str(int(x['weight'])),str(int(enabled)),lit(sha)])+')')
  q('INSERT IGNORE INTO playerbot_curated_names(name,category,tier,gender_hint,class_hint,race_hint,weight,enabled,import_sha) VALUES '+','.join(values))
 for ident,name,_ in owned_accounts():q(f"INSERT IGNORE INTO playerbot_roster_accounts(account_id,username,provenance) VALUES({ident},{lit(name)},'prior-bot-type-registry')")
 (a.out/'import.json').write_text(json.dumps(dict(csv_sha256=sha,imported=len(rows),disabled=len(disabled),disabled_candidates=disabled),indent=2));print('Imported',len(rows),'candidates;',len(disabled),'disabled by validation/style; ownership recorded')
def plan():
 file=a.out/'rename-plan.json'
 if file.exists():
  r=json.loads(file.read_text());assert r['csv_sha256']==sha;print('Reusing durable plan with',len(r['renames']),'entries');return r
 accounts=owned_accounts();ids=','.join(x[0] for x in accounts);bots=q('SELECT guid,account,name,race,class,gender FROM characters WHERE account IN('+ids+') ORDER BY guid');existing={x[0].lower() for x in q('SELECT name FROM characters WHERE name IS NOT NULL')};claimed={x[0]:x[1] for x in q('SELECT name,owner_guid FROM playerbot_curated_names WHERE owner_guid<>0')};enabled={r[0] for r in q('SELECT name FROM playerbot_curated_names WHERE enabled=1 AND owner_guid=0')};available=[x for x in pool_rows() if x['name'].lower() not in existing and x['name'] in enabled];rng=random.Random(sha);renames=[];skipped=0
 for guid,account,old,race,cls,gender in bots:
  if claimed.get(old)==guid:skipped+=1;continue
  roll=rng.randrange(100);sex=1 if gender=='1' else 2
  preferred=[x for x in available if (x['class_id']==int(cls) if roll<15 else x['race_id']==int(race) if roll<20 else x['gender_id']==sex if roll<(45 if gender=='1' else 35) else x['gender_id']==0 and x['class_id']==0 and x['race_id']==0)]
  bucket=preferred or available
  if not bucket:raise RuntimeError('Pool exhausted; no generated names')
  pick=rng.choices(bucket,weights=[int(x['weight']) for x in bucket],k=1)[0];available.remove(pick);renames.append(dict(guid=int(guid),account=int(account),old=old,new=pick['name']))
 r=dict(csv_sha256=sha,accounts=len(accounts),confirmed_bot_characters=len(bots),already_curated=skipped,renames=renames);file.write_text(json.dumps(r,indent=2))
 with (a.out/'rename-plan.csv').open('w',encoding='utf-8',newline='') as f:
  w=csv.DictWriter(f,fieldnames=['guid','account','old','new']);w.writeheader();w.writerows(renames)
 print('Dry-run plan:',len(renames),'renames;',skipped,'already-curated characters preserved');return r
def rename(rollback=False):
 offline();assert (a.out/'backup.json').exists(),'Verified database backup required';r=plan();before=fingerprint();lines=['START TRANSACTION;'];current_rows={int(x[0]):x[1:] for x in q('SELECT guid,account,name FROM characters')};claims={x[0]:x[1:] for x in q('SELECT name,owner_guid,owner_account FROM playerbot_curated_names')};ownership={int(x[0]) for x in owned_accounts()};name_owners={}
 for ident,(acct,nm) in current_rows.items():name_owners.setdefault(nm.lower(),[]).append(ident)
 for x in r['renames']:
  old,new=(x['new'],x['old']) if rollback else (x['old'],x['new']);current=[current_rows[x['guid']]];assert x['account'] in ownership
  assert current and current[0][0]==str(x['account']) and current[0][1] in [old,new],f'Ownership/name changed for GUID {x["guid"]}'
  conflicts=[g for g in name_owners.get(new.lower(),[]) if g!=x['guid']];assert not conflicts,'Target name now belongs to another character; rollback/apply refused'
  if not rollback:
   claim=[claims[new]];assert claim[0][0] in ['0',str(x['guid'])]
   lines.append(f"UPDATE playerbot_curated_names SET owner_guid={x['guid']},owner_account={x['account']},reserved_at=CURRENT_TIMESTAMP WHERE name={lit(new)};")
   lines.append(f"INSERT INTO playerbot_name_renames(guid,account_id,old_name,new_name,import_sha,state) VALUES({x['guid']},{x['account']},{lit(old)},{lit(new)},{lit(sha)},'planned') ON DUPLICATE KEY UPDATE account_id=VALUES(account_id);")
  lines.append(f"UPDATE characters SET name={lit(new)} WHERE guid={x['guid']} AND account={x['account']} AND name={lit(old)};")
  state='rolled_back' if rollback else 'applied';lines.append(f"UPDATE playerbot_name_renames SET state='{state}',applied_at=CURRENT_TIMESTAMP WHERE guid={x['guid']};")
  if rollback:lines.append(f"UPDATE playerbot_curated_names SET owner_guid=0,owner_account=0 WHERE name={lit(old)} AND owner_guid={x['guid']};")
 lines.append('COMMIT;');q('\n'.join(lines));after=fingerprint();assert before==after,'A protected row or dependent table changed';label='rollback' if rollback else 'rename';(a.out/(label+'-verification.json')).write_text(json.dumps(dict(before=before,after=after,protected_identical=True,renames=len(r['renames'])),indent=2));print(label,':',len(r['renames']),'names; all protected fingerprints unchanged')
def verify():
 owned=owned_accounts();ids=','.join(x[0] for x in owned);r=dict(accounts=len(owned),stored_bots=int(q('SELECT COUNT(*) FROM characters WHERE account IN('+ids+')')[0][0]),online_bots=int(q('SELECT COUNT(*) FROM characters WHERE online<>0 AND account IN('+ids+')')[0][0]),oversized_accounts=q('SELECT account,COUNT(*) FROM characters WHERE account IN('+ids+') GROUP BY account HAVING COUNT(*)>11'),duplicates=q('SELECT LOWER(name),COUNT(*) FROM characters WHERE account IN('+ids+') GROUP BY LOWER(name) HAVING COUNT(*)>1'),curated_bot_names=int(q('SELECT COUNT(*) FROM characters c JOIN playerbot_curated_names n ON c.name=n.name AND c.guid=n.owner_guid AND c.account=n.owner_account WHERE c.account IN('+ids+')')[0][0]));(a.out/'roster-verification.json').write_text(json.dumps(r,indent=2));print(json.dumps(r))
if a.mode in ['backup','import','rename','rollback'] and not a.apply:
 print('Dry-run only. Use --apply to execute',a.mode);sys.exit(0)
# OS lock is released automatically on interruption; no stale lock-file removal needed.
with (a.out/'migration.lock').open('a+b') as lock:
 lock.seek(0);lock.write(b'0');lock.flush();lock.seek(0)
 if os.name=='nt':
  import msvcrt
  msvcrt.locking(lock.fileno(),msvcrt.LK_NBLCK,1)
 else:
  import fcntl
  fcntl.flock(lock.fileno(),fcntl.LOCK_EX|fcntl.LOCK_NB)
 if a.mode=='backup':backup()
 elif a.mode=='import':import_pool()
 elif a.mode=='plan':plan()
 elif a.mode=='rename':rename()
 elif a.mode=='rollback':rename(True)
 else:verify()
