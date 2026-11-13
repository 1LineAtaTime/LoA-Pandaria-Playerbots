"""Observe a normal-role quest14212 vehicle session in the owned isolated QA realm.

Prerequisite history/level/location are fixture setup, not gameplay verification.
The quest under test is accepted through real client packets, never completed by SQL.
"""
import json
import math
import struct
import time
import traceback
from datetime import datetime,timezone
from pool import Pool,ROOT
from protocol import QuestPackets
from qa_actions import ObservedWorld
from evidence import write_json
import headless


def main():
    p=Pool();c=p.ensure(22,1);a=next(a for a in p.state['accounts'] if a['id']==c['account_id']);lease=p.lease(a)
    out=ROOT/'out/quest-qa/sacrifices'/datetime.now(timezone.utc).strftime('%Y%m%d-%H%M%S');out.mkdir(parents=True)
    result=dict(quest=14212,character_guid=c['guid'],source_commit=p.environment['source_commit'],binary_sha256=p.environment.get('qa_binary_sha256'),setup='Controlled level4/prerequisite history/location; normal account, no quest14212 credit granted',packets=[],status='UNVERIFIED');w=None
    try:
        p.reset(c)
        actor=p.db.q('World','SELECT guid,map,position_x,position_y,position_z FROM creature WHERE id=35552 LIMIT 1')[0]
        spawn,mapid,x,y,z=actor
        p.db.q('Character',f"UPDATE characters SET level=4,map={mapid},zone=4755,position_x={x},position_y={y},position_z={z} WHERE guid={c['guid']}")
        p.db.q('Character',f"INSERT INTO character_queststatus_rewarded(guid,quest,active) VALUES"+','.join(f"({c['guid']},{q},1)" for q in (14078,14159,14293,14294)))
        before=p.db.q('Character',f"SELECT level,xp,money FROM characters WHERE guid={c['guid']}")[0]
        result['initial_balances']=before
        w=p.connect(a);w.char_enum();tracker=ObservedWorld(w);offers=QuestPackets();old=w.auto;start=time.time()
        def observe(op,b):
            old(op,b);offers.observe(w,op,b)
            try:tracker.handle(op,b,time.time())
            except (ValueError,IndexError,KeyError,struct.error,EOFError):tracker.decode_errors+=1
            if op==0x1645 and len(b)>=13:
                count,kind,quest,required,entry=struct.unpack_from('<HBIHI',b)
                if quest==14212:
                    event=dict(count=count,required=required,entry=entry,t=round(time.time()-start,3))
                    result.setdefault('objective_credit',[]).append(event);p.event(out.name,'q14212-r22-c1-l4','objective_credit','OBSERVED',dict(required=30),event);write_json(out/'trace.json',result)
            if op in (0x1450,0x06aa,0x0df1):
                result.setdefault('combat_packets',[]).append(dict(t=round(time.time()-start,3),op=hex(op),hex=b.hex()))
            if op in (0x095a,0x1043):
                event=dict(t=round(time.time()-start,3),op=hex(op),hex=b.hex())
                if op==0x095a and len(b)>=49:
                    r=headless.MvReader(b);mask=[0]*8;mask[7]=r.bit();mask[4]=r.bit();hist=r.bits(21);spells=r.bits(22);mask[2]=r.bit();cooldowns=r.bits(20)
                    for i in (5,3,6,0,1):mask[i]=r.bit()
                    r.flush();event.update(history=hist,spell_count=spells,cooldowns=cooldowns,buttons=[r.u32()&0xffffff for _ in range(10)])
                    r.raw(cooldowns*14);guid=[0]*8
                    def byte(i):
                        if mask[i]:guid[i]=r.u8()^1
                    for i in (2,7,0,3):byte(i)
                    r.raw(4)
                    for i in (1,4,6):byte(i)
                    r.raw(4);byte(5);event['vehicle_guid']=int.from_bytes(bytes(guid),'little')
                if op==0x1043:
                    r=headless.MvReader(b);r.bit();r.bit();event['allow_move']=bool(r.bit())
                result['packets'].append(event)
            write_json(out/'trace.json',result) if op in (0x095a,0x1043) else None
        w.auto=observe;assert w.player_login(c['guid']);w.pump(3)
        giver=(0xf13<<52)|(35552<<32)|int(spawn)
        assert giver in tracker.visible,'Quest giver is not visible in normal quest phase'
        w.quest_hello(giver);w.pump(2)
        result['offers']=offers.events
        assert 14212 in offers.offered,'Sacrifices not offered to prepared normal character'
        w.quest_accept(14212,giver);w.pump(8)
        result['quest_log']=tracker.quest_slots
        horses={e['vehicle_guid'] for e in result['packets'] if 'vehicle_guid' in e}
        result['horses']=[dict(guid=g,position=tracker.position(g),health=tracker.health.get(g)) for g in horses]
        assert result['horses'],'Horse was not observed after normal accept'
        assert any(67063 in e.get('buttons',[]) for e in result['packets']),'Throw Torch absent from vehicle actionbar packets'
        # Exercise ground-target casts against observed live, nearby enemies.
        # One-hit/GM completion is not used for this vehicle mechanic.
        deadline=time.time()+90;attempts=[]
        while time.time()<deadline and tracker.health.get(next(iter(horses)),0)>0:
            horsepos=tracker.position(next(iter(horses)))
            stalkers=sorted(tracker.actors(35229,alive=True),key=lambda pair:math.dist(horsepos,pair[1])) if horsepos else []
            if stalkers and math.dist(horsepos,stalkers[0][1])<=40:
                target,pos=stalkers[0];w.cast(67063,dest=pos)
                attempts.append(dict(spell=67063,target_guid=target,destination=pos,distance=math.dist(horsepos,pos)))
            w.pump(2)
            result.setdefault('survival',[]).append(dict(t=round(time.time()-start,3),rider_health=tracker.health.get(w.char_guid),horse_health=tracker.health.get(next(iter(horses))),horse_position=tracker.position(next(iter(horses)))))
            if result.get('objective_credit') and result['objective_credit'][-1]['count']>=30:break
            if attempts and not any(67063 in e.get('buttons',[]) for e in result['packets'][-2:]):
                # Pet-bar removal at ride end, not a license to cast from the ground.
                if result['packets'][-1].get('hex')=='0000000000000000000000000000000000':break
        result['cast_attempts']=attempts
        assert result.get('objective_credit') and result['objective_credit'][-1]['count']==30,'Did not earn30 actual roundup credits'
        # Completing objectives must not fake arrival. Wait for the authored
        # route to release the vehicle, then interact with its actual end NPC.
        deadline=time.time()+100
        while time.time()<deadline:
            if result['packets'][-1].get('hex')=='0000000000000000000000000000000000':break
            w.pump(2);p.heartbeat(a['id'])
            result.setdefault('survival',[]).append(dict(t=round(time.time()-start,3),rider_health=tracker.health.get(w.char_guid),horse_health=tracker.health.get(next(iter(horses))),horse_position=tracker.position(next(iter(horses)))))
        else:raise AssertionError('Vehicle route did not release its rider')
        end=p.db.q('World','SELECT guid FROM creature WHERE id=35618 LIMIT 1')[0][0]
        receiver=(0xf13<<52)|(35618<<32)|int(end)
        w.quest_hello(receiver);w.pump(1);w.quest_complete(14212,receiver);w.pump(1);w.quest_reward(14212,0,receiver);w.pump(2)
        assert w.logout()
        rewarded=p.db.q('Character',f"SELECT quest FROM character_queststatus_rewarded WHERE guid={c['guid']} AND quest=14212")
        assert rewarded,'Normal turn-in did not persist the quest reward'
        after=p.db.q('Character',f"SELECT level,xp,money FROM characters WHERE guid={c['guid']}")[0]
        # Authored75copper x this runtime's reviewed Rate.Drop.Money=2.
        # The earlier75-only assertion was a harness oracle error, not a game defect.
        assert int(after[2])-int(before[2])==150,'Reviewed75copper x configured2 expected150 copper'
        assert int(after[0])>int(before[0]) or int(after[1])>int(before[1]),'No quest XP awarded'
        result['turnin']=dict(entry=35618,reward_persisted=True,money_delta=150,authored_money=75,configured_money_rate=2,before=before,after=after,followup_offered=14218 in offers.offered)
        assert 14218 in offers.offered,'By Blood And Ash follow-up was not observed'
        w.s.close();w=None
        w=p.connect(a);assert any(ch['name']==c['name'] for ch in w.char_enum())
        fresh=ObservedWorld(w);old=w.auto
        def reconnect_observe(op,b):
            old(op,b)
            try:fresh.handle(op,b,time.time())
            except (ValueError,IndexError,KeyError,struct.error,EOFError):fresh.decode_errors+=1
        w.auto=reconnect_observe;assert w.player_login(c['guid']);w.pump(2)
        assert 14212 not in fresh.quest_slots.values(),'Rewarded quest remained active after reconnect'
        flags=fresh.unit_flags.get(w.char_guid)
        assert flags is not None and not flags&2,'Temporary non-attackable rider protection leaked'
        result['reconnect']=dict(rewarded_quest_not_active=True,unit_flags=flags,temporary_protection_cleared=True)
        result['status']='INTEGRATION_VERIFIED'
        result['limitations']=['Client rendering not established','Other class/race/level and negative eligibility cases not established','XP increase observed; exact independent XP oracle not yet reviewed']
    except Exception as e:result.update(status='FAIL' if isinstance(e,AssertionError) else 'BLOCKED',error=str(e),traceback=traceback.format_exc())
    finally:
        if w:
            try:w.logout()
            finally:w.s.close()
        p.release(a,lease);write_json(out/'trace.json',result)
    print(json.dumps(dict(report=str(out/'trace.json'),status=result['status'],error=result.get('error'),packets=len(result['packets']))))


if __name__=='__main__':main()
