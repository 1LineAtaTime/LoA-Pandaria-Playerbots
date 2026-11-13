"""Owned-account SRP regression: short valid fields, fresh generation and corrupt-data rejection.
No existing account credentials are read or changed; report contains no credential material.
"""
import sys,secrets,hashlib,json,subprocess
from pathlib import Path
R=Path(__file__).resolve().parents[2];sys.path.insert(0,str(R/'contrib/headless-client'))
import headless
from starter_campaign import Database
from qa_cleanup import cleanup_owned
headless.log=lambda *a,**k:None
d=Database(R/'exports/worldserver.conf',R/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
pid=int(subprocess.check_output(['powershell','-NoProfile','-Command',"Get-CimInstance Win32_Process | Where-Object Name -eq 'worldserver.exe' | Select-Object -ExpandProperty ProcessId"],text=True).strip())
name='Qsrp'+''.join(secrets.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(5));account=name.upper();password=secrets.token_hex(10).upper();aid=None;report={'passed':False,'owned_account_only':True}
try:
 digest=hashlib.sha1((account+':'+password).encode()).digest()
 # Both valid numeric fields deliberately lose high zero bytes in BigNumber notation.
 for _ in range(5000):
  salt=secrets.token_bytes(31)+b'\0';x=int.from_bytes(hashlib.sha1(salt+digest).digest(),'little');v=pow(headless.G_INT,x,headless.N_INT);vh=format(v,'X')
  if len(vh)<64:break
 else:raise RuntimeError('Unable to construct short verifier fixture')
 sh=format(int.from_bytes(salt,'little'),'X')
 d.q('Login',"INSERT INTO account(username,sha_pass_hash,s,v,expansion) VALUES('"+account+"','"+digest.hex()+"','"+sh+"','"+vh+"',4)")
 aid=int(d.q('Login',"SELECT id FROM account WHERE username='"+account+"'")[0][0])
 assert len(headless.auth_login('127.0.0.1',3724,account,password))==40;report['short_numeric_srp_authenticated']=True
 d.q('Login',f"UPDATE account SET s='',v='' WHERE id={aid}")
 assert len(headless.auth_login('127.0.0.1',3724,account,password))==40
 assert d.q('Login',f'SELECT LENGTH(s),LENGTH(v) FROM account WHERE id={aid}')==[['64','64']];report['generated_fields_fixed_width']=True
 d.q('Login',f"UPDATE account SET v=REPEAT('Z',64) WHERE id={aid}")
 assert len(headless.auth_login('127.0.0.1',3724,account,password))==40;report['malformed_srp_rederived_from_valid_legacy_hash']=True
 d.q('Login',f"UPDATE account SET v='',s='',sha_pass_hash='' WHERE id={aid}")
 try:headless.auth_login('127.0.0.1',3724,account,password)
 except RuntimeError as e:
  assert 'AUTH_LOGON_CHALLENGE rejected' in str(e);report['unrecoverable_credentials_rejected']=True
 else:raise AssertionError('Invalid credentials unexpectedly accepted')
 d.q('Login',f"UPDATE account SET sha_pass_hash='{digest.hex()}' WHERE id={aid}")
 assert len(headless.auth_login('127.0.0.1',3724,account,password))==40;report['authserver_survived_rejection']=True;report['passed']=True
 print('SRP credential regression passed',flush=True)
finally:
 if aid:report['cleanup']=cleanup_owned(d,pid,account,name,aid)
 (R/'out/auth-srp-regression.json').write_text(json.dumps(report,indent=2))
