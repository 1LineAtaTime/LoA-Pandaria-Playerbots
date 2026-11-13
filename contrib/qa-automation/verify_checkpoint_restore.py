"""Exercise scoped SQL rollback on a uniquely owned disposable table, then remove it."""
import argparse
import json
import sys
import uuid
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'contrib/headless-client'))
from starter_campaign import Database
from checkpoint import database_backups,private_path,sha


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--config',type=Path,default=ROOT/'exports/worldserver.conf')
    p.add_argument('--client',type=Path,default=ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
    p.add_argument('--run-dir',type=Path,required=True)
    a=p.parse_args();out=private_path(ROOT,a.run_dir);out.mkdir(parents=True,exist_ok=False)
    db=Database(a.config,a.client);table='qa_checkpoint_'+uuid.uuid4().hex[:16];created=False
    report=dict(table=table,passed=False,fixture_only=True)
    try:
        db.q('World',f'CREATE TABLE `{table}` (id INT NOT NULL PRIMARY KEY,value VARCHAR(80) NOT NULL) ENGINE=InnoDB')
        created=True
        db.q('World',f"INSERT INTO `{table}` VALUES (1,'before'),(3,'untouched')")
        backup=database_backups(db,[dict(database='World',table=table,key='id',ids=[1,2])],out)[0]
        db.q('World',f"UPDATE `{table}` SET value='changed' WHERE id=1; INSERT INTO `{table}` VALUES (2,'new after checkpoint')")
        db.q('World',(out/backup['restore_file']).read_text(encoding='utf-8'))
        actual=db.q('World',f'SELECT id,value FROM `{table}` ORDER BY id')
        assert actual==[['1','before'],['3','untouched']],actual
        report.update(passed=True,backup=backup,restored_existing_row=True,removed_new_scoped_row=True,unselected_row_preserved=True)
    finally:
        if created:
            db.q('World',f'DROP TABLE `{table}`')
            report['fixture_removed']=not db.q('World',"SELECT TABLE_NAME FROM information_schema.TABLES WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME='"+table+"'")
        (out/'restore-verification.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(report))
    return int(not report['passed'] or not report.get('fixture_removed'))


if __name__=='__main__':raise SystemExit(main())
