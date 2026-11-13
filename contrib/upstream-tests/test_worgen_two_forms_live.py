"""Controlled prior-checkpoint fixture; real well interactions and quest reward.

No objective credit or Two Forms spell is injected. Only owned QA bodies are used.
"""
import argparse, hashlib, json, sys, time
from pathlib import Path
ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT/'contrib/quest-qa'), str(ROOT/'contrib/headless-client'), str(Path(__file__).parent)]
from pool import Pool
from qa_actions import ObservedWorld
from test_gilneas_cage_live import phase_packet, teleport_packet
from test_worgen_consumables_live import single_aura
import headless

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--class-id', type=int, choices=(1,3,4,5,8,9,11), default=4)
    args = parser.parse_args()
    pool = Pool(); c = pool.ensure(22, args.class_id)
    account = next(a for a in pool.state['accounts'] if a['id'] == c['account_id'])
    lease = pool.lease(account); w = None
    result = dict(quest=24593, class_id=args.class_id, execution_profile='controlled-prerequisite-fixture', gm=False,
                  quest_credit_injected=False, reward_spell_injected=False, passed=False, phases=[], wells=[], auras=[])
    try:
        pool.reset(c); guid = c['guid']
        pool.db.q('Character', f'UPDATE characters SET level=12,map=654,zone=4714,position_x=-2068.71,position_y=1277.9,position_z=-85.3105,health=300 WHERE guid={guid}')
        previous = [14078,14159,14293,14221,14222,14375,14367,14368,14369,14386,14396,14395,14397,14402,14405,14463,14465,14466,24628,24646]
        pool.db.q('Character', 'INSERT INTO character_queststatus_rewarded(guid,quest,active) VALUES '+','.join(f'({guid},{q},1)' for q in previous))
        assert not pool.db.q('Character', f'SELECT spell FROM character_spell WHERE guid={guid} AND spell=68996'), 'Fixture already knows Two Forms'
        w = pool.connect(account); w.char_enum(); tracker = ObservedWorld(w); prior = w.auto
        def observe(op, body):
            prior(op, body)
            if op == headless.SMSG_MOVE_TELEPORT: w.pos,w.orientation = teleport_packet(body)
            if op == 0x02A2: result['phases'].append(phase_packet(body))
            if op == 0x72:
                aura=single_aura(body)
                if aura: result['auras'].append(aura)
            try: tracker.handle(op, body, time.time())
            except (ValueError, KeyError, IndexError, EOFError): tracker.decode_errors += 1
        w.auto = observe; assert w.player_login(guid); w.pump(3)
        w.move_goto(w.pos[0]+.1,w.pos[1],w.pos[2]); w.pump(.5)
        giver = (0xF13<<52)|(37195<<32)|219156
        assert giver in tracker.visible, 'Crowley not visible at the later quest checkpoint'
        result['position_before_accept']=w.pos
        w.quest_hello(giver); w.pump(.5); result['gossip']=getattr(w,'gossip',None); w.quest_accept(24593,giver); w.pump(1)
        result['quest_slots']=dict(tracker.quest_slots); result['position_after_accept']=w.pos
        assert 24593 in tracker.quest_slots.values(), 'Neither Human Nor Beast was not accepted'
        rows = pool.db.q('World', 'SELECT guid,id,position_x,position_y,position_z FROM gameobject WHERE id IN(201950,201951,201952) AND map=654 ORDER BY id')
        for spawn,entry,x,y,z in rows:
            w.move_walk([(float(x)+1,float(y),float(z))]); w.pump(1)
            matches=tracker.actors(int(entry),'gameobject')
            result['visible_gameobjects']=[(hex(g),pos) for g,pos in tracker.visible.items() if g>>52==0xF11]
            result.setdefault('visible_wells',[]).append(dict(entry=int(entry),matches=matches,decode_errors=tracker.decode_errors))
            assert len(matches)==1, f'Well {entry} not visible or ambiguous'
            raw=matches[0][0]
            w.gameobject_use(raw); w.pump(6)
            result['wells'].append(int(entry))
        w.move_walk([(-2068.71,1277.9,-85.3105)]); w.pump(1)
        w.quest_complete(24593,giver); w.pump(1)
        w.quest_reward(24593,52942,giver); w.pump(3)
        result['auras_before_two_forms']=list(result['auras'])
        w.cast(68996); w.pump(3)
        assert 97709 in result['auras'][len(result['auras_before_two_forms']):], 'Learned Two Forms did not apply altered form'
        assert w.logout(), 'Logout refused'; w.s.close(); w=None
        deadline=time.monotonic()+10
        while time.monotonic()<deadline:
            rewarded=pool.db.q('Character',f'SELECT quest FROM character_queststatus_rewarded WHERE guid={guid} AND quest=24593')
            known=pool.db.q('Character',f'SELECT spell FROM character_spell WHERE guid={guid} AND spell=68996')
            if rewarded and known: break
            time.sleep(.2)
        result['rewarded']=bool(rewarded); result['two_forms_saved']=bool(known)
        assert rewarded, 'Real well use/reward did not complete the quest'
        assert known, 'Normal quest reward did not teach Two Forms'
        w=pool.connect(account); w.char_enum(); tracker=ObservedWorld(w); prior=w.auto
        w.auto=observe; assert w.player_login(guid); w.pump(2)
        assert 68996 in tracker.known_spells, 'Two Forms absent from spellbook after reconnect'
        assert w.logout(); w.s.close(); w=None
        result['reconnect_spellbook_verified']=True
        result['passed']=True
    except Exception as e:
        result['error']=str(e); result['traceback']=__import__('traceback').format_exc()
    finally:
        if w: w.logout(); w.s.close()
        pool.release(account,lease)
        result['binary_sha256']=hashlib.sha256((pool.runtime/'worldserver.exe').read_bytes()).hexdigest()
        args.out.parent.mkdir(parents=True,exist_ok=True)
        args.out.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(result)); return int(not result['passed'])
if __name__=='__main__': sys.exit(main())
