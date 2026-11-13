"""One-shot reviewed gameplay contracts on the existing single test-server pair."""
import argparse,datetime,json,subprocess,sys
from regular_runtime import ROOT,validate
sys.path.insert(0,str(ROOT/'contrib/qa-automation'))
from evidence import write_json
from run_lock import RunLock


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--select',default='',help='Optional existing contract/scenario ID; default is all reviewed cases')
    parser.add_argument('--force',action='store_true',help='Compatibility option; this runner always executes fresh cases')
    a=parser.parse_args()
    stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    folder=ROOT/'out/qa-reports'/stamp;folder.mkdir(parents=True,exist_ok=False)
    report=dict(started_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),status='RUNNING',
        profile='regular-realm-protocol-regression',selection=a.select or 'all reviewed starter cases',all_content_certified=False,
        server_pairs=1,game_client_required=False,codex_required=False)
    def save():
        write_json(folder/'run.json',report)
        write_json(folder.parent/'latest.json',dict(report=str(folder/'README.md'),status=report['status']))
        lines=['# QA results','', 'Status: '+report['status'],
                 'Profile: reviewed starter interactions on the regular LoA realm; temporary normal-security test accounts.',
            'Uses the same auth/world pair as Start-Server.cmd. No game client or observer required.','',
            'Scope: reviewed cases only; whole-game coverage remains unverified.']
        if report.get('result'):
            result=report['result']
            lines+=['','Gameplay: '+str(result.get('gameplay_summary',{}).get('counts',{})),
                    'Data audits: '+str(result.get('data_summary',{}).get('counts',{}))]
            issues=[]
            for category,key in (('gameplay','gameplay'),('data','data_audits')):
                for row in result.get(key,[]):
                    if row.get('status') in ('failed','blocked','untested'):
                        issues.append(dict(category=category,**row))
            write_json(folder/'repair-queue.json',dict(native_source=report.get('native_source'),
                result_file=str(folder/'details/qa-report.json'),issues=issues))
            if issues:
                lines+=['','## Findings','', '| Type | Check | Status | Evidence |','|---|---|---|---|']
                for row in issues:
                    evidence=row.get('error',row.get('reason',str(row.get('violation_count',''))+' reference violations'))
                    evidence=str(evidence).replace('|','/').replace('\n',' ')[:500]
                    lines.append('| '+row['category']+' | '+row['id']+' | '+row['status']+' | '+evidence+' |')
        if report.get('runtime_diagnostics'):
            d=report['runtime_diagnostics']
            lines+=['','Runtime warning/error signatures: '+str(d['unique_signatures'])+'. See runtime-diagnostics.json.']
        if report.get('error'):lines+=['','Problem: '+report['error']]
        lines+=['','Details: startup.log, tests.log, details/qa-report.md, and repair-queue.json. Per-case evidence is retained under details/.']
        (folder/'README.md').write_text('\n'.join(lines)+'\n',encoding='utf-8')
    save();print('QA report: '+str(folder/'README.md'),flush=True)
    try:
        with RunLock(ROOT/'out/qa-automation/runner.lock'):
            with (folder/'startup.log').open('w',encoding='utf-8') as stream:
                setup=subprocess.run(['powershell.exe','-NoProfile','-File',str(ROOT/'contrib/windows/Start-TestServer.ps1')],
                    cwd=ROOT,stdout=stream,stderr=subprocess.STDOUT,timeout=240)
            if setup.returncode:raise RuntimeError('The single test-server pair did not start; see startup.log')
            runtime=validate()
            report['runtime']=str(runtime['runtime'])
            manifest=json.loads((runtime['runtime']/'ACTIVE-RUNTIME-MANIFEST.json').read_text(encoding='utf-8-sig'))
            report['native_source']=manifest['native_source']
            from qa_spec import load_suite
            suite=load_suite(ROOT/'contrib/headless-client/qa-scenarios.json')
            # Reuse the authored real-interaction and malformed-request regressions,
            # rather than launching the retired separate QA realm to exercise them.
            for name in ('qa-workflow-scenarios.json','qa-invalid-quest-scenarios.json'):
                suite['scenarios'].extend(load_suite(ROOT/'contrib/headless-client'/name)['scenarios'])
            suite['name']='loa-single-realm-regression-v2'
            if a.select:
                suite['scenarios']=[c for c in suite['scenarios'] if c['id']==a.select]
                if not suite['scenarios']:raise RuntimeError('No existing starter scenario matches --select; use IDs from contrib/headless-client/qa-scenarios.json')
            write_json(folder/'suite.json',suite)
            server=json.loads((ROOT/'out/qa-automation/server/latest.json').read_text(encoding='utf-8-sig'))
            pid=next(p['Pid'] for p in server['Processes'] if p['Name']=='worldserver')
            command=[sys.executable,str(ROOT/'contrib/headless-client/qa_run.py'),'--config',str(runtime['runtime']/'worldserver.conf'),
                     '--suite',str(folder/'suite.json'),'--report-dir',str(folder/'details'),'--world-pid',str(pid)]
            print('Running reviewed quest cases. Reports are saved automatically; no additional server or client is opened.',flush=True)
            with (folder/'tests.log').open('w',encoding='utf-8') as stream:
                result=subprocess.run(command,cwd=ROOT,stdout=stream,stderr=subprocess.STDOUT)
            details=folder/'details/qa-report.json'
            summary=json.loads(details.read_text(encoding='utf-8')) if details.exists() else None
            report['result']=summary;report['exit_code']=result.returncode
            if summary is None:raise RuntimeError('QA could not produce a complete batch result; inspect tests.log')
            if not summary.get('gameplay'):raise RuntimeError('No gameplay cases were executed')
            report['status']='COMPLETED' if result.returncode==0 else 'NEEDS_REVIEW'
            from runtime_diagnostics import collect
            diagnostics=collect(runtime['runtime']);write_json(folder/'runtime-diagnostics.json',diagnostics)
            report['runtime_diagnostics']=dict(unique_signatures=diagnostics['unique_signatures'],partial_files=diagnostics['partial_files'],files_read=diagnostics['files_read'])
            if diagnostics['unique_signatures'] or diagnostics['partial_files'] or not diagnostics['files_read']:report['status']='NEEDS_REVIEW'
    except KeyboardInterrupt:report.update(status='INTERRUPTED',error='Run interrupted; inspect retained per-case evidence before retrying')
    except Exception as error:report.update(status='BLOCKED',error=str(error))
    finally:
        report['finished_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat();save()
    print('QA '+report['status']+'. Report: '+str(folder/'README.md'),flush=True)
    return int(report['status']!='COMPLETED')


if __name__=='__main__':raise SystemExit(main())
