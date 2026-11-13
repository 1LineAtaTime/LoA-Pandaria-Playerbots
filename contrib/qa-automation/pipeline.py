"""One repeatable regression run with private checkpoints and explicit content debt."""
import argparse
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT/'contrib/headless-client'), str(ROOT/'contrib/content-audit')]
from starter_campaign import Database
from qa_spec import load_suite
from qa_run import AUDITS
from compare_quest_audits import load as load_dependencies, compare as compare_dependencies
from checkpoint import runtime_checkpoint, database_backups, private_path, sha, validate_plan
from diagnostics import snapshot, compare as compare_logs
from prioritize import generate as generate_priority
from evidence import write_json
from run_lock import RunLock
from recover_accounts import recover

HERE = Path(__file__).parent
SUITES = {
    'starter': 'qa-scenarios.json', 'invalid_requests': 'qa-invalid-quest-scenarios.json',
    'gossip': 'qa-legendary-scenarios.json', 'workflows': 'qa-workflow-scenarios.json',
}


def native_changes(root, revision):
    paths = subprocess.check_output(['git','diff','--name-only',revision,'--','src','modules','dep','CMakeLists.txt','cmake'],cwd=root,text=True).splitlines()
    paths += subprocess.check_output(['git','ls-files','--others','--exclude-standard','--','src','modules','dep','cmake'],cwd=root,text=True).splitlines()
    return [p for p in paths if Path(p).suffix.lower() in ('.cpp','.h','.hpp','.c','.cmake','.txt','.in')]


def data_comparison(actual, baseline):
    expected = {r['id']: {tuple(row) for row in r['rows']} for r in baseline['audits']}
    new=[dict(audit=ident,issue='lost_audit_coverage') for ident in sorted(set(expected)-set(actual))]; absent=[]
    for ident, rows in actual.items():
        values={tuple(r) for r in rows}
        new += [dict(audit=ident,row=list(r)) for r in sorted(values-expected.get(ident,set()))]
        absent += [dict(audit=ident,row=list(r)) for r in sorted(expected.get(ident,set())-values)]
    return dict(new_findings=new,absent_previous_findings=absent,regression_free=not new,
                content_ready=not any(actual.values()), note='Known findings remain debt; an absent row needs repair review.')


def choose_suites(profile, changed):
    if profile=='full':return list(SUITES)
    picked={'workflows'}
    if any('battle' in p.lower() or 'tarecgosa' in p.lower() for p in changed):picked.add('gossip')
    if any('QuestHandler' in p or 'qa_actions' in p or 'qa_spec' in p or 'starter_campaign' in p for p in changed):
        picked.update(('starter','invalid_requests'))
    return [s for s in SUITES if s in picked]


def validate_gameplay_contract(suite,results,required):
    declared={x['id'] for x in suite['scenarios']}
    if not set(required)<=declared:raise ValueError('Previously required gameplay coverage was removed')
    ids=[x['id'] for x in results]
    if len(ids)!=len(set(ids)) or declared!=set(ids):raise ValueError('Incomplete or duplicate scenario results')
    passed={x['id'] for x in results if x['status']=='passed'}
    if not set(required)<=passed:raise ValueError('Previously required gameplay coverage no longer passes')


def source_fingerprints():
    files=list(HERE.glob('*.py'))+list(HERE.glob('*.json'))
    files+=list((ROOT/'contrib/headless-client').glob('qa_*.py'))+list((ROOT/'contrib/headless-client').glob('qa-*.json'))
    files+=list((ROOT/'contrib/content-audit').glob('*.py'))
    files += [ROOT/'contrib/headless-client/starter_campaign.py',ROOT/'contrib/headless-client/headless.py']
    return {str(f.relative_to(ROOT)).replace('\\','/'):sha(f) for f in files}


def build_tools(directory):
    cache=Path(directory)/'CMakeCache.txt'
    match=re.search(r'^CMAKE_COMMAND:[^=]+=(.+)$',cache.read_text(),re.M) if cache.exists() else None
    cmake=Path(match[1].strip()) if match else Path(shutil.which('cmake') or 'missing-cmake')
    ctest=cmake.with_name('ctest.exe' if os.name=='nt' else 'ctest')
    if not cmake.is_file() or not ctest.is_file():raise RuntimeError('Configured CMake/CTest tools are unavailable')
    return str(cmake),str(ctest)


def run_pipeline():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--profile',choices=('targeted','full','adventure'),default='targeted')
    p.add_argument('--guid',type=int,default=120,help='Existing persistent QA character for adventure profile')
    p.add_argument('--episodes',type=int,default=12,help='Bounded adventure episodes (1..100)')
    p.add_argument('--episode-seconds',type=int,default=600,help='Native gameplay per episode (30..3600 seconds)')
    p.add_argument('--stop-level',type=int,default=15,help='Stop when this legitimately earned level is observed')
    p.add_argument('--experimental-autoplay',action='store_true',help='Developer opt-in only; autonomous navigation has known failures')
    p.add_argument('--run-dir',type=Path)
    p.add_argument('--config',type=Path,default=ROOT/'exports/worldserver.conf')
    p.add_argument('--client',type=Path,default=ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
    p.add_argument('--world-pid',type=int)
    p.add_argument('--audit-only',action='store_true')
    p.add_argument('--backup-plan',type=Path)
    p.add_argument('--build',action='store_true',help='Force one Visual Studio/MSBuild build before checks')
    p.add_argument('--build-dir',type=Path,default=ROOT/'out/build/vs2022-x64')
    a=p.parse_args()
    if a.profile=='adventure':
        if not a.experimental_autoplay:
            p.error('Autonomous questing is paused after circling/terrain failures. Use Run-QA.cmd for reviewed protocol tests.')
        if not 1<=a.episodes<=100 or not 30<=a.episode_seconds<=3600 or not 1<=a.stop_level<=90:
            p.error('Adventure bounds: episodes1..100, seconds30..3600, stop-level1..90')
        if a.build or a.backup_plan or a.audit_only or a.run_dir or a.config!=ROOT/'exports/worldserver.conf':
            p.error('Adventure uses only the existing owned QA runtime; build/backup/audit/config/run-dir overrides are unsupported')
        sys.path.insert(0,str(ROOT/'contrib/quest-qa'))
        from adventure_campaign import run
        return run(a)
    if not a.audit_only and not a.world_pid:p.error('--world-pid is required for native test cleanup')
    if a.backup_plan:validate_plan(json.loads(a.backup_plan.read_text()))
    active=json.loads((ROOT/'exports/ACTIVE-RUNTIME-MANIFEST.json').read_text(encoding='utf-8-sig'))
    changed=subprocess.check_output(['git','diff','--name-only',active['native_source']],cwd=ROOT,text=True).splitlines()
    untracked=subprocess.check_output(['git','ls-files','--others','--exclude-standard','--','contrib'],cwd=ROOT,text=True).splitlines()
    selected=choose_suites(a.profile,changed+untracked)
    required=json.loads((HERE/'required-gameplay.json').read_text())
    for suite in selected:
        spec=load_suite(ROOT/'contrib/headless-client'/SUITES[suite])
        if not set(required[suite])<={x['id'] for x in spec['scenarios']}:raise ValueError('Required scenario removed: '+suite)
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    run=private_path(ROOT,a.run_dir or ROOT/'out/qa-automation/runs'/stamp)
    if run.exists():raise ValueError('Use a fresh run directory')
    run.mkdir(parents=True)
    report=dict(version=1,profile=a.profile,started_utc=datetime.now(timezone.utc).isoformat(),
                status='running',regression_free=False,content_ready=False,all_content_validated=False,
                changed_paths=changed+untracked,selected_suites=selected,steps=[],gameplay={},notes=[])
    def save():
        write_json(run/'run.json',report)
    def command(name,args,console=False):
        step=dict(name=name,status='running');report['steps'].append(step);save()
        started=datetime.now(timezone.utc)
        if console:
            result=subprocess.run(args,cwd=ROOT)
        else:
            with (run/(name+'.log')).open('w',encoding='utf-8') as stream:
                result=subprocess.run(args,cwd=ROOT,stdout=stream,stderr=subprocess.STDOUT,text=True,encoding='utf-8')
        step.update(exit_code=result.returncode,elapsed_seconds=(datetime.now(timezone.utc)-started).total_seconds(),
                    status='passed' if result.returncode==0 else 'needs_review')
        if not console:
            log=run/(name+'.log');text=log.read_text(encoding='utf-8',errors='replace')
            step['log_sha256']=sha(log)
            unit=re.search(r'Ran (\d+) tests?',text)
            native_test=re.search(r'0 tests failed out of (\d+)',text)
            if unit:step['tests_passed']=int(unit[1]) if result.returncode==0 else None
            if native_test:step['tests_passed']=int(native_test[1])
        save()
        return result.returncode
    before=snapshot(ROOT/'exports/logs');report['tool_sources']=source_fingerprints();save()
    try:
        checkpoint=runtime_checkpoint(ROOT,run/'checkpoint')
        db=Database(a.config,a.client)
        if a.backup_plan:
            checkpoint['database_backups']=database_backups(db,json.loads(a.backup_plan.read_text()),run/'checkpoint')
            (run/'checkpoint/checkpoint.json').write_text(json.dumps(checkpoint,indent=2)+'\n',encoding='utf-8')
        report['checkpoint']=dict(path='checkpoint/checkpoint.json',sha256=sha(run/'checkpoint/checkpoint.json'))
        native=native_changes(ROOT,active['native_source']);report['native_changes']=native
        cmake,ctest=build_tools(a.build_dir) if (native or a.build or a.profile=='full') else (None,None)
        sql_changes=[x for x in changed if x.startswith('sql/') and x.endswith('.sql')]
        if sql_changes and not a.backup_plan:
            raise RuntimeError('Changed migrations require a reviewed --backup-plan before continuing')
        if native or a.build:
            if command('build',[cmake,'--build',str(a.build_dir),'--config','RelWithDebInfo','--parallel']):
                raise RuntimeError('Native build failed')
            if not a.audit_only and sha(ROOT/'exports/worldserver.exe') != sha(a.build_dir/'bin/RelWithDebInfo/worldserver.exe'):
                raise RuntimeError('Build complete; stage/restart the matching worldserver before live QA. Checkpoint retained.')
        # Tests are registered explicitly; unrelated historical SQL fixture programs are not imported by discovery.
        tests=[
            'contrib/content-audit/test_catalogue.py','contrib/content-audit/test_quest_dependencies.py',
            'contrib/content-audit/test_compare_quest_audits.py','contrib/content-audit/test_addon_auras.py',
            'contrib/headless-client/test_qa.py','contrib/headless-client/test_qa_actions.py',
            'contrib/qa-automation/test_automation.py',
        ]
        for test in tests:
            if command(Path(test).stem,[sys.executable,test]):raise RuntimeError('Unit checks failed: '+test)
        if a.profile=='full' or native or a.build:
            if command('native_tests',[ctest,'--test-dir',str(a.build_dir),'-C','RelWithDebInfo','--output-on-failure','--output-junit',str(run/'native-tests.xml')]):
                raise RuntimeError('Native regression tests failed')
        if command('dependencies',[sys.executable,'contrib/content-audit/quest_dependencies.py','--output',str(run/'dependencies'),'--artifacts',str(run/'graph'),'--config',str(a.config),'--client',str(a.client)]):
            raise RuntimeError('Dependency generation failed')
        dep=compare_dependencies(load_dependencies(ROOT/'doc/quest-dependency-audit-548'),load_dependencies(run/'dependencies'))
        (run/'dependency-comparison.json').write_text(json.dumps(dep,indent=2)+'\n')
        report['dependencies']=dep
        if not dep['regression_free']:raise RuntimeError('New quest dependency failures')
        actual={ident:db.q('World',query) for ident,query in AUDITS}
        baseline=json.loads((HERE/'known-data-findings.json').read_text())
        report['data']=data_comparison(actual,baseline)
        (run/'data-audits.json').write_text(json.dumps(actual,indent=2)+'\n')
        if not report['data']['regression_free']:raise RuntimeError('New structural data failures')
        aura_exit=command('addon_auras',[sys.executable,'contrib/content-audit/addon_auras.py','--config',str(a.config),'--client',str(a.client),'--report',str(run/'addon-auras.json')])
        if aura_exit not in (0,1) or not (run/'addon-auras.json').exists():raise RuntimeError('Aura audit did not complete')
        aura=json.loads((run/'addon-auras.json').read_text())
        new_aura=[x for x in aura['findings'] if x not in baseline['addon_aura_findings']]
        report['addon_auras']=dict(findings=aura['findings'],new_findings=new_aura)
        if new_aura:raise RuntimeError('New addon aura references')
        if aura_exit:report['steps'][-1]['status']='completed_with_known_content_debt'
        classifications=json.loads((HERE/'availability.json').read_text())
        ranked=generate_priority(run/'dependencies',run/'prioritized-repairs.json',classifications['quests'])
        report['priority_queue']=dict(groups=len(ranked),top_targets=[dict(target=r['target_id'],reason=r['priority_reason']) for r in ranked[:10]])
        if not a.audit_only:
            if sha(ROOT/'exports/worldserver.exe') != next(x['sha256'] for x in active['artifacts'] if x['name']=='worldserver.exe'):
                raise RuntimeError('Live runtime manifest is stale; verify deployment before gameplay')
            for ident in selected:
                folder=run/ident
                code=command('live_'+ident,[sys.executable,'contrib/headless-client/qa_run.py','--suite',str(ROOT/'contrib/headless-client'/SUITES[ident]),'--report-dir',str(folder),'--world-pid',str(a.world_pid),'--config',str(a.config),'--client',str(a.client)],console=True)
                if not (folder/'qa-report.json').exists():
                    def cleanup_command(text):
                        subprocess.run([sys.executable,str(ROOT/'contrib/windows/Send-ServerConsoleCommand.py'),str(a.world_pid),text],check=True)
                    recovered=recover(db,folder,cleanup_command)
                    report.setdefault('recovered_accounts',[]).extend(recovered)
                    raise RuntimeError('Live suite produced no report; verified journals recovered: '+ident)
                result=json.loads((folder/'qa-report.json').read_text())
                validate_gameplay_contract(load_suite(ROOT/'contrib/headless-client'/SUITES[ident]),result['gameplay'],required[ident])
                counts=result['gameplay_summary']['counts']
                report['gameplay'][ident]=dict(counts=counts,cleanup_verified=all(c['account_absent'] and c['character_absent'] for c in result['cleanup']),report=str((folder/'qa-report.json').relative_to(run)),sha256=sha(folder/'qa-report.json'))
                # Exit1 for known data debt is retained, but never accepted as an unexplained test failure.
                if code not in (0,1) or counts['failed'] or counts['blocked'] or not report['gameplay'][ident]['cleanup_verified']:
                    raise RuntimeError('Live scenario or cleanup failed: '+ident)
                if code:report['steps'][-1]['status']='completed_with_known_content_debt'
                if ident=='workflows':
                    from qa_actions import OPS
                    exercised={action['op'] for case in result['gameplay'] if case['status']=='passed' for action in case.get('actions',[]) if action['status']=='action_finished'}
                    report['action_coverage']=dict(exercised_in_passing_contracts=sorted(exercised),implemented_without_live_contract=sorted(set(OPS)-exercised),all_mechanics_verified=False)
        report['status']='completed'
        report['regression_free']=True
        report['notes'].append('Known content debt and untested mechanics remain; successful automation does not certify the whole game.')
    except KeyboardInterrupt:
        report.update(status='interrupted',error='Interrupted; verify child exit before journal recovery.',regression_free=False)
    except Exception as error:
        report.update(status='failed',error=str(error),regression_free=False)
    finally:
        current_sources=source_fingerprints()
        if current_sources!=report['tool_sources']:
            report.update(status='tool_sources_changed',regression_free=False)
            report['notes'].append('Tools changed during this run; do not accept it as a reproducible pass.')
        after=snapshot(ROOT/'exports/logs');logs=compare_logs(before,after)
        report['diagnostics']=logs
        (run/'diagnostic-snapshots.json').write_text(json.dumps(dict(before=before,after=after),indent=2)+'\n')
        if logs['requires_review']:
            report['regression_free']=False
            if report['status']=='completed':report['status']='needs_diagnostic_review'
        report['finished_utc']=datetime.now(timezone.utc).isoformat();save()
        lines=['# Automated regression run','',f"Status: {report['status']}",f"Regression free: {report['regression_free']}",'Content ready: False','',
               '## Gameplay','']
        for name,r in report['gameplay'].items():lines.append(f"- {name}: {r['counts']}; cleanup={r['cleanup_verified']}")
        lines+=['','## Diagnostic review','',f"New signatures: {len(logs['new_signatures'])}; recurring groups: {len(logs['recurring'])}",'',
                'Private rollback artifacts: checkpoint/checkpoint.json','Prioritized shared repairs: prioritized-repairs.json']
        if report.get('error'):lines+=['','Failure: '+report['error']]
        (run/'README.md').write_text('\n'.join(lines)+'\n',encoding='utf-8')
    print(json.dumps(dict(report=str(run/'run.json'),status=report['status'],regression_free=report['regression_free'],content_ready=False)))
    return int(not report['regression_free'])


def main():
    with RunLock(ROOT/'out/qa-automation/runner.lock'):
        return run_pipeline()


if __name__=='__main__':raise SystemExit(main())
