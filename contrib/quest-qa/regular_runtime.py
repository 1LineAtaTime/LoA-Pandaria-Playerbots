"""Validate the existing regular LoA exports runtime without changing accounts."""
import hashlib,json,re,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'contrib/headless-client'))
from starter_campaign import Database


def validate():
    runtime=ROOT/'exports'
    world=(runtime/'worldserver.conf').read_text(encoding='utf-8-sig')
    auth=(runtime/'authserver.conf').read_text(encoding='utf-8-sig')
    def setting(text,name):
        found=re.search(r'^\s*'+re.escape(name)+r'\s*=\s*(.*?)\s*$',text,re.M)
        if not found:raise RuntimeError('Missing regular runtime setting: '+name)
        return found[1].strip('"')
    if setting(auth,'RealmServerPort')!='3724' or setting(world,'WorldServerPort')!='8085':
        raise RuntimeError('Expected the original regular LoA ports3724/8085')
    if setting(auth,'LoginDatabaseInfo')!=setting(world,'LoginDatabaseInfo'):
        raise RuntimeError('Regular auth/world configurations point at different login databases')
    db=Database(runtime/'worldserver.conf',ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
    prefix=db.connections['Login'][4].removesuffix('auth')
    if prefix not in ('mop_','mop_loa_') or any(v[4]!=prefix+{'Login':'auth','Character':'characters','World':'world'}[k] for k,v in db.connections.items()):
        raise RuntimeError('Regular runtime must use one consistent mop_ or mop_loa_ database set')
    for connection,suffix in (('Playerbots','playerbots'),('Archive','log')):
        fields=setting(world,connection+'DatabaseInfo').split(';')
        if len(fields)!=5 or fields[4]!=prefix+suffix:
            raise RuntimeError('Regular '+connection+' database must use the selected database prefix')
    manifest=json.loads((runtime/'ACTIVE-RUNTIME-MANIFEST.json').read_text(encoding='utf-8-sig'))
    expected=next(a['sha256'] for a in manifest['artifacts'] if a['name']=='worldserver.exe')
    if hashlib.sha256((runtime/'worldserver.exe').read_bytes()).hexdigest()!=expected:
        raise RuntimeError('Regular worldserver differs from its recorded deployment manifest')
    realm_id=int(setting(world,'RealmID'))
    rows=db.q('Login',f'SELECT name,address,port,gamebuild FROM realmlist WHERE id={realm_id}')
    if len(rows)!=1 or rows[0][2:]!=['8085','18414']:
        raise RuntimeError('Regular realm entry does not match the expected world port/build')
    return dict(runtime=runtime,db=db,realm_name=rows[0][0],realm_id=realm_id)


if __name__=='__main__':
    validate();print('Configured regular runtime validated on3724/8085; no account data changed.')
