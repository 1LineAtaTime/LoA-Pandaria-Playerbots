"""Verify native cleanup of one deliberately journalled, empty disposable account."""
import argparse
import hashlib
import json
import secrets
import string
import subprocess
import sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'contrib/headless-client'))
from starter_campaign import Database
from checkpoint import private_path
from recover_accounts import recover
from evidence import write_json


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--run-dir',type=Path,required=True);p.add_argument('--world-pid',type=int,required=True)
    a=p.parse_args();directory=private_path(ROOT,a.run_dir);directory.mkdir(parents=True,exist_ok=False)
    db=Database(ROOT/'exports/worldserver.conf',ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
    account='QZ'+''.join(secrets.choice(string.ascii_uppercase) for _ in range(8));name=account.capitalize()
    assert not db.q('Login',"SELECT id FROM account WHERE username='"+account+"'")
    digest=hashlib.sha1((account+':'+secrets.token_hex(16).upper()).encode()).hexdigest()
    db.q('Login',f"INSERT INTO account(username,sha_pass_hash,expansion) VALUES ('{account}','{digest}',4)")
    ident=int(db.q('Login',"SELECT id FROM account WHERE username='"+account+"'")[0][0])
    write_json(directory/'trace-0.json',dict(id='recovery-fixture',account=account,name=name,account_created=True,account_id=ident,guid=None))
    def command(text):subprocess.run([sys.executable,str(ROOT/'contrib/windows/Send-ServerConsoleCommand.py'),str(a.world_pid),text],check=True)
    result=recover(db,directory,command)
    assert len(result)==1 and result[0]['account_absent'] and result[0]['character_absent']
    again=recover(db,directory,command);assert len(again)==1
    write_json(directory/'verification.json',dict(passed=True,owned_account_removed=True,idempotent=True))
    print(json.dumps(dict(passed=True,owned_account_removed=True,idempotent=True)))


if __name__=='__main__':main()
