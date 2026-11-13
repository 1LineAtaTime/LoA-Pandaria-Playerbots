"""Run read-only content audits and normal-client gameplay contracts; keep evidence separate."""
import argparse,json,sys,subprocess,time,hashlib
from datetime import datetime,timezone
from pathlib import Path
from starter_campaign import Database,ROOT
from qa_spec import load_suite
from qa_report import summarize
from qa_inventory import inventory

AUDITS=[
 ('objective_quest','SELECT o.questId,o.id FROM quest_objective o LEFT JOIN quest_template q ON q.ID=o.questId WHERE q.ID IS NULL'),
 ('objective_creature','SELECT o.questId,o.id,o.objectId FROM quest_objective o LEFT JOIN creature_template c ON c.entry=o.objectId WHERE o.type IN (0,3) AND o.objectId>0 AND c.entry IS NULL'),
 ('objective_item','SELECT o.questId,o.id,o.objectId FROM quest_objective o LEFT JOIN item_template i ON i.entry=o.objectId WHERE o.type=1 AND o.objectId>0 AND i.entry IS NULL'),
 ('objective_gameobject','SELECT o.questId,o.id,o.objectId FROM quest_objective o LEFT JOIN gameobject_template g ON g.entry=o.objectId WHERE o.type=2 AND o.objectId>0 AND g.entry IS NULL'),
 ('quest_start_target','SELECT r.id,r.quest FROM creature_queststarter r LEFT JOIN creature_template c ON c.entry=r.id LEFT JOIN quest_template q ON q.ID=r.quest WHERE c.entry IS NULL OR q.ID IS NULL'),
 ('quest_end_target','SELECT r.id,r.quest FROM creature_questender r LEFT JOIN creature_template c ON c.entry=r.id LEFT JOIN quest_template q ON q.ID=r.quest WHERE c.entry IS NULL OR q.ID IS NULL'),
 ('waypoint_ownership','SELECT a.guid,a.path_id FROM creature_addon a LEFT JOIN creature c ON c.guid=a.guid WHERE c.guid IS NULL'),
 ('waypoint_path','SELECT DISTINCT a.guid,a.path_id FROM creature_addon a LEFT JOIN waypoint_data w ON w.id=a.path_id WHERE a.path_id<>0 AND w.id IS NULL'),
 ('active_smartai_ownership',"SELECT DISTINCT s.entryorguid FROM smart_scripts s JOIN creature_template c ON c.entry=s.entryorguid WHERE s.source_type=0 AND c.AIName NOT IN ('SmartAI','SmartVehicleAI')"),
 ('shop_item_reference','SELECT p.id,p.itemId FROM battle_pay_product_items p LEFT JOIN item_template i ON i.entry=p.itemId WHERE p.itemId<>0 AND i.entry IS NULL')]

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--config',type=Path,default=ROOT/'exports/worldserver.conf');p.add_argument('--client',type=Path,default=ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe');p.add_argument('--suite',type=Path,default=Path(__file__).with_name('qa-scenarios.json'));p.add_argument('--report-dir',type=Path,required=True);p.add_argument('--world-pid',type=int);p.add_argument('--audit-only',action='store_true');a=p.parse_args()
 suite=load_suite(a.suite) # validate before creating any test accounts
 if not a.audit_only and not a.world_pid:p.error('--world-pid required for native cleanup')
 a.report_dir.mkdir(parents=True,exist_ok=True);db=Database(a.config,a.client)
 report=dict(timestamp_utc=datetime.now(timezone.utc).isoformat(),revision=subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),suite=suite['name'],data_audits=[],gameplay=[],data_checks_are_gameplay=False,cleanup=[])
 report['suite_sha256']=hashlib.sha256(a.suite.read_bytes()).hexdigest()
 report['harness_sha256']={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in Path(__file__).parent.glob('qa_*.py')}
 report['harness_sha256']['starter_campaign.py']=hashlib.sha256(Path(__file__).with_name('starter_campaign.py').read_bytes()).hexdigest()
 report['harness_sha256']['headless.py']=hashlib.sha256(Path(__file__).with_name('headless.py').read_bytes()).hexdigest()
 report['harness_sha256']['automation_evidence.py']=hashlib.sha256((ROOT/'contrib/qa-automation/evidence.py').read_bytes()).hexdigest()
 for ident,query in AUDITS:
  try:
   rows=db.q('World',query);result=dict(id=ident,status='failed' if rows else 'passed',violation_count=len(rows),sample=rows[:20])
  except Exception as e:result=dict(id=ident,status='blocked',reason=str(e))
  report['data_audits'].append(result)
 for table in ['quest_template','creature_template','item_template']:
  report.setdefault('content_inventory',{})[table]=int(db.q('World','SELECT COUNT(*) FROM '+table)[0][0])
 if not a.audit_only:
  child=a.report_dir/'gameplay.json'
  if child.exists():child.unlink() # never mistake a previous run for current evidence
  cmd=[sys.executable,str(Path(__file__).with_name('starter_campaign.py')),'--suite',str(a.suite),'--config',str(a.config),'--client',str(a.client),'--report',str(child),'--world-pid',str(a.world_pid)]
  process=subprocess.run(cmd) # inherit handles: Windows console cleanup must not run behind captured pipes
  if child.exists():
   data=json.loads(child.read_text());report['gameplay']=data['results'];report['cleanup']=data['cleanup']
   expected_accounts={r['account'] for r in data['results'] if r.get('account_created')}
   cleaned_accounts={r['account'] for r in data['cleanup'] if r['account_absent'] and r['character_absent']}
   if expected_accounts!=cleaned_accounts:report['gameplay'].append(dict(id='test_account_cleanup',status='failed'))
  else:report['gameplay']=[dict(id='campaign_process',status='blocked',reason='No report; exit '+str(process.returncode))]
 else:report['gameplay']=[dict(id=c['id'],status='untested',reason='Audit-only run') for c in suite['scenarios']]
 report['quest_coverage']=inventory(db,a.report_dir,report['gameplay'])
 report['gameplay_summary']=summarize(report['gameplay']);report['data_summary']=summarize(report['data_audits']);report['all_content_validated']=False
 (a.report_dir/'qa-report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
 lines=['# Automated QA results','', 'Gameplay: '+str(report['gameplay_summary']['counts']),'Data audits: '+str(report['data_summary']['counts']),'','Passing data checks does not establish gameplay correctness.','', '| Scenario | Status | Last step / reason |','|---|---|---|']
 for r in report['gameplay']:lines.append('| '+r['id']+' | '+r['status']+' | '+str(r.get('error',r.get('reason',r.get('last_step','')))).replace('|','/').replace('\n',' ')+' |')
 (a.report_dir/'qa-report.md').write_text('\n'.join(lines)+'\n',encoding='utf-8')
 print(json.dumps(dict(report=str(a.report_dir/'qa-report.json'),gameplay=report['gameplay_summary'],data=report['data_summary'])))
 return int(not report['gameplay_summary']['ready'] or not report['data_summary']['ready'])
if __name__=='__main__':sys.exit(main())
