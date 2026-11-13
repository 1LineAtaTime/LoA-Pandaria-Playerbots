"""Owned normal-security checkpoint: quest29901 gossip summons and credits Anduin.

This checks the repaired binding, not the complete cinematic or quest chain.
"""
import argparse, hashlib, json, sys, time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
sys.path[:0]=[str(ROOT/'contrib/quest-qa'),str(ROOT/'contrib/headless-client')]
from pool import Pool
from qa_actions import ObservedWorld
import headless

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--out',type=Path,required=True);args=parser.parse_args()
    pool=Pool();c=pool.ensure(22,4);account=next(a for a in pool.state['accounts'] if a['id']==c['account_id']);lease=pool.lease(account);w=None
    result=dict(quest=29901,profile='controlled-prerequisite-fixture',gm=False,credit_injected=False,passed=False)
    try:
        pool.reset(c);guid=c['guid']
        pool.db.q('Character',f'UPDATE characters SET level=85,map=870,zone=5785,position_x=-36.775,position_y=-3149.96,position_z=86.1625,health=300 WHERE guid={guid}')
        pool.db.q('Character',f'INSERT INTO character_queststatus_rewarded(guid,quest,active) VALUES({guid},29900,1)')
        w=pool.connect(account);w.char_enum();tracker=ObservedWorld(w);prior=w.auto
        def observe(op,body):
            prior(op,body)
            try:tracker.handle(op,body,time.time())
            except (ValueError,KeyError,IndexError,EOFError):tracker.decode_errors+=1
        w.auto=observe;assert w.player_login(guid);w.pump(3)
        w.move_goto(w.pos[0]+.1,w.pos[1],w.pos[2]);w.pump(.5)
        giver=(0xF13<<52)|(56434<<32)|552167
        assert giver in tracker.visible,'Upper-floor Anduin not visible'
        w.quest_hello(giver);w.pump(.5);w.quest_accept(29901,giver);w.pump(1)
        assert 29901 in tracker.quest_slots.values(),'Quest not accepted'
        w.gossip_hello(giver);w.pump(1)
        options=[o for o in getattr(w,'gossip',{}).get('options',[]) if 'time to go home' in o['text'].lower()]
        result['gossip_options']=options;assert len(options)==1,'Native quest gossip option missing'
        w.gossip_select(options[0]['id']);w.pump(3)
        result['summoned_anduin_visible']=bool(tracker.actors(56485))
        assert result['summoned_anduin_visible'],'Native gossip did not summon Anduin'
        assert w.logout(),'Logout refused';w.s.close();w=None
        deadline=time.monotonic()+10
        while True:
            rows=pool.db.q('Character',f'SELECT amount FROM character_queststatus_objective WHERE guid={guid} AND objectiveId=253525')
            if rows or time.monotonic()>deadline:break
            time.sleep(.1)
        result['saved_objective']=rows;assert rows==[['1']],'Speak to Anduin credit not saved'
        result['passed']=True
    except Exception as e:result['error']=str(e);result['traceback']=__import__('traceback').format_exc()
    finally:
        if w:w.logout();w.s.close()
        pool.release(account,lease)
        result['binary_sha256']=hashlib.sha256((pool.runtime/'worldserver.exe').read_bytes()).hexdigest()
        args.out.parent.mkdir(parents=True,exist_ok=True);args.out.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(result));return int(not result['passed'])
if __name__=='__main__':sys.exit(main())
