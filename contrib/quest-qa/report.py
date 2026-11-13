"""Combine durable attempts with the complete catalog, without inflating coverage."""
import collections
import csv
import json
import os
import sys
from pathlib import Path
from pool import ROOT
from catalogue_matrix import STAGES


def main():
    root=ROOT/'out/quest-qa';catalog=root/'catalogue'
    profiles=json.loads((catalog/'eligibility-profiles.json').read_text())
    fields=['scenario_id','quest_id','quest_name','race','class','faction','level','character_id','server_build','source_commit','started_utc','finished_utc','verification','status',*STAGES,'expected','observed','error','evidence']
    results=[];failures=[];passed=[]
    for path in sorted((root/'runs').glob('*/execution.json')):
        r=json.loads(path.read_text());steps=r.get('stages',[])
        # Earlier failures exposed framework assumptions, not game defects.
        # Preserve original raw records and explicitly annotate that disposition.
        harness_error=not steps or r.get('error')=='Cutting Teeth follow-up not offered'
        status='BLOCKED_HARNESS' if r['status']!='PASS' and harness_error else r['status']
        row=dict(scenario_id=r['scenario'],quest_id=r['quest'],quest_name=r['quest_name'],race=r['race'],**{'class':r['class_id']},faction=r['faction'],level=r['level'],character_id=r['character_guid'],server_build=r['server_build'],source_commit=r.get('source_commit',r['server_build']),started_utc=r['started_utc'],finished_utc=r.get('finished_utc',''),verification=r['verification'],status=status,expected=json.dumps([s['expected'] for s in steps]),observed=json.dumps(steps),error=r.get('error',''),evidence=str(path.relative_to(ROOT)))
        for stage in STAGES:row[stage]='SEE_RECORDED_STAGES' if steps else 'NOT_TESTED'
        results.append(row)
        if status!='PASS':failures.append(row)
        else:passed.append(row)
    for path in sorted((root/'sacrifices').glob('*/trace.json')):
        r=json.loads(path.read_text());status={'INTEGRATION_VERIFIED':'PASS','INTEGRATION_OBSERVED':'PARTIAL'}.get(r['status'],r['status'])
        if r.get('error')=='Normal turn-in did not persist the quest reward':status='FAIL_GAMEPLAY'
        if r.get('error')=='Local reviewed quest reward expected75 copper':status='BLOCKED_HARNESS'
        row=dict(scenario_id='q14212-r22-c1-l4-'+path.parent.name,quest_id=14212,quest_name='Sacrifices',race=22,**{'class':1},faction='Alliance',level=4,character_id=r['character_guid'],server_build=r.get('binary_sha256','f5a152480b+candidate'),source_commit=r['source_commit'],verification='INTEGRATION_VERIFIED' if status=='PASS' else 'PARTIAL_OR_UNVERIFIED',status=status,expected='Normal accept,ThrowTorch,30 roundup credits,33-point arrival,75copper,XP increase,followup,reconnect and protection cleanup',observed=json.dumps(r),error=r.get('error',''),evidence=str(path.relative_to(ROOT)))
        row.update({s:'NOT_ESTABLISHED' for s in STAGES})
        if r.get('offers'):row['offered']='OBSERVED'
        if r.get('quest_log'):row['accepted']='OBSERVED'
        if r.get('objective_credit'):row['objectives_executed']='TORCH_CASTS';row['objective_credit']=str(r['objective_credit'][-1]['count'])+'/30'
        if r.get('turnin'):row['turned_in']='PERSISTED';row['rewards']=str(r['turnin']['money_delta'])+'COPPER_XP_INCREASE_EXACT_XP_ORACLE_PENDING';row['followup']=str(r['turnin']['followup_offered'])
        if r.get('reconnect'):row['reconnect']='OBSERVED'
        results.append(row)
        if status=='PASS':passed.append(row)
        elif status not in ('PARTIAL','UNVERIFIED'):failures.append(row)
    # Every quest retains an explicit unverified whole-quest row, even if one
    # particular race/class/level lifecycle succeeded.
    catalog_rows=[dict(scenario_id='catalog-'+str(q['quest']),quest_id=q['quest'],quest_name=q['name'],verification='STRUCTURAL_CATALOG_ONLY',status='UNVERIFIED',**{s:'NOT_TESTED' for s in STAGES},expected='All reviewed eligible and negative scenarios',observed='No full eligibility certification',error=q['dynamic_axes_status'],evidence='eligibility-profiles.json#'+q['profile_id']) for q in profiles]
    with (catalog/'QUEST_QA_FAILURES.csv').open(encoding='utf-8',newline='') as f:
        failures += [r for r in csv.DictReader(f) if r['verification']=='USER_REPORTED']
    for name,rows in [('QUEST_QA_EXECUTION_RESULTS.csv',catalog_rows+results),('QUEST_QA_FAILURES.csv',failures)]:
        with (catalog/name).open('w',newline='',encoding='utf-8') as f:
            writer=csv.DictWriter(f,fieldnames=fields);writer.writeheader();writer.writerows(rows)
    counts=collections.Counter(r['status'] for r in results)
    summary=dict(quest_count=len(profiles),attempted_bounded_runs=len(results),bounded_statuses=dict(counts),bounded_gameplay_verified=len(passed),full_gameplay_verified_quests=0,all_dynamic_axes_expanded=False)
    latest={}
    for row in sorted(results,key=lambda r:r.get('finished_utc','')):
        key=f"q{row['quest_id']}-r{row['race']}-c{row['class']}-l{row['level']}";latest[key]=row
    summary['unique_bounded_cases_with_latest_pass']=sum(r['status']=='PASS' for r in latest.values())
    summary['latest_case_statuses']=dict(collections.Counter(r['status'] for r in latest.values()))
    with (catalog/'QUEST_QA_LATEST_CASES.csv').open('w',newline='',encoding='utf-8') as f:
        writer=csv.DictWriter(f,fieldnames=fields);writer.writeheader();writer.writerows(latest.values())
    (catalog/'execution-summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    (catalog/'QUEST_QA_COVERAGE.md').write_text('# Quest QA coverage\n\n'+f"Catalog: {len(profiles):,} quests;6,183,814 planned race/class/level-boundary scenarios. Dynamic eligibility axes remain unexpanded.\n\n"+f"Recorded bounded runs: {len(results)}; successful lifecycle runs: {len(passed)}. Statuses: {dict(counts)}.\n\n"+'Full quest gameplay certification: **0 quests**. One successful scenario does not establish every eligibility case, mechanic or reward choice. No catalog rows are gameplay passes. Original failed framework runs are retained with explicit harness dispositions.\n')
    horse=any(int(r['quest_id'])==14212 for r in passed)
    (catalog/'QUEST_QA_BLOCKERS.md').write_text('# Remaining blockers\n\n'+('- Sacrifices14212: casting,30credits,route,turn-in,reward,follow-up,reconnect and flag cleanup passed in the isolated client. Visual UI acceptance and exact independent XP oracle remain open.\n' if horse else '- Sacrifices14212: full vehicle lifecycle not yet verified.\n')+'- Most mechanic drivers and dynamic eligibility axes remain unimplemented/unexpanded.\n- Fishing,mining,herbalism,crafting,all spells/items,encounters,dungeons and raids are not certified by quest catalog enumeration.\n- Shared/account-wide reset and dependency invalidation remain incomplete.\n- High damage or invincibility cannot establish encounter mechanics or negative eligibility.\n')
    if '--update-matrix' in sys.argv:
        keys={f"q{r['quest_id']}-r{r['race']}-c{r['class']}-l{r['level']}" for r in passed}
        matrix=catalog/'QUEST_QA_MATRIX.csv';temp=catalog/'QUEST_QA_MATRIX.updating.csv';updated=0
        with matrix.open(newline='',encoding='utf-8') as src,temp.open('w',newline='',encoding='utf-8') as dst:
            reader=csv.DictReader(src);writer=csv.DictWriter(dst,fieldnames=reader.fieldnames);writer.writeheader()
            for row in reader:
                if row['scenario_id'] in keys:
                    row['verification']='BOUNDED_GAMEPLAY_CASE_VERIFIED';row['execution_status']='SEE_EXECUTION_RESULTS';updated+=1
                writer.writerow(row)
        os.replace(temp,matrix);summary['matrix_rows_linked_to_bounded_passes']=updated
    print(json.dumps(summary))


if __name__=='__main__':main()
