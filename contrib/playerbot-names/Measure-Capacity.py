"""Observe the existing server at one bot-count stage; never starts another realm.
Records actual online counts, process CPU/RAM, DB connections and warning/error deltas.
"""
import argparse,ctypes as C,os,json,re,sys,time,subprocess
from ctypes import wintypes as W
from pathlib import Path
R=Path(__file__).resolve().parents[2];sys.path.insert(0,str(R/'contrib/headless-client'));from starter_campaign import Database
p=argparse.ArgumentParser();p.add_argument('--target',type=int,required=True);p.add_argument('--hold',type=int,default=60);p.add_argument('--timeout',type=int,default=600);a=p.parse_args();d=Database(R/'exports/worldserver.conf',R/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe');O=R/'out/playerbot-naming-20261112';pid=int(subprocess.check_output(['powershell','-NoProfile','-Command',"Get-CimInstance Win32_Process | Where-Object Name -eq 'worldserver.exe' | Select-Object -ExpandProperty ProcessId"],text=True).strip())
k=C.WinDLL('kernel32',use_last_error=True);ps=C.WinDLL('psapi',use_last_error=True);k.OpenProcess.argtypes=[W.DWORD,W.BOOL,W.DWORD];k.OpenProcess.restype=C.c_void_p;k.GetProcessTimes.argtypes=[C.c_void_p,C.c_void_p,C.c_void_p,C.c_void_p,C.c_void_p]
class Mem(C.Structure):_fields_=[('cb',W.DWORD),('faults',W.DWORD)]+[(n,C.c_size_t) for n in ['peak_working','working','peak_paged','paged','peak_nonpaged','nonpaged','pagefile','peak_pagefile','private']]
ps.GetProcessMemoryInfo.argtypes=[C.c_void_p,C.c_void_p,W.DWORD];h=k.OpenProcess(0x410,False,pid);assert h
log=R/'exports/logs/Server.log';offset=log.stat().st_size;samples=[];start=time.monotonic();stable_since=None;previous=None
try:
 while time.monotonic()-start<a.timeout:
  times=[C.c_ulonglong() for _ in range(4)];assert k.GetProcessTimes(h,*[C.byref(x) for x in times]),'Process exited';cpu=(times[2].value+times[3].value)/1e7;now=time.monotonic();m=Mem();m.cb=C.sizeof(m);assert ps.GetProcessMemoryInfo(h,C.byref(m),m.cb)
  n=int(d.q('Character','SELECT COUNT(*) FROM characters c JOIN playerbot_roster_accounts r ON r.account_id=c.account JOIN mop_playerbots.playerbots_account_type t ON t.account_id=c.account WHERE c.online<>0 AND t.account_type=1')[0][0]);db=d.q('World',"SHOW GLOBAL STATUS WHERE Variable_name IN('Threads_connected','Threads_running','Slow_queries')")
  sample=dict(seconds=round(now-start,1),online_random_bots=n,working_mb=round(m.working/1048576,1),private_mb=round(m.private/1048576,1),db=dict(db))
  if previous:sample['cpu_core_percent']=round(100*(cpu-previous[1])/(now-previous[0]),1);sample['cpu_host_percent']=round(sample['cpu_core_percent']/(os.cpu_count() or 1),1)
  previous=(now,cpu);samples.append(sample)
  if n==a.target:
   if stable_since is None:stable_since=now
   if now-stable_since>=a.hold:break
  else:stable_since=None
  if len(samples)%6==0:print('Capacity stage',a.target,':',n,'online;',sample.get('cpu_host_percent'),'percent host CPU;',sample['working_mb'],'MB',flush=True)
  time.sleep(5)
finally:
 with log.open('rb') as f:f.seek(offset);text=f.read().decode('utf-8',errors='replace')
 report=dict(pid=pid,target=a.target,hold_seconds=a.hold,sustained=stable_since is not None and time.monotonic()-stable_since>=a.hold,samples=samples,warning_lines=len(re.findall(r'\bWARN\b',text)),error_lines=len(re.findall(r'\bERROR\b',text)),update_percentiles=re.findall(r'Percentiles \(95, 99, max\): ([^\r\n]+)',text)[-10:])
 (O/f'capacity-{a.target}.json').write_text(json.dumps(report,indent=2));print('Stage',a.target,'sustained:',report['sustained'],flush=True)
assert report['sustained'],'Target count was not sustained in observation window'
