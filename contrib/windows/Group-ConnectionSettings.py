"""Group active connection settings without changing any configuration values.
Run from the repository root: python contrib/windows/Group-ConnectionSettings.py
Backups stay under ignored out/config-layout-backups; they contain credentials.
"""
import argparse,collections,json,re
from datetime import datetime
from pathlib import Path

KEY=re.compile(r'^\s*([A-Za-z][A-Za-z0-9_.]*)\s*=\s*(.*)$')
def assignments(lines):
    return collections.Counter((m[1],m[2]) for line in lines if (m:=KEY.match(line)))
def group(path,backup):
    raw=path.read_bytes();lines=raw.decode('utf-8-sig').splitlines()
    section='['+path.stem+']'
    assert lines.count(section)==1, 'Expected one matching config section'
    keys=['RealmID','WorldServerPort','RealmServerPort','BindIP']
    for db in ['Login','World','Character','Archive','Playerbots']:
        keys += [db+'DatabaseInfo',db+'Database.WorkerThreads',db+'Database.SynchThreads']
    if '# BEGIN GROUPED CONNECTION SETTINGS' in lines:
        begin=lines.index('# BEGIN GROUPED CONNECTION SETTINGS');end=lines.index('# END GROUPED CONNECTION SETTINGS')
        assert all(begin<i<end for i,line in enumerate(lines) if (m:=KEY.match(line)) and m[1] in keys), 'New connection settings outside existing group require review'
        print(json.dumps(dict(file=path.name,already_grouped=True,all_values_unchanged=True)))
        return
    selected={};remaining=[]
    for line in lines:
        m=KEY.match(line)
        if m and m[1] in keys:
            assert m[1] not in selected,'Duplicate active connection key'
            selected[m[1]]=line
        elif line.strip() not in ['# BEGIN GROUPED CONNECTION SETTINGS','# END GROUPED CONNECTION SETTINGS']:
            remaining.append(line)
    header=['','# BEGIN GROUPED CONNECTION SETTINGS',
            '# Realm network and database access. Keep private working configs out of public Git.',
            '# DatabaseInfo format: host;port;user;password;schema.',
            '# Auth listener port is RealmServerPort in authserver.conf.']
    for key in keys:
        if key in selected:header.append(selected[key])
    header += ['# END GROUPED CONNECTION SETTINGS','']
    i=remaining.index(section)+1;new=remaining[:i]+header+remaining[i:]
    assert assignments(lines)==assignments(new),'Configuration values changed'
    backup.mkdir(parents=True,exist_ok=True);(backup/path.name).write_bytes(raw)
    path.write_text('\n'.join(new).rstrip()+'\n',encoding='utf-8')
    print(json.dumps(dict(file=path.name,grouped_keys=list(selected),all_values_unchanged=True)))
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('files',type=Path,nargs='*',default=[Path('exports/worldserver.conf'),Path('exports/authserver.conf')]);p.add_argument('--backup-dir',type=Path,default=Path('out/config-layout-backups')/datetime.now().strftime('%Y%m%dT%H%M%S'));a=p.parse_args()
    for f in a.files:group(f,a.backup_dir)
