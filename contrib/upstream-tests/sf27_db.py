"""Minimal local MariaDB reader for the reproducible SF27 data tools."""
from pathlib import Path
import argparse,re,subprocess,os,types

def load_db():
 parser=argparse.ArgumentParser()
 parser.add_argument('--runtime-root',type=Path,default=Path(__file__).resolve().parents[2])
 parser.add_argument('--config',type=Path)
 parser.add_argument('--client',type=Path)
 args=parser.parse_args();root=args.runtime_root.resolve()
 config=args.config or root/'exports/worldserver.conf'
 client=args.client or root/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe'
 text=config.read_text(encoding='utf-8-sig');match=re.search(r'^WorldDatabaseInfo\s*=\s*"([^"]+)"',text,re.M)
 if not match:raise RuntimeError('WorldDatabaseInfo is missing')
 host,port,user,password,database=match[1].split(';')
 command=[str(client.resolve()),'--no-defaults','--protocol=TCP','--host='+host,'--port='+port,'--user='+user,'--database='+database,'--batch','--skip-column-names','--default-character-set=utf8mb4']
 def q(sql,timeout=180):
  marker='__CODEX_SF27_DB_COMPLETED__'
  payload=sql.rstrip().rstrip(';')+";\nSELECT '"+marker+"';\n"
  for attempt in range(3):
   result=subprocess.run(command,input=payload,text=True,encoding='utf-8',capture_output=True,timeout=timeout,env=dict(os.environ,MYSQL_PWD=password))
   if result.returncode:raise RuntimeError(result.stderr.replace(password,'[REDACTED]')[:500]+'\n...\n'+result.stderr.replace(password,'[REDACTED]')[-1200:])
   if result.stdout.rstrip().endswith(marker):return result.stdout.rsplit(marker,1)[0].rstrip('\r\n')
  raise RuntimeError('MariaDB client returned an incomplete response without its completion marker')

 return types.SimpleNamespace(q=q,database=database,runtime_root=root)
