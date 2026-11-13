"""Recover only journal-owned disposable QA accounts; never sweep an account-name prefix."""
import argparse
import json
import re
import subprocess
import sys
import time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'contrib/headless-client'))
from starter_campaign import Database
from evidence import write_json
from checkpoint import private_path


def recover(db,directory,command):
    results=[]
    def absent(kind,query):
        for _ in range(30):
            if not db.q(kind,query):return True
            time.sleep(.2)
        return False
    for path in sorted(Path(directory).glob('trace-*.json')):
        journal=json.loads(path.read_text());account=journal.get('account','');name=journal.get('name','');ident=journal.get('account_id')
        if journal.get('account_created') is not True:
            if re.fullmatch(r'QZ[A-Z]{6,10}',account) and db.q('Login',"SELECT id FROM account WHERE username='"+account+"'"):
                raise ValueError('Planned account exists without confirmed journal ownership; manual review required')
            continue
        if not re.fullmatch(r'QZ[A-Z]{6,10}',account) or name.upper()!=account or type(ident) is not int or ident<=0:
            raise ValueError('Journal lacks verified ownership; manual review required: '+path.name)
        current=db.q('Login',"SELECT id FROM account WHERE username='"+account+"'")
        if current and int(current[0][0])!=ident:
            raise ValueError('Account identity changed; refusing recovery')
        characters=db.q('Character',f'SELECT guid,name FROM characters WHERE account={ident}') if current else []
        if any(row[1].casefold()!=name.casefold() for row in characters):
            raise ValueError('Unexpected character ownership; refusing recovery')
        if characters and (type(journal.get('guid')) is not int or any(int(r[0])!=journal['guid'] for r in characters)):
            raise ValueError('Live character identity is unverified; refusing recovery')
        if current:command('account delete '+account)
        assert absent('Login',"SELECT id FROM account WHERE username='"+account+"'"),'Account cleanup not acknowledged'
        # A reused character name belonging to somebody else must never be deleted.
        soft=db.q('Character',"SELECT guid FROM characters WHERE deleteInfos_Name='"+name+"'")
        owned_guid=journal.get('guid')
        if soft and (type(owned_guid) is not int or any(int(r[0])!=owned_guid for r in soft)):
            raise ValueError('Soft-deleted character identity is unverified; refusing hard delete')
        if soft:command('character deleted delete '+name)
        clean=absent('Character',"SELECT guid FROM characters WHERE name='"+name+"' OR deleteInfos_Name='"+name+"'")
        if not clean:raise RuntimeError('Character cleanup not acknowledged')
        results.append(dict(journal=path.name,account=account,account_absent=True,character_absent=True))
    return results


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--journal-dir',type=Path,required=True);p.add_argument('--world-pid',type=int,required=True)
    p.add_argument('--config',type=Path,default=ROOT/'exports/worldserver.conf')
    p.add_argument('--client',type=Path,default=ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
    a=p.parse_args();directory=private_path(ROOT,a.journal_dir)
    def command(text):subprocess.run([sys.executable,str(ROOT/'contrib/windows/Send-ServerConsoleCommand.py'),str(a.world_pid),text],check=True)
    results=recover(Database(a.config,a.client),directory,command)
    write_json(directory/'recovery.json',dict(verified_owned_accounts=results,passed=True))
    print(json.dumps(dict(recovered_or_already_clean=len(results),passed=True)))


if __name__=='__main__':main()
