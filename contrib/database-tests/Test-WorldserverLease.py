"""Check an already-running world rejects a second process on the same character DB.
Uses a private config copy under ignored out/; never starts a second gameplay realm.
"""
import argparse,json,re,subprocess,sys,time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'contrib/headless-client'))
from starter_campaign import Database


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--config',type=Path,default=ROOT/'exports/worldserver.conf')
    parser.add_argument('--exe',type=Path,default=ROOT/'exports/worldserver.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'out/database-tests/world-lease')
    args=parser.parse_args()
    db=Database(args.config,ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
    holder=db.q('Character',"SELECT IS_USED_LOCK(CONCAT('loa-world:',SHA1(DATABASE())))")
    if not holder or holder[0][0]=='NULL':
        raise RuntimeError('Start the updated regular worldserver first; refusing an unguarded second startup')
    args.output.mkdir(parents=True,exist_ok=True)
    config=args.output/'private-worldserver.conf'
    text=args.config.read_text(encoding='utf-8-sig')
    text=re.sub(r'^LogsDir\s*=.*$',lambda _: 'LogsDir = "'+str(args.output.resolve()).replace('\\','/')+'"',text,flags=re.M)
    config.write_text(text,encoding='utf-8')
    before=db.q('Login','SELECT id,flag FROM realmlist ORDER BY id')
    started=time.monotonic()
    result=subprocess.run([str(args.exe.resolve()),'-c',str(config.resolve())],cwd=args.exe.resolve().parent,capture_output=True,timeout=60)
    log=(args.output/'Server.log').read_text(errors='replace')
    assert result.returncode==1,'Duplicate process did not exit cleanly with startup failure'
    assert 'Cannot acquire exclusive character database lease' in log,'No lease rejection diagnostic'
    assert db.q('Character',"SELECT IS_USED_LOCK(CONCAT('loa-world:',SHA1(DATABASE())))")==holder,'Original lease owner changed'
    assert db.q('Login','SELECT id,flag FROM realmlist ORDER BY id')==before,'Rejected duplicate changed realm state'
    report=dict(passed=True,duplicate_world_rejected=True,clean_process_exit=True,original_lease_owner_preserved=True,realm_flags_preserved=True,elapsed_seconds=round(time.monotonic()-started,2))
    (args.output/'result.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report))


if __name__=='__main__':main()
