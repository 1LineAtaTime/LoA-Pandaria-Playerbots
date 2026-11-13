"""Bounded, resumable observer for the EXISTING native self-bot quest AI.

No quest drivers, teleports, resets, XP, item or equipment grants. Persistent
bodies use the existing pool lease, normal socket login and Playerbots commands.
"""
import argparse, datetime, hashlib, json, subprocess, sys, time
from pathlib import Path
from pool import Pool
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'contrib/headless-client'))
from qa_actions import ObservedWorld
from adventure_progress import ProgressWatchdog
from adventure_notes import Notes
from adventure_campaign import world_pid
from evidence import write_json
import headless

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--guid',type=int,required=True);p.add_argument('--world-pid',type=int,required=True)
    p.add_argument('--seconds',type=int,default=300);p.add_argument('--run',required=True)
    a=p.parse_args()
    if not 30<=a.seconds<=3600 or not a.run.replace('-','').replace('_','').isalnum():p.error('Bounded duration and simple run ID required')
    pool=Pool();c=next(c for c in pool.state['characters'] if c['guid']==a.guid)
    if world_pid(pool.runtime)!=a.world_pid:raise RuntimeError('World PID does not match the isolated QA executable')
    if c.get('purpose')!='persistent-adventure':raise RuntimeError('Designate a persistent adventure body first')
    pool.validate(c);account=next(x for x in pool.state['accounts'] if x['id']==c['account_id'])
    folder=ROOT/'out/quest-qa/adventure'/a.run;folder.mkdir(parents=True,exist_ok=False)
    lease=pool.lease(account,a.seconds+180);w=None
    write_json(folder/'worker-control.json',dict(worker_pid=__import__('os').getpid(),account_id=account['id'],lease_owner=lease))
    stream=(folder/'events.jsonl').open('x',encoding='utf-8')
    notes=Notes(folder/'notes.md',a.run,a.guid)
    result=dict(run_id=a.run,guid=a.guid,profile='natural-native-playerbots',movement_multiplier=1,
                administrative_grants=False,quest_completion_injected=False,milestone1='NOT_EVALUATED',execution_status='RUNNING',selfbot_enabled=False,chat=[])
    result['native_source']=pool.environment.get('native_source_commit')
    result['native_worktree_dirty']=pool.environment.get('native_worktree_dirty',False)
    result['native_patch_sha256']=pool.environment.get('native_patch_sha256')
    result['controller_sha256']=hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
    result['progress_observer_sha256']=hashlib.sha256((Path(__file__).parent/'adventure_progress.py').read_bytes()).hexdigest()
    result['binary_sha256_started']=hashlib.sha256((pool.runtime/'worldserver.exe').read_bytes()).hexdigest()
    native_log=pool.runtime/'logs/Server.log';log_offset=native_log.stat().st_size
    def event(kind,**data):
        observed=dict(run_id=a.run,guid=a.guid,timestamp=datetime.datetime.now(datetime.timezone.utc).isoformat(),event=kind,**data)
        stream.write(json.dumps(observed)+'\n');stream.flush();notes.record(observed)
    def console(text):
        subprocess.run([sys.executable,str(ROOT/'contrib/windows/Send-ServerConsoleCommand.py'),str(a.world_pid),text],check=True,capture_output=True)
        event('controller_command',command=text)
    def state():
        return dict(character=pool.db.q('Character',f'SELECT level,xp,money,map,position_x,position_y,position_z,health,totaltime FROM characters WHERE guid={a.guid}'),
                    quests=pool.db.q('Character',f'SELECT quest,status FROM character_queststatus WHERE guid={a.guid} ORDER BY quest'),
                    objectives=pool.db.q('Character',f'SELECT objectiveId,amount FROM character_queststatus_objective WHERE guid={a.guid} ORDER BY objectiveId'),
                    rewarded=pool.db.q('Character',f'SELECT quest FROM character_queststatus_rewarded WHERE guid={a.guid} ORDER BY quest'),
                    equipment=pool.db.q('Character',f'SELECT v.slot,i.itemEntry,i.guid FROM character_inventory v JOIN item_instance i ON i.guid=v.item WHERE v.guid={a.guid} AND v.bag=0 AND v.slot<19 ORDER BY v.slot'))
    try:
        before=state();event('initial_state',actual=before)
        w=pool.connect(account);w.char_enum()
        w.char_name=c['name'];w.char_race=c['race'];w.language=headless.RACE_LANGUAGE.get(c['race'],headless.LANG_COMMON)
        tracker=ObservedWorld(w);prior=w.auto
        def observe(op,body):
            prior(op,body)
            if op==headless.SMSG_MESSAGECHAT:
                chat=w._parse_messagechat(body)
                if chat:
                    result['chat'].append(chat['message']);event('chat',message=chat['message'])
                    if 'Enable player botAI' in chat['message']:result['selfbot_enabled']=True
                    if 'Disable player botAI' in chat['message']:result['selfbot_enabled']=False
            if op==0x1645:event('objective_packet',opcode=hex(op),payload=body.hex())
            if op==headless.SMSG_CAST_FAILED:event('spell_failure',opcode=hex(op),payload=body.hex())
            try:tracker.handle(op,body,time.time())
            except (ValueError,KeyError,IndexError,EOFError):tracker.decode_errors+=1
        w.auto=observe;assert w.player_login(a.guid);w.pump(2)
        result['initial_spellbook']=sorted(tracker.known_spells)
        event('initial_spellbook',actual=result['initial_spellbook'])
        w.move_goto(w.pos[0]+.05,w.pos[1],w.pos[2]);w.pump(.5)
        w.send_chat('say','.npcbot self');w.pump(1)
        assert result['selfbot_enabled'], 'Native self-bot enable was not acknowledged: '+repr(result['chat'])
        result['selfbot_enable_acknowledged']=True
        console('server set loglevel logger playerbots 1')
        console('server set loglevel appender Server 1')
        console(f"botscenario {c['name']} strategy +new rpg,+grind,-follow,-stay,-start duel,-pvp nc")
        deadline=time.monotonic()+a.seconds;previous=None;next_sample=0
        watchdog=ProgressWatchdog();known_spells=set(tracker.known_spells)
        while time.monotonic()<deadline:
            w.pump(1)
            if (folder/'STOP').exists():event('stop_requested');break
            if time.monotonic()>=next_sample:
                if native_log.stat().st_size<log_offset:log_offset=0
                with native_log.open('rb') as native:
                    native.seek(log_offset);lines=native.read();log_offset=native.tell()
                for line in lines.decode('utf-8',errors='replace').splitlines():
                    if c['name'] in line:event('native_activity',message=line)
                pool.heartbeat(account['id'],a.seconds+180)
                console(f"botscenario {c['name']} save");w.pump(.3);current=state()
                if current!=previous:event('state',actual=current);previous=current
                for delta in watchdog.observe(current,time.monotonic(),in_combat=bool(tracker.unit_flags.get(w.char_guid,0)&0x00080000)):
                    kind=delta.pop('event')
                    if kind=='suspected_stall':delta['actual']=current
                    event(kind,**delta)
                learned=tracker.known_spells-known_spells
                if learned:event('spells_learned',actual=sorted(learned));known_spells.update(learned)
                console(f"botscenario {c['name']} rpgtarget get")
                next_sample=time.monotonic()+15
        w.send_chat('say','.npcbot self');w.pump(.5)
        result['graceful_logout']=w.logout();w.s.close();w=None
        result['final_state']=state();result['initial_state']=before
        result['rewarded_delta']=sorted(set(r[0] for r in result['final_state']['rewarded'])-set(r[0] for r in before['rewarded']))
        result['final_spellbook']=sorted(known_spells)
        result['note']='Milestone acceptance requires independent evidence review; EXECUTED means this bounded episode completed, not full-content certification.'
        w=pool.connect(account);w.char_enum();tracker=ObservedWorld(w);prior=w.auto;w.auto=observe
        assert w.player_login(a.guid);w.pump(2)
        result['reconnect_quest_slots']=dict(tracker.quest_slots)
        result['reconnect_state']=state()
        result['reconnect_spellbook']=sorted(tracker.known_spells)
        assert result['reconnect_state']['rewarded']==result['final_state']['rewarded'],'Rewarded quests changed on reconnect'
        assert result['reconnect_state']['equipment']==result['final_state']['equipment'],'Equipment changed on reconnect'
        assert known_spells<=tracker.known_spells,'Known spells lost on reconnect'
        result['reconnect_verified']=True
        assert w.logout();w.s.close();w=None
        event('finished',actual=result['final_state'])
        result['execution_status']='EXECUTED'
    except Exception as e:
        result['execution_status']='BLOCKED';result['error']=str(e);result['traceback']=__import__('traceback').format_exc();event('failure',error=str(e))
    finally:
        if w:
            try:
                if result['selfbot_enabled']:w.send_chat('say','.npcbot self');w.pump(.3)
                w.logout()
            except Exception as e:
                result['cleanup_error']=str(e)
                result.setdefault('error','Cleanup failed: '+str(e))
            finally:w.s.close()
        pool.release(account,lease);stream.close();notes.close()
        result['binary_sha256']=hashlib.sha256((pool.runtime/'worldserver.exe').read_bytes()).hexdigest()
        if result['binary_sha256']!=result['binary_sha256_started']:
            result['error']='QA binary changed during the episode';result['execution_status']='BLOCKED'
        (folder/'result.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
        pool.registry.execute('INSERT INTO events(run,scenario,step,at,status,expected,observed) VALUES(?,?,?,?,?,?,?)',
            (a.run,'native-adventure','bounded-run',time.time(),'FAIL' if 'error' in result else result['execution_status'],
             'Three consecutive native quests, earned/equipped upgrade and reconnect',str(folder/'result.json')))
        pool.registry.commit()
    print(json.dumps(dict(run_id=a.run,rewarded_delta=result.get('rewarded_delta'),error=result.get('error'),status=result['execution_status'])));return int('error' in result)
if __name__=='__main__':sys.exit(main())
