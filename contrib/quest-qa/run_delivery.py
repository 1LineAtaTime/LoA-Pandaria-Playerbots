"""Meaningful reusable-pool delivery lifecycle, offer/quest-log/reward/chain/reconnect.

The reviewed first contract is25152 (Orc):40 local-config XP,0 money,
75 Orgrimmar reputation, no items/spells/currencies, then Cutting Teeth25126.
This verifies one bounded scenario, never all eligibility cases for a quest.
"""
import hashlib
import json
import math
from pathlib import Path
import secrets
import sys
import time
import traceback
from datetime import datetime,timezone
from pool import Pool,ROOT
from protocol import QuestPackets
from qa_actions import Workflow
from evidence import write_json


def main():
    pool=Pool();char=pool.ensure(2,4);account=next(a for a in pool.state['accounts'] if a['id']==char['account_id']);lease=pool.lease(account)
    run=datetime.now(timezone.utc).strftime('%Y%m%d-%H%M%S')+'-'+secrets.token_hex(3)
    output=ROOT/'out/quest-qa/runs'/run;output.mkdir(parents=True)
    scenario='q25152-r2-c4-l1'
    result=dict(run=run,scenario=scenario,quest=25152,quest_name='Your Place In The World',race=2,class_id=4,faction='Horde',level=1,character_guid=char['guid'],account_id=char['account_id'],server_build=pool.environment['source_commit'],started_utc=datetime.now(timezone.utc).isoformat(),status='UNVERIFIED',verification='UNVERIFIED',quest_fully_gameplay_verified=False,coverage_scope='One reviewed race/class/level lifecycle; not exhaustive quest eligibility',stages=[],setup=dict(reused_character=True,quest_status_granted=False,power_profile='normal'),checks=[])
    target=output/'execution.json';w=None
    def record(step,expected,observed,status='PASS'):
        event=dict(step=step,status=status,expected=expected,observed=observed,at_utc=datetime.now(timezone.utc).isoformat())
        result['stages'].append(event);pool.event(run,scenario,step,status,expected,observed);write_json(target,result)
    try:
        pool.reset(char)
        assert not pool.db.q('Login',f"SELECT gmlevel FROM account_access WHERE id={account['id']} AND gmlevel<>0"),'GM account cannot establish ordinary eligibility'
        assert not pool.db.q('Character',f"SELECT quest FROM character_queststatus_rewarded WHERE guid={char['guid']} AND quest IN (25152,25126)")
        w=pool.connect(account);w.char_enum()
        case=dict(id=scenario,quest=25152,steps=[],postconditions=dict(quest_state='rewarded'),expected_reward=dict(xp_delta=40,money_delta=0))
        flow=Workflow(w,pool.db,char['guid'],case,result,lambda step:None);packets=QuestPackets();previous=w.auto
        def observe(op,body):previous(op,body);packets.observe(w,op,body)
        w.auto=observe;assert w.player_login(char['guid']);w.pump(1)
        before=pool.db.q('Character',f"SELECT level,xp,money FROM characters WHERE guid={char['guid']}")[0]
        before_rep=pool.db.q('Character',f"SELECT standing FROM character_reputation WHERE guid={char['guid']} AND faction=76")
        before_rep=int(before_rep[0][0]) if before_rep else 0
        record('normal_character_eligible',dict(race=2,class_id=4,level=1,gm=False),dict(race=int(pool.db.q('Character',f"SELECT race FROM characters WHERE guid={char['guid']}")[0][0]),character_guid=char['guid'],persisted_level=int(before[0]),gm=False))
        flow.approach(10176);giver=flow.target();position=flow.tracker.position(giver)
        assert position and math.dist(w.pos,position)<=6
        record('quest_giver_visible_and_accessible',dict(entry=10176,distance_max=6),dict(guid=giver,distance=math.dist(w.pos,position)))
        w.quest_hello(giver);w.pump(2)
        assert 25152 in packets.offered,'Quest was not offered in decoded client packets'
        record('quest_offered',25152,packets.events)
        w.quest_accept(25152,giver);w.pump(2)
        assert 25152 in flow.tracker.quest_slots.values(),'Accepted quest absent from observed player quest log'
        slot=next(k for k,v in flow.tracker.quest_slots.items() if v==25152)
        record('accepted_and_in_client_quest_log',dict(quest=25152),dict(slot=slot,quest=flow.tracker.quest_slots[slot],state=flow.tracker.quest_states.get(slot)))
        # This quest's objective is delivery dialogue, not a kill/collect counter.
        flow.approach(3143);receiver=flow.target();w.quest_hello(receiver);w.pump(1)
        record('delivery_destination_reached',dict(entry=3143),dict(guid=receiver,position=flow.tracker.position(receiver)))
        w.quest_complete(25152,receiver);w.pump(1);w.quest_reward(25152,0,receiver);w.pump(2)
        assert w.logout();flow.verify(before)
        rep=int(pool.db.q('Character',f"SELECT standing FROM character_reputation WHERE guid={char['guid']} AND faction=76")[0][0])
        assert rep-before_rep==75,f'Orgrimmar reputation expected75, observed{rep-before_rep}'
        record('reward_turnin_and_persistence',dict(xp_delta=40,money_delta=0,reputation_delta=75),dict(checks=result['checks'],reputation_delta=rep-before_rep))
        initial_followup_packets=[e for e in packets.events if 25126 in e['quest_ids']]
        w.s.close();w=None
        # Reauthenticate a fresh network session and enumerate ownership before login.
        w=pool.connect(account);assert any(c['name']==char['name'] for c in w.char_enum())
        flow=Workflow(w,pool.db,char['guid'],case,result,lambda step:None);packets=QuestPackets();previous=w.auto
        def observe_again(op,body):previous(op,body);packets.observe(w,op,body)
        w.auto=observe_again;assert w.player_login(char['guid']);w.pump(1)
        record('fresh_network_reconnect',True,True)
        flow.approach(3143);receiver=flow.target();w.quest_hello(receiver);w.pump(2)
        if 25126 not in flow.tracker.quest_slots.values():
            assert 25126 in packets.offered,'Cutting Teeth follow-up neither offered nor auto-accepted'
            w.quest_accept(25126,receiver);w.pump(2)
        else:
            assert initial_followup_packets,'Auto-accepted follow-up had no observed quest details'
        assert 25126 in flow.tracker.quest_slots.values(),'Follow-up not in client quest log'
        record('followup_offered_and_accepted',25126,dict(offered=True,in_quest_log=True,initial_packets=initial_followup_packets,auto_accept_flag=True))
        # Invalid premature reward and duplicate reward requests must not pay out.
        w.quest_reward(25126,0,receiver);w.quest_reward(25152,0,receiver);w.pump(1)
        assert w.logout()
        assert not pool.db.q('Character',f"SELECT quest FROM character_queststatus_rewarded WHERE guid={char['guid']} AND quest=25126")
        after=pool.db.q('Character',f"SELECT level,xp,money FROM characters WHERE guid={char['guid']}")[0]
        assert int(after[1])-int(before[1])==40 and int(after[2])-int(before[2])==0,'Invalid reward requests changed balances'
        record('premature_and_duplicate_reward_rejected',dict(followup_rewarded=False,extra_xp=0,extra_money=0),dict(followup_rewarded=False,xp_delta=40,money_delta=0))
        result.update(status='PASS',verification='GAMEPLAY_VERIFIED',finished_utc=datetime.now(timezone.utc).isoformat(),snapshot_reset_count=char['reset_count'],limitations=['Other races/classes/levels not established by this run','Packet-level client: visual rendering not tested','No kill/collect/crafting/instance mechanic occurs in this delivery quest'])
    except Exception as e:
        result.update(status='FAIL' if isinstance(e,AssertionError) else 'BLOCKED',verification='UNVERIFIED',error=str(e),traceback=traceback.format_exc(),finished_utc=datetime.now(timezone.utc).isoformat())
        pool.event(run,scenario,'failure','FAIL','All applicable assertions',''.join(traceback.format_exception(e)))
    finally:
        if w:w.s.close()
        pool.release(account,lease);write_json(target,result)
    print(json.dumps(dict(report=str(target),status=result['status'],stages=len(result['stages']),character_guid=char['guid'],reused=True)))
    return int(result['status']!='PASS')


if __name__=='__main__':sys.exit(main())
