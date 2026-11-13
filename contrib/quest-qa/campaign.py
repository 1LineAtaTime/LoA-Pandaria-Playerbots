"""Reviewed normal-power lifecycle contracts, durable stages and dependency-aware batches.

Only owned isolated QA identities are mutable. No objective,level,gear or reward
grants occur in acceptance workers. A bounded contract never certifies a quest.
"""
import argparse
import collections
import concurrent.futures
import hashlib
import json
import math
import re
from pathlib import Path
import secrets
import socket
import subprocess
import sys
import time
import traceback
from datetime import datetime,timezone
from pool import Pool,ROOT
from catalogue_matrix import playable_pairs
from protocol import QuestPackets
from qa_actions import Workflow
from evidence import write_json

HERE=Path(__file__).resolve().parent


def digest(value):return hashlib.sha256(json.dumps(value,sort_keys=True).encode()).hexdigest()
def now():return datetime.now(timezone.utc).isoformat()


def scenarios():
    contracts=json.loads((HERE/'contracts.json').read_text())['contracts'];pairs=playable_pairs(ROOT/'exports/dbc/CharBaseInfo.dbc');result=[]
    for contract in contracts:
        for race,klass in pairs:
            if race!=contract['race'] or (klass==6 if contract['classes']=='nonheroic' else klass not in contract['classes']):continue
            c={**contract,'class_id':klass,'level':1};c['scenario']=f"q{c['quest']}-r{race}-c{klass}-l1";result.append(c)
    return result


def dependencies(pool,c):
    ids=','.join(map(str,{c['giver'],c['ender'],c.get('kill',{}).get('entry',0)}));qid=c['quest']
    queries=[f'SELECT * FROM quest_template WHERE ID IN({qid},{c["followup"]}) ORDER BY ID',f'SELECT * FROM quest_template_addon WHERE ID IN({qid},{c["followup"]}) ORDER BY ID',f'SELECT * FROM quest_objective WHERE questId IN({qid},{c["followup"]}) ORDER BY id',f'SELECT * FROM creature_template WHERE entry IN({ids}) ORDER BY entry',f'SELECT * FROM creature WHERE id IN({ids}) ORDER BY guid',f'SELECT * FROM smart_scripts WHERE entryorguid IN({ids}) ORDER BY entryorguid,source_type,id',f'SELECT * FROM conditions WHERE SourceEntry IN({ids},{qid},{c["followup"]}) ORDER BY SourceTypeOrReferenceId,SourceGroup,SourceEntry,SourceId,ElseGroup,ConditionTypeOrReference',f'SELECT * FROM playercreateinfo WHERE race={c["race"]} AND class={c["class_id"]}']
    queries += [f'SELECT * FROM creature_queststarter WHERE quest IN({qid},{c["followup"]}) ORDER BY id,quest',f'SELECT * FROM creature_questender WHERE quest IN({qid},{c["followup"]}) ORDER BY id,quest',f'SELECT * FROM creature_template_addon WHERE entry IN({ids}) ORDER BY entry']
    if not hasattr(pool,'global_reference_signature'):
        # Conservative fallback for incompletely mapped transitive dependencies.
        # Never reuse a pass when item/spell/starting-skill data changed silently.
        names=['item_template','spell_area','spell_script_names','reputation_reward_rate','reputation_spillover_template','playercreateinfo_spell_custom','playercreateinfo_cast_spell','playercreateinfo_item','playercreateinfo_skills','playercreateinfo_action']
        pool.global_reference_signature=pool.db.q('World','CHECKSUM TABLE '+','.join('`'+n+'`' for n in names)+' EXTENDED')
        if any(value=='NULL' for _,value in pool.global_reference_signature):raise RuntimeError('Reference checksum unavailable; cache cannot be trusted')
    return digest([pool.global_reference_signature]+[pool.db.q('World',q) for q in queries])


def identity(pool,c):
    files=[HERE/'contracts.json',HERE/'campaign.py',HERE/'pool.py',HERE/'protocol.py',ROOT/'contrib/headless-client/qa_actions.py',ROOT/'contrib/headless-client/headless.py',ROOT/'exports/dbc/QuestXP.dbc',pool.runtime/'worldserver.conf']
    captured=[]
    for path in files:
        raw=path.read_bytes();sha=hashlib.sha256(raw).hexdigest();stored=raw
        if path.suffix=='.conf':stored=re.sub(rb'(?m)^\s*\w*DatabaseInfo\s*=.*$',b'DatabaseInfo = <redacted isolated QA connection>',raw)
        stored_sha=hashlib.sha256(stored).hexdigest();archive=ROOT/'out/quest-qa/frameworks'/stored_sha/path.name;archive.parent.mkdir(parents=True,exist_ok=True)
        if not archive.exists():archive.write_bytes(stored)
        captured.append(dict(source=path.relative_to(ROOT).as_posix(),sha256=sha,archive=archive.relative_to(ROOT).as_posix(),archive_sha256=stored_sha,credentials_redacted=stored!=raw))
    return dict(contract=digest(c),framework=digest([(r['source'],r['sha256']) for r in captured]),framework_files=captured,server_binary=hashlib.sha256((pool.runtime/'worldserver.exe').read_bytes()).hexdigest(),world_dependencies=dependencies(pool,c))


def total_xp(row):
    # Reviewed18414 level1..4 thresholds. Broader levels require a reviewed oracle.
    level,xp=int(row[0]),int(row[1]);thresholds=[400,900,1400]
    if not 1<=level<=4:raise RuntimeError('XP carry oracle supports levels1..4 only')
    return sum(thresholds[:level-1])+xp


def semantic_contract(c):
    keys=('quest','race','class_id','level','giver','ender','xp','money','reputation','items','followup')
    value={k:c[k] for k in keys}
    if c.get('kill'):value['kill']={k:c['kill'][k] for k in ('entry','count')}
    return value


def worker(c,run_dir,fingerprint,resume=None):
    pool=Pool();char=pool.ensure(c['race'],c['class_id']);account=next(a for a in pool.state['accounts'] if a['id']==char['account_id']);lease=pool.lease(account)
    result=dict(run=run_dir.name,scenario=c['scenario'],quest=c['quest'],quest_name=c['name'],race=c['race'],class_id=c['class_id'],faction=c['faction'],level=1,character_guid=char['guid'],account_id=account['id'],server_build=fingerprint['server_binary'],native_source_commit=pool.environment.get('native_source_commit'),source_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),dependencies=fingerprint,started_utc=now(),status='RUNNING',verification='UNVERIFIED',quest_fully_gameplay_verified=False,execution_profile='natural',acceptance_eligible=True,setup=dict(baseline_reset_only=not bool(resume),resumed_without_reset=bool(resume),gm=False,level_granted=False,gear_granted=False,credit_granted=False),stages=[],checks=[])
    target=run_dir/'execution.json';w=None;write_json(target,result)
    def record(step,expected,observed):
        event=dict(step=step,status='PASS',expected=expected,observed=observed,at_utc=now());result['stages'].append(event)
        pool.heartbeat(account['id']);pool.event(run_dir.name,c['scenario'],step,'PASS',expected,observed);write_json(target,result)
    def connect():
        client=pool.connect(account);assert any(ch['name']==char['name'] for ch in client.char_enum())
        flow=Workflow(client,pool.db,char['guid'],dict(quest=c['quest']),result,lambda _:None);packets=QuestPackets();previous=client.auto
        def observe(op,body):previous(op,body);packets.observe(client,op,body)
        client.auto=observe;assert client.player_login(char['guid']);client.pump(1);return client,flow,packets
    def balances():return pool.db.q('Character',f"SELECT level,xp,money FROM characters WHERE guid={char['guid']}")[0]
    def reputation():
        return {f:int((pool.db.q('Character',f"SELECT standing FROM character_reputation WHERE guid={char['guid']} AND faction={int(f)}") or [['0']])[0][0]) for f in c['reputation']}
    def items():
        return {i:int(pool.db.q('Character',f"SELECT COALESCE(SUM(count),0) FROM item_instance WHERE owner_guid={char['guid']} AND itemEntry={int(i)}")[0][0]) for i in c['items']}
    def saved(query,timeout=8):
        deadline=time.monotonic()+timeout
        while time.monotonic()<deadline:
            rows=pool.db.q('Character',query)
            if rows:return rows
            time.sleep(.2)
        return []
    try:
        for source in fingerprint.get('framework_files',[]):
            if hashlib.sha256((ROOT/source['source']).read_bytes()).hexdigest()!=source['sha256']:raise RuntimeError('Queued framework changed before execution: '+source['source'])
        if not resume:pool.reset(char)
        assert not pool.db.q('Login',f"SELECT id FROM account_access WHERE id={account['id']} AND gmlevel<>0"),'Privileged identity rejected'
        assert not pool.db.q('Login',f"SELECT permissionId FROM rbac_account_permissions WHERE accountId={account['id']} AND granted=1"),'Explicit privilege grants rejected'
        assert int(balances()[0])==1,'Acceptance body is not level1'
        record('natural_profile',dict(level=1,gm=False,fixture_grants=False),dict(level=1,gm=False,fixture_grants=False,baseline_sha256=char['snapshot_sha256']))
        if resume:
            source=Path(resume).resolve()
            if not source.is_relative_to(ROOT/'out/quest-qa/runs'):raise RuntimeError('Resume evidence must belong to the QA ledger')
            old=json.loads(source.read_text())
            assert old['scenario']==c['scenario'] and old['character_guid']==char['guid']
            assert old.get('execution_profile')=='natural' and old.get('error') in ('Completed kill objectives not persisted','Actor outside bounded route')
            if old.get('error')=='Actor outside bounded route':assert old.get('resumed_from'),'Only a previously verified combat checkpoint may resume a return-route failure'
            old_contract=json.loads((source.parent/'job.json').read_text())['contract']
            assert semantic_contract(old_contract)==semantic_contract(c),'Objectives,rewards or eligibility differ from the earned checkpoint'
            assert old['dependencies']['server_binary']==fingerprint['server_binary'] and old['dependencies']['world_dependencies']==fingerprint['world_dependencies']
            assert any(s['step']=='actual_combat_and_credit' and s['status']=='PASS' for s in old['stages'])
            assert saved(f"SELECT quest FROM character_queststatus WHERE guid={char['guid']} AND quest={c['quest']} AND status=1")
            assert not pool.db.q('Character',f"SELECT quest FROM character_queststatus_rewarded WHERE guid={char['guid']} AND quest={c['quest']}")
            for other in (ROOT/'out/quest-qa/runs').glob('*/execution.json'):
                if other.resolve() in (source,target.resolve()):continue
                data=json.loads(other.read_text())
                if data.get('character_guid')==char['guid'] and data.get('finished_utc','')>old['finished_utc']:raise RuntimeError('A later run touched the checkpoint character')
            result['stages']=old['stages']+result['stages'];result['objective_credit']=old['objective_credit'];result['combat_attempts']=old.get('combat_attempts',[])
            result['resumed_from']=dict(path=str(source.relative_to(ROOT)),sha256=hashlib.sha256(source.read_bytes()).hexdigest(),reason='Async save completed after the original single-read assertion; no objectives granted or rerun')
            record('verified_persisted_combat_checkpoint',dict(quest=c['quest'],status=1,credit=c['kill']['count']),dict(status=1,parent_evidence=result['resumed_from']))
        w,flow,packets=connect()
        record('initial_normal_spellbook',dict(source='18414 initial/learned spell packets'),dict(spells=sorted(flow.tracker.known_spells),health=flow.tracker.health.get(w.char_guid),max_health=flow.tracker.max_health.get(w.char_guid)))
        if not resume:
            flow.approach(c['giver']);giver=flow.target();w.quest_hello(giver);w.pump(1)
            autoaccepted=c['quest'] in flow.tracker.quest_slots.values()
            assert c['quest'] in packets.offered or autoaccepted,'Quest neither offered nor naturally auto-accepted after baseline login'
            record('visible_giver_and_offer',dict(entry=c['giver'],quest=c['quest']),dict(guid=giver,offer_packets=packets.events,first_login_autoaccepted=autoaccepted))
            if not autoaccepted:w.quest_accept(c['quest'],giver);w.pump(1)
            assert c['quest'] in flow.tracker.quest_slots.values(),'Accepted quest absent from client quest log'
            record('normal_accept_and_quest_log',c['quest'],dict(quest_slots=flow.tracker.quest_slots))
        if c.get('kill') and not resume:
            flow.kill(c['kill']);record('actual_combat_and_credit',c['kill'],dict(credits=flow.progress,attempts=result.get('combat_attempts',[])))
            # Separate combat XP from quest reward XP through a real save/relogin.
            assert w.logout();w.s.close();w=None
            assert saved(f"SELECT quest FROM character_queststatus WHERE guid={char['guid']} AND quest={c['quest']} AND status=1"),'Completed kill objectives not persisted'
            w,flow,packets=connect()
        before=balances();before_rep=reputation();before_items=items()
        if flow.tracker.coinage is not None:before[2]=str(flow.tracker.coinage)
        record('pre_reward_observed_balances',dict(source='client_coinage_and_persisted_xp'),dict(balances=before,client_coinage=flow.tracker.coinage,known_spells=sorted(flow.tracker.known_spells)))
        flow.approach(c['ender'],limit=c.get('return_distance',100));ender=flow.target();w.quest_hello(ender);w.pump(.5);w.quest_complete(c['quest'],ender);w.pump(.5);w.quest_reward(c['quest'],0,ender);w.pump(1)
        # A duplicate reward request must not pay twice or duplicate the item.
        w.quest_reward(c['quest'],0,ender);w.pump(.5);followup=list(packets.events)
        assert w.logout();w.s.close();w=None
        assert saved(f"SELECT quest FROM character_queststatus_rewarded WHERE guid={char['guid']} AND quest={c['quest']}"),'Reward not persisted'
        after=balances();rep=reputation();owned=items()
        assert total_xp(after)-total_xp(before)==c['xp'],f"Quest XP expected{c['xp']}, observed{total_xp(after)-total_xp(before)}"
        assert int(after[2])-int(before[2])==c['money'],f"Money expected{c['money']}, observed{int(after[2])-int(before[2])}"
        for f,n in c['reputation'].items():assert rep[f]-before_rep[f]==n,f'Reputation{f} expected{n}, observed{rep[f]-before_rep[f]}'
        for i,n in c['items'].items():assert owned[i]-before_items[i]==n,f'Reward item{i} expected{n}, observed{owned[i]-before_items[i]}'
        record('turnin_exact_rewards_and_duplicate_rejection',dict(xp=c['xp'],money=c['money'],reputation=c['reputation'],items=c['items']),dict(before=before,after=after,reputation_before=before_rep,reputation_after=rep,item_counts_before=before_items,item_counts_after=owned))
        w,flow,packets=connect();assert c['quest'] not in flow.tracker.quest_slots.values(),'Rewarded quest active after reconnect'
        record('fresh_authenticated_reconnect',dict(reward_persisted=True),dict(rewarded_quest_absent_from_log=True))
        flow.approach(c['ender'],limit=c.get('return_distance',100));ender=flow.target();w.quest_hello(ender);w.pump(1)
        observed_followup=c['followup'] in packets.offered or c['followup'] in flow.tracker.quest_slots.values() or any(c['followup'] in e['quest_ids'] for e in followup)
        assert observed_followup,'Expected follow-up neither offered nor auto-accepted'
        record('followup_available',c['followup'],dict(observed=True,packets=packets.events,auto_accepted=c['followup'] in flow.tracker.quest_slots.values()))
        assert w.logout();w.s.close();w=None
        result.update(status='PASS',verification='GAMEPLAY_VERIFIED',limitations=['Bounded contract only; not exhaustive eligibility or retail accuracy','Visual rendering and all spell effects not established']+([f"Reward spell{c['unverified_reward_cast']} effect not decoded"] if c.get('unverified_reward_cast') else []))
    except Exception as e:
        result.update(status='FAIL' if isinstance(e,AssertionError) else 'BLOCKED',error=str(e),traceback=traceback.format_exc());pool.event(run_dir.name,c['scenario'],'failure',result['status'],'Reviewed contract',str(e))
    finally:
        if w:
            try:w.logout()
            finally:w.s.close()
        pool.release(account,lease);result['finished_utc']=now();write_json(target,result)
    return result


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--worker',type=Path);parser.add_argument('--workers',type=int,default=4);parser.add_argument('--force',action='store_true');parser.add_argument('--select',default='');parser.add_argument('--resume',type=Path);a=parser.parse_args()
    if a.worker:
        job=json.loads(a.worker.read_text());r=worker(job['contract'],a.worker.parent,job['fingerprint'],job.get('resume_from'));print(json.dumps(dict(status=r['status'],scenario=r['scenario'],report=str(a.worker.parent/'execution.json'))));return int(r['status']!='PASS')
    pool=Pool()
    config=(pool.runtime/'worldserver.conf').read_text(encoding='utf-8-sig')
    for setting in ('Rate.Drop.Money','Rate.XP.Quest'):
        match=re.search(r'^\s*'+re.escape(setting)+r'\s*=\s*([0-9.]+)',config,re.M)
        if not match or float(match[1])!=1:
            raise RuntimeError('Reviewed reward oracle requires '+setting+' = 1; configuration was not changed')
    native=pool.environment.get('native_source_commit')
    if not native:raise RuntimeError('QA native source provenance is missing; stage a documented build first')
    changed=subprocess.check_output(['git','diff','--name-only',native,'--','src','dep','modules','cmake','CMakeLists.txt'],cwd=ROOT,text=True).strip()
    untracked=subprocess.check_output(['git','ls-files','--others','--exclude-standard','--','src','dep','modules','cmake'],cwd=ROOT,text=True).strip()
    if changed or untracked:raise RuntimeError('Native source differs from the staged QA build; compile and stage it before crediting tests')
    deadline=time.monotonic()+45
    for port in (3725,8086):
        while True:
            try:
                with socket.create_connection(('127.0.0.1',port),timeout=.5):break
            except OSError:
                if time.monotonic()>=deadline:raise RuntimeError('Owned QA runtime is not ready on port'+str(port))
                time.sleep(.25)
    batch=ROOT/'out/quest-qa/batches'/(datetime.now(timezone.utc).strftime('%Y%m%d-%H%M%S')+'-'+secrets.token_hex(3));batch.mkdir(parents=True)
    previous=[]
    for p in (ROOT/'out/quest-qa/runs').glob('*/execution.json'):
        r=json.loads(p.read_text())
        if r.get('execution_profile')=='natural' and r['status']=='PASS':previous.append(r)
    lanes=collections.defaultdict(list);report=dict(started_utc=now(),execution_profile='natural',workers=max(1,min(a.workers,8)),scheduled=[],cached=[],results=[],coverage_scope='Only reviewed contracts; unimplemented mechanics remain unverified')
    for c in scenarios():
        if a.select and a.select not in c['id'] and a.select!=c['scenario']:continue
        if a.resume and c['scenario']!=json.loads(a.resume.read_text())['scenario']:continue
        char=pool.ensure(c['race'],c['class_id']);fp=identity(pool,c)
        if not a.force and any(r['scenario']==c['scenario'] and r.get('dependencies')==fp for r in previous):report['cached'].append(c['scenario']);continue
        run=ROOT/'out/quest-qa/runs'/(batch.name+'-'+c['scenario']);run.mkdir(parents=True);job=run/'job.json';write_json(job,dict(contract=c,fingerprint=fp,resume_from=str(a.resume.resolve()) if a.resume else None));lanes[char['account_id']].append(job);report['scheduled'].append(c['scenario'])
    write_json(batch/'batch.json',report)
    def lane(jobs):
        rows=[]
        for job in jobs:
            p=subprocess.run([sys.executable,str(Path(__file__).resolve()),'--worker',str(job)],cwd=ROOT,text=True,capture_output=True,timeout=600)
            target=job.parent/'execution.json'
            if not target.exists():
                spec=json.loads(job.read_text());c=spec['contract']
                row=dict(run=job.parent.name,scenario=c['scenario'],quest=c['quest'],quest_name=c['name'],race=c['race'],class_id=c['class_id'],faction=c['faction'],level=1,character_guid=None,account_id=None,source_commit='WORKER_INITIALIZATION_FAILED',server_build=spec['fingerprint']['server_binary'],started_utc=now(),finished_utc=now(),status='BLOCKED',verification='UNVERIFIED',stages=[],checks=[],error=p.stderr[-1500:],execution_profile='natural',acceptance_eligible=False)
                write_json(target,row)
            else:row=json.loads(target.read_text())
            rows.append(dict(scenario=row['scenario'],status=row['status'],error=row.get('error'),evidence=str(target.relative_to(ROOT))))
        return rows
    with concurrent.futures.ThreadPoolExecutor(max_workers=report['workers']) as executor:
        futures=[executor.submit(lane,jobs) for jobs in lanes.values()]
        for future in concurrent.futures.as_completed(futures):
            report['results'].extend(future.result());write_json(batch/'batch.json',report)
    report.update(finished_utc=now(),counts=dict(collections.Counter(r['status'] for r in report['results'])));write_json(batch/'batch.json',report)
    print(json.dumps(dict(report=str(batch/'batch.json'),scheduled=len(report['scheduled']),cached=len(report['cached']),counts=report['counts'])))
    return int(any(r['status']!='PASS' for r in report['results']))


if __name__=='__main__':sys.exit(main() or 0)
