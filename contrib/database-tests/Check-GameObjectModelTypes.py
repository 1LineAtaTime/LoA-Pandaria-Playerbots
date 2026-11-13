"""Read-only build18414 WMO/gameobject-type compatibility audit.

The original client excludes types11/14/15/33 from CGGameObject's M2 loader.
Sending a WMO as generic type5 reached a null M2 reference release in a real
client dump. This audit checks the world template against the actual DBC path;
it does not claim that the client has rendered or that all asset files exist.
"""
import argparse
import json
from pathlib import Path
import struct
import sys

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'contrib/headless-client'))
from starter_campaign import Database

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--config',type=Path,default=ROOT/'exports/worldserver.conf')
    p.add_argument('--client',type=Path,default=ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
    p.add_argument('--dbc',type=Path,default=ROOT/'exports/dbc/GameObjectDisplayInfo.dbc')
    p.add_argument('--entries',type=int,nargs='+',help='Scope to these template entries; omitted means all templates')
    p.add_argument('--report',type=Path)
    args=p.parse_args()
    b=args.dbc.read_bytes()
    magic,count,fields,size,strings=struct.unpack_from('<4s4I',b)
    if magic!=b'WDBC' or fields<2 or size<8 or len(b)<20+count*size+strings:
        raise ValueError('Invalid GameObjectDisplayInfo DBC')
    text=b[20+count*size:20+count*size+strings];models={}
    for i in range(count):
        display,offset=struct.unpack_from('<II',b,20+i*size)
        if offset>=len(text):raise ValueError('Invalid model string offset')
        models[display]=text[offset:].split(b'\0',1)[0].decode('utf-8',errors='replace')
    db=Database(args.config,args.client)
    where=' WHERE entry IN ('+','.join(str(x) for x in args.entries)+')' if args.entries else ''
    rows=db.q('World','SELECT entry,type,displayId FROM gameobject_template'+where)
    failures=[];wmo_count=0
    for entry,kind,display in rows:
        path=models.get(int(display),'')
        if path.lower().endswith('.wmo'):
            wmo_count+=1
            if int(kind) not in (11,14,15,33):
                failures.append(dict(entry=int(entry),type=int(kind),display=int(display),model=path))
    missing=sorted(set(args.entries or [])-{int(row[0]) for row in rows})
    report=dict(scope=args.entries or 'all templates',templates_checked=len(rows),wmo_templates=wmo_count,
                incompatible_types=failures,missing_entries=missing,passed=not failures and not missing,
                limitation='Static compatibility only; real-client rendering is a separate check')
    if args.report:
        args.report.parent.mkdir(parents=True,exist_ok=True)
        args.report.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({k:v for k,v in report.items() if k!='incompatible_types'},indent=2))
    print('Incompatible WMO templates:',len(failures))
    return 0 if report['passed'] else 1

if __name__=='__main__':sys.exit(main())
