"""Generate a complete quest catalog and explicit race/class/level boundary matrix.

This is test planning, not simulated gameplay. Unexpanded dynamic eligibility
axes and unavailable mechanic drivers remain visible blockers.
"""
import argparse
import collections
import csv
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys
from datetime import datetime,timezone
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'contrib/headless-client'))
from starter_campaign import Database

STAGES=('offered','accepted','objectives_executed','objective_credit','complete','turned_in','items_consumed','rewards','reconnect','repeatability','followup','negative_cases')


def level_cases(minimum,maximum):
    minimum=max(1,int(minimum));maximum=min(90,int(maximum) or 90)
    values=[('at_minimum',minimum),('at_maximum',maximum)]
    if minimum>1:values.append(('below_minimum',minimum-1))
    if maximum<90:values.append(('above_maximum',maximum+1))
    seen=set()
    for tag,level in values:
        if 1<=level<=90 and level not in seen:
            seen.add(level);yield tag,level


def playable_pairs(path):
    raw=path.read_bytes();magic,n,fields,size,strings=struct.unpack_from('<4s4I',raw)
    if magic!=b'WDBC' or fields!=3 or size!=8 or len(raw)!=20+n*size+strings:
        raise ValueError('Unsupported 18414 CharBaseInfo layout')
    playable={1,2,3,4,5,6,7,8,9,10,11,22,24,25,26}
    return sorted({struct.unpack_from('<IBB2x',raw,20+i*8)[1:] for i in range(n) if struct.unpack_from('<IBB2x',raw,20+i*8)[1] in playable})


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output',type=Path,default=ROOT/'out/quest-qa/catalogue')
    p.add_argument('--config',type=Path,default=ROOT/'exports/worldserver.conf')
    a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True)
    db=Database(a.config,ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
    rows=db.q('World','SELECT q.ID,HEX(q.LogTitle),q.MinLevel,COALESCE(NULLIF(x.MaxLevel,0),NULLIF(q.MaxLevel,0),90),q.AllowableRaces,COALESCE(x.AllowableClasses,0),COALESCE(x.PrevQuestID,0),COALESCE(x.RequiredSkillID,0),COALESCE(x.RequiredSkillPoints,0),COALESCE(x.RequiredMinRepFaction,0),COALESCE(x.RequiredMinRepValue,0),COALESCE(x.RequiredMaxRepFaction,0),COALESCE(x.RequiredMaxRepValue,0),COALESCE(x.ExclusiveGroup,0),q.Flags,COALESCE(x.SpecialFlags,0),COALESCE(x.NextQuestID,0),q.RewardNextQuest FROM quest_template q LEFT JOIN quest_template_addon x ON x.ID=q.ID ORDER BY q.ID')
    objectives=collections.defaultdict(list)
    for qid,oid,kind,target,amount in db.q('World','SELECT questId,id,type,objectId,amount FROM quest_objective ORDER BY questId,id'):
        objectives[int(qid)].append(dict(id=int(oid),type=int(kind),target=int(target),amount=int(amount)))
    starts=collections.defaultdict(list)
    for kind,ident,qid in db.q('World',"SELECT 'creature',id,quest FROM creature_queststarter UNION ALL SELECT 'gameobject',id,quest FROM gameobject_queststarter"):
        starts[int(qid)].append(dict(kind=kind,id=int(ident)))
    ends=collections.defaultdict(list)
    for kind,ident,qid in db.q('World',"SELECT 'creature',id,quest FROM creature_questender UNION ALL SELECT 'gameobject',id,quest FROM gameobject_questender"):
        ends[int(qid)].append(dict(kind=kind,id=int(ident)))
    dbc=ROOT/'exports/dbc/CharBaseInfo.dbc';pairs=playable_pairs(dbc)
    configured={tuple(map(int,r)) for r in db.q('World','SELECT race,class FROM playercreateinfo')}
    pair_discrepancies=dict(client_only=sorted(set(pairs)-configured),world_only=sorted(configured-set(pairs)))
    source=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip()
    profiles=[];counts=collections.Counter();factions={1:'Alliance',3:'Alliance',4:'Alliance',7:'Alliance',11:'Alliance',22:'Alliance',25:'Alliance',24:'Neutral'}
    with (a.output/'QUEST_QA_MATRIX.csv').open('w',newline='',encoding='utf-8') as stream:
        writer=csv.writer(stream);writer.writerow(['scenario_id','quest_id','quest_name','race','class','faction_state','level_boundary','level','static_eligibility','eligibility_profile','objective_types','verification','execution_status','dynamic_axes'])
        for row in rows:
            qid=int(row[0]);name=bytes.fromhex(row[1]).decode('utf-8');minimum,maximum,races,classes=map(int,row[2:6])
            profile=dict(quest=qid,name=name,min_level=minimum,max_level=maximum,race_mask=races,class_mask=classes,previous_quest=int(row[6]),skill=int(row[7]),skill_points=int(row[8]),min_rep_faction=int(row[9]),min_rep=int(row[10]),max_rep_faction=int(row[11]),max_rep=int(row[12]),exclusive_group=int(row[13]),flags=int(row[14]),special_flags=int(row[15]),next_quest=int(row[16]),reward_next_quest=int(row[17]),objectives=objectives[qid],givers=starts[qid],enders=ends[qid],dynamic_axes_status='UNEXPANDED: condition/phase/world-state/spec/party/reset/reward-choice oracle review required',full_gameplay_verified=False)
            profile_id=hashlib.sha256(json.dumps(profile,sort_keys=True).encode()).hexdigest()[:16];profile['profile_id']=profile_id;profiles.append(profile)
            for race,klass in pairs:
                for boundary,level in level_cases(minimum,maximum):
                    eligible=(not races or races&(1<<(race-1))) and (not classes or classes&(1<<(klass-1))) and level>=minimum and level<=maximum
                    category='ALLOW_STATIC_GATES_ONLY' if eligible else 'DENY_STATIC_GATE'
                    scenario=f'q{qid}-r{race}-c{klass}-l{level}'
                    writer.writerow([scenario,qid,name,race,klass,factions.get(race,'Horde'),boundary,level,category,profile_id,'|'.join(map(str,sorted({o['type'] for o in objectives[qid]}))),'UNVERIFIED','NOT_RUN','UNEXPANDED'])
                    counts['scenarios']+=1;counts[category]+=1
            counts['quests']+=1
    (a.output/'eligibility-profiles.json').write_text(json.dumps(profiles,indent=2)+'\n',encoding='utf-8')
    fields=['scenario_id','quest_id','quest_name','race','class','faction','level','character_id','server_build','source_commit','started_utc','finished_utc','verification','status',*STAGES,'expected','observed','error','evidence']
    now=datetime.now(timezone.utc).isoformat()
    with (a.output/'QUEST_QA_EXECUTION_RESULTS.csv').open('w',newline='',encoding='utf-8') as f:
        writer=csv.DictWriter(f,fieldnames=fields);writer.writeheader()
        for q in profiles:
            writer.writerow(dict(scenario_id='catalog-'+str(q['quest']),quest_id=q['quest'],quest_name=q['name'],source_commit=source,started_utc=now,finished_utc=now,verification='STRUCTURAL_CATALOG_ONLY',status='UNVERIFIED',**{s:'NOT_TESTED' for s in STAGES},expected='Actual objective execution and full lifecycle evidence',observed='Catalog enumerated; no gameplay driver dispatched by this command',error='MECHANIC_OR_ELIGIBILITY_DRIVER_NOT_YET_AVAILABLE',evidence='eligibility-profiles.json#'+q['profile_id']))
    with (a.output/'QUEST_QA_FAILURES.csv').open('w',newline='',encoding='utf-8') as f:
        writer=csv.DictWriter(f,fieldnames=fields);writer.writeheader()
        writer.writerow(dict(scenario_id='reported-sacrifices-missing-actionbar',quest_id=14212,quest_name='Sacrifices',race=22,**{'class':4},character_id='ExampleCharacter (user)',server_build='f5a152480b',source_commit=source,verification='USER_REPORTED',status='FAIL',objectives_executed='BLOCKED',expected='Throw Torch67063 usable from vehicle;30 legitimate roundup credits;turn-in',observed='Horse follows route; ability absent; horse dies',error='VEHICLE_ACTIONBAR_OR_CONTROL_PATH',evidence='User report; previous ride fixture tested only boarding/logout'))
    summary=dict(created_utc=now,source_commit=source,quest_count=counts['quests'],scenario_count=counts['scenarios'],static_gate_counts={k:v for k,v in counts.items() if k not in ('quests','scenarios')},playable_pair_count=len(pairs),pair_discrepancies=pair_discrepancies,client_reference_sha256=hashlib.sha256(dbc.read_bytes()).hexdigest(),full_gameplay_verified_quests=0,full_gameplay_coverage_percent=0,gameplay_tests_executed_by_this_command=0,all_dynamic_axes_expanded=False)
    (a.output/'coverage.json').write_text(json.dumps(summary,indent=2)+'\n')
    (a.output/'QUEST_QA_COVERAGE.md').write_text('# Quest QA coverage\n\n'+f"Catalog: {counts['quests']:,} quests. Matrix: {counts['scenarios']:,} explicit race/class/level cases across {len(pairs)} client-defined pairs.\n\n"+'Full quest gameplay verification: **0%**. Catalog enumeration is not a gameplay attempt. Historical bounded assertion passes remain partial evidence, not full-catalog certification. Dynamic condition, phase, specification, party and reset axes are UNEXPANDED; this is not yet an exhaustive eligibility Cartesian product.\n')
    (a.output/'QUEST_QA_BLOCKERS.md').write_text('# Quest QA blockers\n\n- Sacrifices14212: missing vehicle action bar/control; objective use, credit, survival and turn-in unverified.\n- Complete reusable lifecycle drivers are absent for most mechanics, scripted phases, instances, professions, bosses and special spells.\n- Dynamic eligibility and independent expected outcomes require review; database values are self-consistency inputs, not a retail oracle.\n- Historical human six-kill workflow reached5/6; GUID retry/respawn handling is a harness blocker, not proof of a broken quest.\n- Invincible or high-damage fixtures cannot prove encounter phases, death mechanics, balance or negative eligibility cases.\n')
    (a.output/'QUEST_QA_RUNBOOK.md').write_text('# Quest QA runbook\n\n1. Audit prior evidence; retain assertion results while reclassifying whole-quest overclaims.\n2. Provision only owned mop_loa_qa_ schemas with environment.py. Keep live ports3724/8085 untouched.\n3. Run reusable pool workers against QA ports3725/8086 only; persist leases, snapshots and per-stage evidence.\n4. Controlled level/gear/prerequisite setup is not objective execution. Never grant completion as a test.\n5. Export every attempted stage and failure. Missing drivers are BLOCKED/UNVERIFIED, never PASS.\n6. Back up and repair proven defects, then replay the original scenario and affected dependencies.\n7. Regenerate matrix after changed dependencies; preserve prior run ledgers.\n\nThis catalog exporter does not launch accounts or gameplay. The managed runtime/runner is a separate implementation and execution step.\n')
    print(json.dumps(summary))


if __name__=='__main__':main()
