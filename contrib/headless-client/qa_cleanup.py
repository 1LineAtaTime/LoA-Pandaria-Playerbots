"""Remove only journal-owned disposable identities, character dependencies first."""
import subprocess
import sys
import time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]


def cleanup_owned(db, world_pid, account, name, account_id, guid=None):
    if not account_id or not account.isalnum() or not name.isalpha():
        raise ValueError('Durable account ownership and exact character name required')
    rows=db.q('Login',f"SELECT id,username FROM account WHERE id={int(account_id)}")
    if rows and rows[0][1].upper()!=account.upper():
        raise RuntimeError('Account ID was reused; refusing cleanup')
    def console(command):
        subprocess.run([sys.executable,str(ROOT/'contrib/windows/Send-ServerConsoleCommand.py'),str(world_pid),command],check=True)
    def absent(kind,query,timeout=30):
        for _ in range(int(timeout*2)):
            if not db.q(kind,query):return True
            time.sleep(.5)
        return False
    chars=db.q('Character',f'SELECT guid,name,account,online,deleteInfos_Account,deleteInfos_Name FROM characters WHERE account={int(account_id)}'+(f' OR guid={int(guid)}' if guid else ''))
    for c in chars:
        if not guid or int(c[0])!=int(guid) or (c[1]!=name and c[5]!=name) or int(c[2]) not in (0,int(account_id)):
            raise RuntimeError('Unexpected character ownership; refusing cleanup')
        if int(c[2])==0 and int(c[4])!=int(account_id):
            raise RuntimeError('Deleted character ownership does not match journal')
        # A failed gameplay client can disconnect while the native server retains
        # it for its 60-second grace period. Do not confuse that with orphan data.
        if not absent('Character',f'SELECT guid FROM characters WHERE guid={int(guid)} AND online<>0',timeout=90):
            raise RuntimeError('Character still online; account preserved')
        console(('character erase ' if int(c[2]) else 'character deleted delete ')+name)
        if not absent('Character',f'SELECT guid FROM characters WHERE guid={int(guid)}'):
            raise RuntimeError('Native character removal incomplete; account preserved')
    dependencies={}
    if guid:
        tables=db.q('Character',"SELECT TABLE_NAME FROM information_schema.COLUMNS WHERE TABLE_SCHEMA=DATABASE() AND COLUMN_NAME='guid' AND TABLE_NAME LIKE 'character\\_%'")
        for (table,) in tables:
            if not table.replace('_','').isalnum():raise ValueError('Invalid table identifier')
            dependencies[table]=int(db.q('Character',f'SELECT COUNT(*) FROM `{table}` WHERE guid={int(guid)}')[0][0])
        for table,column in [('item_instance','owner_guid'),('mail','receiver'),('corpse','guid')]:
            dependencies[table]=int(db.q('Character',f'SELECT COUNT(*) FROM `{table}` WHERE `{column}`={int(guid)}')[0][0])
        if any(dependencies.values()):
            raise RuntimeError('Dependent character data remains; account preserved')
    if rows:console('account delete '+account)
    account_absent=absent('Login',f'SELECT id FROM account WHERE id={int(account_id)}')
    return dict(account=account,account_absent=account_absent,character_absent=not guid or not db.q('Character',f'SELECT guid FROM characters WHERE guid={int(guid)}'),
                cleanup_order='character_and_dependencies_before_account',dependencies=dependencies)
