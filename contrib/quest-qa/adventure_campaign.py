"""Unattended episodes using the existing persistent native adventure worker.

Invoked by qa-automation/pipeline.py --profile adventure. Reports are private,
append-only per run, and never equate a successful process with content coverage.
"""
import datetime
import hashlib
import json
import os
import re
import subprocess
import sys
import time
from pathlib import Path
from pool import Pool, ROOT
from diagnostics import snapshot, compare
from evidence import write_json


def process_alive(pid):
    if os.name!='nt':
        try:os.kill(pid,0);return True
        except ProcessLookupError:return False
    import ctypes
    from ctypes import wintypes
    kernel=ctypes.WinDLL('kernel32',use_last_error=True)
    kernel.OpenProcess.restype=wintypes.HANDLE
    kernel.OpenProcess.argtypes=[wintypes.DWORD,wintypes.BOOL,wintypes.DWORD]
    kernel.GetExitCodeProcess.argtypes=[wintypes.HANDLE,ctypes.POINTER(wintypes.DWORD)]
    kernel.CloseHandle.argtypes=[wintypes.HANDLE]
    handle=kernel.OpenProcess(0x1000,False,pid)
    if not handle:
        if ctypes.get_last_error()==87:return False
        raise ctypes.WinError(ctypes.get_last_error())
    try:
        code=wintypes.DWORD()
        if not kernel.GetExitCodeProcess(handle,ctypes.byref(code)):raise ctypes.WinError(ctypes.get_last_error())
        return code.value==259
    finally:kernel.CloseHandle(handle)


def release_dead_worker_lease(pool,episode,pid):
    control=episode/'worker-control.json'
    if not control.exists() or process_alive(pid):return
    record=json.loads(control.read_text(encoding='utf-8'))
    if record.get('worker_pid')!=pid:return
    # Match the exact worker token; never erase another run's replacement lease.
    pool.registry.execute('DELETE FROM leases WHERE account_id=? AND owner=?',
                          (record['account_id'],record['lease_owner']))
    pool.registry.commit()


def validate_runtime(pool):
    for name,key,port in [('authserver','RealmServerPort',3725),('worldserver','WorldServerPort',8086)]:
        text=(pool.runtime/(name+'.conf')).read_text(encoding='utf-8-sig')
        if not re.search(r'^\s*BindIP\s*=\s*"127\.0\.0\.1"\s*$',text,re.M):
            raise RuntimeError('QA bind must be loopback in '+name+'.conf')
        if not re.search(r'^\s*'+key+r'\s*=\s*'+str(port)+r'\s*$',text,re.M):
            raise RuntimeError('Unexpected QA port in '+name+'.conf')
        if name=='authserver':
            connection=re.search(r'^\s*LoginDatabaseInfo\s*=\s*"([^"]+)"',text,re.M)
            if not connection or connection[1].split(';')!=pool.db.connections['Login']:
                raise RuntimeError('QA auth configuration must use the same owned Login database as QA world')
    expected=pool.environment.get('worldserver_sha256')
    if not expected or hashlib.sha256((pool.runtime/'worldserver.exe').read_bytes()).hexdigest()!=expected:
        raise RuntimeError('QA binary differs from recorded deployment hash')


def world_pid(runtime):
    # Discover the exact owned runtime, not whichever worldserver happens to be
    # first. A production process can be running on this machine simultaneously.
    command = "$ErrorActionPreference='Stop'; $env:PSModulePath += ';'+(Join-Path $PSHOME 'Modules'); Get-CimInstance Win32_Process -Filter \"name='worldserver.exe'\" | Select-Object ProcessId,ExecutablePath | ConvertTo-Json -Compress"
    result = subprocess.run(['powershell.exe', '-NoProfile', '-Command', command],
                            capture_output=True, text=True, check=True, timeout=30)
    rows = json.loads(result.stdout or '[]')
    if isinstance(rows, dict): rows = [rows]
    expected = (runtime/'worldserver.exe').resolve()
    matches = [int(r['ProcessId']) for r in rows if r.get('ExecutablePath') and Path(r['ExecutablePath']).resolve() == expected]
    if len(matches) != 1:
        raise RuntimeError('Expected exactly one existing isolated QA worldserver; start the prepared QA runtime first')
    return matches[0]


def summarize_result(result, signals,server_alive=True):
    if not server_alive:
        return 'BLOCKED','QA worldserver exited during the episode; evidence retained, no gameplay failure inferred'
    if result.get('error') or not result.get('reconnect_verified'):
        return 'BLOCKED', result.get('error', 'Normal reconnect verification missing')
    if signals:
        return 'NEEDS_REVIEW', 'Suspected stalled progress; diagnose evidence before classifying a game defect'
    return 'EXECUTED', 'Bounded native gameplay and reconnect executed; no full-content certification implied'


def run(a):
    pool = Pool()
    stamp = datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    folder = ROOT/'out/qa-automation/adventure'/stamp
    folder.mkdir(parents=True, exist_ok=False)
    report = dict(version=1, run_id=stamp, profile='natural-native-playerbots',
                  status='RUNNING', started_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                  guid=a.guid, episode_seconds=a.episode_seconds, planned_episodes=a.episodes,
                  stop_level=a.stop_level, episodes=[], findings=[], all_content_certified=False,
                  administrative_grants=False, production_modified=False)
    report['tool_sources']={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest()
        for p in [Path(__file__),ROOT/'contrib/quest-qa/adventure.py',ROOT/'contrib/quest-qa/adventure_progress.py',ROOT/'contrib/quest-qa/adventure_notes.py']}
    for relative in report['tool_sources']:
        archived=folder/'framework'/Path(relative).name
        archived.parent.mkdir(exist_ok=True)
        archived.write_bytes((ROOT/relative).read_bytes())
    before = snapshot(pool.runtime/'logs')

    def save():
        write_json(folder/'run.json', report)
        note_run=report.get('active_episode') or (report['episodes'][-1]['run'] if report['episodes'] else None)
        note_path=ROOT/'out/quest-qa/adventure'/note_run/'notes.md' if note_run else None
        write_json(folder.parent/'latest.json',dict(run=stamp, report=str(folder/'README.md'), status=report['status'],notes=str(note_path) if note_path else None))
        lines = ['# Unattended native gameplay QA', '', 'Status: '+report['status'],
                 f"Character GUID: {a.guid}. Natural movement/XP; no grants or resets.",
                 'Whole-game coverage: **not certified**.', '',
                 '| Episode | Status | Level | Newly rewarded quests | Evidence |',
                 '| --- | --- | --- | --- | --- |']
        for e in report['episodes']:
            evidence=Path(e['evidence'])
            lines.append(f"| {e['run']} | {e['status']} | {e.get('level','?')} | {e.get('rewarded',[])} | [result]({evidence.as_posix()}) |")
        lines += ['', '## Findings for a later repair pass', '']
        if report.get('active_episode'):
            lines += ['Active episode: '+report['active_episode'], '']
        if note_path:
            lines += ['[Live human-readable notes]('+Path(os.path.relpath(note_path,folder)).as_posix()+')', '']
        for f in report['findings']:
            lines.append('- '+f['reason'])
        if not report['findings']:lines.append('No failure reported yet; this does not prove untested content works.')
        lines += ['', 'Evidence includes immutable episode JSONL, saved states, actual native activity,',
                  'equipment/quest transitions, reconnect results and diagnostic differences.',
                  'To stop safely, create a file named `STOP` in this report folder.',
                  'Rerun the same command to resume the persistent character in a new report.']
        (folder/'README.md').write_text('\n'.join(lines)+'\n', encoding='utf-8')
        write_json(folder/'repair-queue.json',dict(run=stamp, findings=report['findings'],
                    note='Triage evidence only. Do not auto-complete quests, reset characters or auto-apply speculative fixes.'))
    save()
    print('QA report: '+str(folder/'README.md'), flush=True)
    child = None
    try:
        validate_runtime(pool)
        c = next((c for c in pool.state['characters'] if c['guid']==a.guid), None)
        if not c or c.get('purpose')!='persistent-adventure':
            raise RuntimeError('Select an existing designated persistent-adventure character; no automatic provisioning/reset')
        for n in range(1, a.episodes+1):
            if (folder/'STOP').exists():report['status']='STOPPED';break
            pid = world_pid(pool.runtime)
            if a.world_pid and a.world_pid != pid:raise RuntimeError('--world-pid does not match the owned isolated QA executable')
            run_id = 'unattended-'+stamp.replace('.','')+'-'+str(n)
            episode = ROOT/'out/quest-qa/adventure'/run_id
            report['active_episode']=run_id;save()
            with (folder/f'episode-{n}.log').open('w', encoding='utf-8') as log:
                child = subprocess.Popen([sys.executable, str(ROOT/'contrib/quest-qa/adventure.py'),
                    '--guid', str(a.guid), '--world-pid', str(pid), '--seconds', str(a.episode_seconds), '--run', run_id],
                    cwd=ROOT, stdout=log, stderr=subprocess.STDOUT)
                deadline = time.monotonic()+a.episode_seconds+180
                stopping = False
                while child.poll() is None:
                    if not process_alive(pid):
                        raise RuntimeError('QA worldserver exited during the episode; gameplay stopped and evidence retained')
                    if (folder/'STOP').exists() or time.monotonic()>deadline:
                        if episode.exists():(episode/'STOP').touch()
                        if not stopping:
                            stopping = True;deadline = time.monotonic()+90
                        elif time.monotonic()>deadline:
                            # Only the timed-out Python worker; never terminate the server.
                            child.kill();child.wait(timeout=10)
                            raise RuntimeError('Episode failed to exit after STOP; worker terminated, inspect lease/online state before retry')
                    time.sleep(1)
                code=child.returncode
                release_dead_worker_lease(pool,episode,child.pid)
                child=None
            result_path=episode/'result.json'
            if not result_path.exists():raise RuntimeError(f'Episode{n} produced no result; inspect episode-{n}.log')
            result=json.loads(result_path.read_text(encoding='utf-8'))
            signals=[]
            events_path=episode/'events.jsonl'
            if events_path.exists():
                signals=[e for e in (json.loads(line) for line in events_path.read_text(encoding='utf-8').splitlines()) if e.get('event')=='suspected_stall']
            server_running=process_alive(pid)
            status,reason=summarize_result(result,signals,server_alive=server_running)
            if code and status=='EXECUTED':status,reason='BLOCKED',f'Worker exit code{code}'
            level=int(result.get('final_state',{}).get('character',[[0]])[0][0])
            row=dict(run=run_id,status=status,reason=reason,level=level,rewarded=result.get('rewarded_delta',[]),
                     evidence=os.path.relpath(result_path,folder), binary_sha256=result.get('binary_sha256'),
                     native_source=result.get('native_source'),reconnect_verified=result.get('reconnect_verified',False))
            report['episodes'].append(row)
            report.pop('active_episode',None)
            if status!='EXECUTED':
                report['findings'].append(dict(classification='deployment_or_environment' if not server_running else 'unknown_or_insufficient_evidence',reason=reason,
                     episode=run_id,evidence=row['evidence'],signals=signals))
                report['status']=status;save();break
            save()
            if (folder/'STOP').exists():report['status']='STOPPED';break
            if level>=a.stop_level:report['status']='TARGET_LEVEL_OBSERVED';break
        else:report['status']='COMPLETED_BOUNDED_RUN'
    except KeyboardInterrupt:
        report['status']='INTERRUPTED'
    except Exception as error:
        report['status']='BLOCKED';report['findings'].append(dict(classification='deployment_or_automation',reason=str(error)))
    finally:
        if child:
            if child.poll() is None:
                if episode.exists():(episode/'STOP').touch()
                try:child.wait(timeout=45)
                except subprocess.TimeoutExpired:child.kill();child.wait(timeout=10)
            release_dead_worker_lease(pool,episode,child.pid)
        changed=[p for p,h in report['tool_sources'].items() if hashlib.sha256((ROOT/p).read_bytes()).hexdigest()!=h]
        if changed:
            report['status']='NEEDS_REVIEW'
            report['findings'].append(dict(classification='automation_source_change',reason='Tools changed during run; archived originals retained',paths=changed))
        report['diagnostics']=compare(before,snapshot(pool.runtime/'logs'))
        if report['diagnostics']['requires_review']:
            report['findings'].append(dict(classification='runtime_diagnostic_review',reason='New or recurring warning/error signatures require review; see run.json diagnostics'))
            if report['status'] in ('TARGET_LEVEL_OBSERVED','COMPLETED_BOUNDED_RUN'):report['status']='NEEDS_REVIEW'
        report['finished_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat();save()
    print(json.dumps(dict(report=str(folder/'README.md'),status=report['status'])))
    return int(report['status'] in ('BLOCKED','NEEDS_REVIEW','INTERRUPTED'))
