"""Provision an isolated, owned QA runtime; never clone personal account/character rows.

Existing production schemas are SELECT-only inputs. Credentials stay under out/.
No server is started/stopped by this provisioning command.
"""
import hashlib
import json
import os
from pathlib import Path
import re
import secrets
import shutil
import subprocess

ROOT=Path(__file__).resolve().parents[2]
RUNTIME=ROOT/'out/quest-qa-runtime'
DOMAINS=('auth','characters','world','playerbots','log')
PREFIX='mop_loa_qa_'
REFERENCES={
 'auth':set('rbac_permissions rbac_default_permissions rbac_linked_permissions build_info realm_classes realm_races realmlist realmlist_proxy warden_checks warden_action addons banned_addons battlepay_schema sql_update updates updates_include version'.split()),
 'characters':set('addons banned_addons ai_playerbot_names playerbots_names playerbots_guild_names playerbots_arena_team_names reserved_name battlepay_schema updates updates_include'.split()),
 'playerbots':{'ai_playerbot_texts'}, 'log':set()}


def replace_setting(text,key,value):
    pattern=r'^\s*'+re.escape(key)+r'\s*=.*$'
    line=key+' = '+str(value)
    return re.sub(pattern,lambda _:line,text,flags=re.M) if re.search(pattern,text,re.M) else text+'\n'+line+'\n'


def provision():
    password=os.environ.get('MOP_QA_ADMIN_PASSWORD')
    if not password:raise RuntimeError('Set MOP_QA_ADMIN_PASSWORD privately for schema provisioning')
    source_config=(ROOT/'exports/worldserver.conf').read_text(encoding='utf-8-sig')
    connections={k:re.search(r'^'+k+r'DatabaseInfo\s*=\s*"([^"]+)"',source_config,re.M)[1].split(';') for k in ('World','Character','Login','Playerbots','Archive')}
    host,port,_,_,_=connections['World']
    client=ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe'
    command=[str(client),'--no-defaults','--protocol=TCP','--host='+host,'--port='+port,'--user='+os.environ.get('MOP_QA_ADMIN_USER','root'),'--batch','--skip-column-names','--default-character-set=utf8mb4']
    def q(sql):
        marker='__QA_PROVISION_DONE__'
        for attempt in range(3):
            p=subprocess.run(command,input=sql.rstrip(';')+"; SELECT '"+marker+"';",text=True,encoding='utf-8',capture_output=True,env=dict(os.environ,MYSQL_PWD=password),timeout=1800)
            if p.returncode:raise RuntimeError(p.stderr.replace(password,'[redacted]')[-900:])
            if p.stdout.rstrip().endswith(marker):return p.stdout.rsplit(marker,1)[0].strip()
            if not sql.lstrip().upper().startswith('SELECT'):
                raise RuntimeError('Incomplete mutation response; inspect the owned QA target before resuming')
        raise RuntimeError('Incomplete database read during QA provisioning')
    RUNTIME.mkdir(parents=True,exist_ok=True)
    path=RUNTIME/'environment.json'
    if path.exists():
        manifest=json.loads(path.read_text())
        if manifest.get('prefix')!=PREFIX:raise RuntimeError('QA ownership manifest differs')
        if manifest.get('state')=='ready':
            print(json.dumps(dict(state='already_ready',runtime=str(RUNTIME),schemas=manifest['schemas'])));return
    else:
        existing=q("SELECT SCHEMA_NAME FROM information_schema.SCHEMATA WHERE SCHEMA_NAME LIKE 'mop\\_loa\\_qa\\_%'")
        if existing:raise RuntimeError('QA schemas already exist without an ownership manifest; refusing overwrite')
        manifest=dict(owner=secrets.token_hex(16),prefix=PREFIX,schemas=[PREFIX+d for d in DOMAINS],state='provisioning',completed=[],production_account_rows_copied=0,production_character_rows_copied=0)
        path.write_text(json.dumps(manifest,indent=2))
    source_for={'auth':connections['Login'][4],'characters':connections['Character'][4],'world':connections['World'][4],'playerbots':connections['Playerbots'][4],'log':connections['Archive'][4]}
    for domain in DOMAINS:
        if domain in manifest['completed']:continue
        target=PREFIX+domain;source=source_for[domain]
        if not re.fullmatch(r'[a-zA-Z0-9_]+',source):raise ValueError('Unexpected source schema')
        q(f'CREATE DATABASE IF NOT EXISTS `{target}` CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;')
        tables=q(f"SELECT TABLE_NAME FROM information_schema.TABLES WHERE TABLE_SCHEMA='{source}' AND TABLE_TYPE='BASE TABLE' ORDER BY TABLE_NAME").splitlines()
        commands=[]
        done=set(manifest.get('tables_completed',{}).get(domain,[]))
        for table in tables:
            if table in done or 'backup' in table.lower() or table.startswith('auth_cleanup_'):continue
            if not re.fullmatch(r'[a-zA-Z0-9_]+',table):raise ValueError('Unexpected table name')
            exists=q(f"SELECT COUNT(*) FROM information_schema.TABLES WHERE TABLE_SCHEMA='{target}' AND TABLE_NAME='{table}'")
            if exists!='0':raise RuntimeError('Interrupted table copy requires inspection: '+target+'.'+table)
            sql=f'CREATE TABLE `{target}`.`{table}` LIKE `{source}`.`{table}`;'
            if domain=='world' or table in REFERENCES.get(domain,set()):
                sql+=f' INSERT INTO `{target}`.`{table}` SELECT * FROM `{source}`.`{table}`;'
            q(sql)
            manifest.setdefault('tables_completed',{}).setdefault(domain,[]).append(table)
            path.write_text(json.dumps(manifest,indent=2))
        manifest['completed'].append(domain);path.write_text(json.dumps(manifest,indent=2))
        print('QA schema populated: '+target,flush=True)
    private=RUNTIME/'credentials.json'
    if private.exists():credentials=json.loads(private.read_text())
    else:
        credentials=dict(user='mop_qa_'+manifest['owner'][:8],password=secrets.token_hex(24))
        private.write_text(json.dumps(credentials))
    address=q("SELECT SUBSTRING_INDEX(USER(),'@',-1)")
    if not re.fullmatch(r'[a-zA-Z0-9_.:-]+',address):raise ValueError('Unexpected database client address')
    for address in sorted({address,'localhost','127.0.0.1'}):
        q("CREATE USER IF NOT EXISTS '"+credentials['user']+"'@'"+address+"' IDENTIFIED BY '"+credentials['password']+"';")
        for schema in manifest['schemas']:q(f"GRANT ALL PRIVILEGES ON `{schema}`.* TO '"+credentials['user']+"'@'"+address+"';")
    q(f"UPDATE `{PREFIX}auth`.realmlist SET name='ExampleRealm QA (isolated)',address='127.0.0.1',localAddress='127.0.0.1',port=8086,allowedSecurityLevel=0,flag=(flag & ~1);")
    if q(f'SELECT COUNT(*) FROM `{PREFIX}auth`.account')!='0' or q(f'SELECT COUNT(*) FROM `{PREFIX}characters`.characters')!='0':
        raise RuntimeError('Fresh QA must not contain imported accounts or characters')
    for name in ('worldserver.conf','authserver.conf','playerbots.conf'):
        text=(ROOT/'exports'/name).read_text(encoding='utf-8-sig')
        for kind,domain in [('Login','auth'),('World','world'),('Character','characters'),('Playerbots','playerbots'),('Archive','log')]:
            text=replace_setting(text,kind+'DatabaseInfo','"'+';'.join([host,port,credentials['user'],credentials['password'],PREFIX+domain])+'"')
        for key,value in {'BindIP':'"127.0.0.1"','LogsDir':'"./logs"'}.items():text=replace_setting(text,key,value)
        if name=='worldserver.conf':
            settings={'WorldServerPort':8086,'DataDir':'"../../exports"','SOAP.Enabled':0,'Ra.Enable':0,'Console.Enable':1,'CharactersPerAccount':11,'CharactersPerRealm':11,'AuctionHouseBot.EnableSeller':0,'AuctionHouseBot.EnableBuyer':0,'AuctionHouseBot.EnableBuyerNeutral':0,'AuctionHouseBot.Seller.Enabled':0,'AuctionHouseBot.Buyer.Enabled':0}
            for key,value in settings.items():text=replace_setting(text,key,value)
        if name=='authserver.conf':text=replace_setting(text,'RealmServerPort',3725)
        if name=='playerbots.conf':
            for key,value in {'AiPlayerbot.Enabled':0,'AiPlayerbot.RandomBotAutologin':0,'AiPlayerbot.RandomBotAccountCount':0,'AiPlayerbot.DeleteRandomBotAccounts':0}.items():text=replace_setting(text,key,value)
        (RUNTIME/name).write_text(text,encoding='utf-8')
    for name in ('worldserver.exe','authserver.exe'):
        shutil.copy2(ROOT/'exports'/name,RUNTIME/name)
    # Only audited NEW/OLD row-alias assignments are retained,never arbitrary
    # source triggers/routines that could address the played schemas.
    from reference_schema import install
    install(RUNTIME)
    for p in (ROOT/'exports').glob('*.dll'):shutil.copy2(p,RUNTIME/p.name)
    (RUNTIME/'logs').mkdir(exist_ok=True)
    for executable,label in [('authserver','QA Authserver'),('worldserver','QA Worldserver')]:
        (RUNTIME/('Start-'+executable+'.cmd')).write_text('@echo off\ncd /d "%~dp0"\nstart "'+label+'" /D "%~dp0" "%~dp0'+executable+'.exe" -c "%~dp0'+executable+'.conf"\nexit /b\n')
    manifest.update(state='ready',auth_port=3725,world_port=8086,bind='127.0.0.1',source_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),source_world=source_for['world'],production_restarted=False,
                    consistency='Table-by-table public world copy; personal auth/character rows excluded. No cross-table atomic snapshot claimed.')
    path.write_text(json.dumps(manifest,indent=2)+'\n')
    print(json.dumps(dict(state='ready',runtime=str(RUNTIME),schemas=manifest['schemas'],personal_rows_imported=0)))


if __name__=='__main__':provision()
