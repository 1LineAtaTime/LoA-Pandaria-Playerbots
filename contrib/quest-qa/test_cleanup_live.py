"""Explicit integration runner; only the owned isolated QA realm is addressable."""
import argparse
import hashlib
import json
import secrets
from pool import Pool,ROOT
from qa_cleanup import cleanup_owned
from evidence import write_json
import headless


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--world-pid',type=int,required=True);a=parser.parse_args()
    p=Pool();name=('Qd'+''.join(secrets.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(8))).capitalize()
    account=dict(name=name.upper(),password=secrets.token_hex(8).upper());digest=hashlib.sha1((account['name']+':'+account['password']).encode()).hexdigest()
    p.db.q('Login',f"INSERT INTO account(username,sha_pass_hash,expansion) VALUES('{account['name']}','{digest}',4)")
    account['id']=int(p.db.q('Login',f"SELECT id FROM account WHERE username='{account['name']}'")[0][0]);w=None;guid=None
    result=dict(scope='Isolated native cleanup integration, not quest gameplay',account_id=account['id'],name=name,status='UNVERIFIED')
    try:
        w=p.connect(account);w.char_enum();w.char_create(name,2,1)
        for _ in range(100):
            op,body=w.read_packet()
            if op==headless.SMSG_CHAR_CREATE:
                assert body==bytes([47]);break
        else:raise AssertionError('No creation response')
        guid=int(p.db.q('Character',f"SELECT guid FROM characters WHERE name='{name}' AND account={account['id']}")[0][0])
        w.s.close();w=None
        result['cleanup']=cleanup_owned(p.db,a.world_pid,account['name'],name,account['id'],guid)
        assert result['cleanup']['account_absent'] and result['cleanup']['character_absent']
        result['status']='INTEGRATION_VERIFIED'
    finally:
        if w:w.s.close()
        write_json(ROOT/'out/quest-qa/cleanup-live.json',result)
    print(json.dumps(result))


if __name__=='__main__':main()
