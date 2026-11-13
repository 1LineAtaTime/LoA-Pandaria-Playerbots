"""Bounded client actions and explicit persisted postconditions, never GM quest completion."""
import math
import struct
import time
import headless

OPS = {
    'approach': {'entry', 'kind', 'max_distance'}, 'gossip': set(), 'select': {'text'},
    'accept': set(), 'turn_in': {'choice'}, 'use_item': {'item'}, 'use_object': set(),
    'loot': set(), 'cast': {'spell', 'target'}, 'checkpoint_character': set(),
    'kill': {'entry', 'count', 'timeout', 'spell', 'spells'},
    'wait_progress': {'entry', 'count', 'timeout'},
    'escort': {'entry', 'credit_entry', 'count', 'timeout'},
    'wait_visible': {'entry', 'present', 'timeout'},
}

# Original 5.4.8 layout, src/server/game/Entities/Object/Updates/UpdateFields.h.
UNIT_HEALTH=0x008+0x019
UNIT_MAX_HEALTH=0x008+0x01F
UNIT_NPC_FLAGS=0x008+0x04F
UNIT_FLAGS=0x008+0x035


def read_values(reader):
    masks=[reader.u32() for _ in range(reader.u8())];values={}
    for block,mask in enumerate(masks):
        for bit in range(32):
            if mask&(1<<bit):values[block*32+bit]=reader.u32()
    if reader.u8():
        dynamic=reader.u32()
        for bit in range(32):
            if dynamic&(1<<bit):reader.u8();reader.raw(4*headless.mv_pop(reader.u32()))
    return values


def validate_workflow(case):
    steps = case.get('steps')
    if not isinstance(steps, list) or not 1 <= len(steps) <= 40:
        raise ValueError('Workflow requires 1..40 steps')
    for s in steps:
        if not isinstance(s, dict) or s.get('op') not in OPS or set(s)-({'op'} | OPS[s['op']]):
            raise ValueError('Unknown action or action field')
        op = s['op']
        required = {'approach': ('entry',), 'select': ('text',), 'use_item': ('item',),
                    'cast': ('spell',), 'kill': ('entry','count'), 'wait_progress': ('entry','count'),
                    'escort': ('entry','credit_entry','count'), 'wait_visible': ('entry','present')}.get(op, ())
        if any(k not in s for k in required):
            raise ValueError(op + ': missing required fields')
        for k in ('entry','item','spell','credit_entry','count'):
            if k in s and (type(s[k]) is not int or not 1 <= s[k] <= 0xffffffff):
                raise ValueError(op + ': positive uint32 ' + k + ' required')
        for k, low, high in [('timeout',1,600 if op=='kill' else 180), ('max_distance',1,150)]:
            if k in s and (type(s[k]) not in (int,float) or not math.isfinite(s[k]) or not low <= s[k] <= high):
                raise ValueError(op + ': invalid ' + k)
        if op == 'approach' and s.get('kind','creature') not in ('creature','gameobject'):
            raise ValueError('Invalid actor kind')
        if op == 'cast' and s.get('target','selected') not in ('self','selected'):
            raise ValueError('Invalid cast target')
        if op == 'wait_visible' and type(s['present']) is not bool:
            raise ValueError('Visibility expectation must be boolean')
        if op == 'select' and (not isinstance(s['text'],str) or not s['text'].strip()):
            raise ValueError('Gossip selection requires text')
        if op == 'turn_in' and (type(s.get('choice',0)) is not int or not 0 <= s.get('choice',0) <= 5):
            raise ValueError('Invalid reward choice')
    post = case.get('postconditions')
    if not isinstance(post,dict) or not post or set(post)-{'quest_state','items'}:
        raise ValueError('Explicit supported postconditions are required')
    if 'quest_state' in post and post['quest_state'] not in ('active','complete','rewarded'):
        raise ValueError('Invalid quest-state expectation')
    if 'items' in post:
        if not isinstance(post['items'],list) or not post['items']:
            raise ValueError('Item expectations must be nonempty')
        for item in post['items']:
            if not isinstance(item,dict) or set(item)!={'item','count'} or type(item['item']) is not int or not 1 <= item['item'] <= 0xffffffff or type(item['count']) is not int or not 0 <= item['count'] <= 1000000:
                raise ValueError('Invalid item-count expectation')
    if any(s['op']=='turn_in' for s in steps) and (post.get('quest_state') != 'rewarded' or not case.get('expected_reward')):
        raise ValueError('Turn-in requires rewarded state and independent reward expectations')


class ObservedWorld(headless.MoveLog):
    """Reuse movement decoding; add static actors and explicit create/out-of-range tracking."""
    def __init__(self, client):
        super().__init__(client, 'units'); self.visible = {}; self.ever_seen = set(); self.decode_errors = 0
        self.health={};self.max_health={};self.npc_flags={};self.unit_flags={};self.quest_slots={};self.quest_states={};self.client=client
        self.known_spells=set();self.coinage_parts={};self.coinage=None;self.object_values={}

    def observe_player_values(self,guid,values):
        if guid!=getattr(self.client,'char_guid',None):return
        self.coinage_parts.setdefault(0x47d,0);self.coinage_parts.setdefault(0x47e,0)
        for field,value in values.items():
            if field in (0x47d,0x47e):self.coinage_parts[field]=value
        if 0x47d in self.coinage_parts:self.coinage=self.coinage_parts[0x47d]+(self.coinage_parts.get(0x47e,0)<<32)
        # UpdateFields.h18414: UNIT_END=0xa0, QUEST_LOG=UNIT_END+0xb,
        # 50 slots,15 uint32 fields per slot. ID/state are offsets0/1.
        base=0xab
        for field,value in values.items():
            relative=field-base
            if 0<=relative<750:
                slot,offset=divmod(relative,15)
                if offset==0:self.quest_slots[slot]=value
                elif offset==1:self.quest_states[slot]=value

    def emit(self, *args):
        pass

    def emit_transport(self, *args):
        pass

    def _update_object(self, body, t):
        r = headless.MvReader(body); r.raw(2)
        for _ in range(r.u32()):
            typ = r.u8()
            if typ == 3:
                for _ in range(r.u32()):
                    guid = headless.mv_read_packed_guid(r)
                    self.visible.pop(guid,None); self.spl.pop(guid,None); self.stat.pop(guid,None);self.object_values.pop(guid,None)
                    self.health.pop(guid,None);self.npc_flags.pop(guid,None);self.unit_flags.pop(guid,None)
                    self.max_health.pop(guid,None)
            elif typ in (1,2):
                guid = headless.mv_read_packed_guid(r); object_type=r.u8()
                mv = headless.mv_read_create_movement(r); values=read_values(r)
                self.object_values[guid]=dict(values)
                self.observe_player_values(guid,values)
                if object_type in (3,4):
                    self.health[guid]=values.get(UNIT_HEALTH,0)
                    self.max_health[guid]=values.get(UNIT_MAX_HEALTH,0)
                    self.npc_flags[guid]=values.get(UNIT_NPC_FLAGS,0)
                    self.unit_flags[guid]=values.get(UNIT_FLAGS,0)
                if mv['pos'] is not None:
                    self.visible[guid] = mv['pos']
                    self.ever_seen.add(guid)
                if mv['living'] and mv['spline'] and mv.get('pts') and mv.get('dur',0)>0:
                    self.spl[guid] = headless._MvSpline(t-mv.get('passed',0)/1000.,mv['pts'],mv['dur'])
            elif typ == 0:
                guid=headless.mv_read_packed_guid(r);values=read_values(r)
                self.object_values.setdefault(guid,{}).update(values)
                self.observe_player_values(guid,values)
                if UNIT_HEALTH in values:self.health[guid]=values[UNIT_HEALTH]
                if UNIT_MAX_HEALTH in values:self.max_health[guid]=values[UNIT_MAX_HEALTH]
                if UNIT_NPC_FLAGS in values:self.npc_flags[guid]=values[UNIT_NPC_FLAGS]
                if UNIT_FLAGS in values:self.unit_flags[guid]=values[UNIT_FLAGS]
            else:
                raise ValueError('Unknown update block')

    def handle(self,op,body,t):
        if op == 0x14c2:  # SMSG_DESTROY_OBJECT, Object::DestroyForPlayer
            r=headless.MvReader(body);mask=[False]*8
            for i in (3,2,4,1):mask[i]=r.bit()
            r.bit()  # death-animation flag
            for i in (7,0,6,5):mask[i]=r.bit()
            r.flush();parts=[0]*8
            for i in (0,4,7,2,6,3,1,5):
                if mask[i]:parts[i]=r.u8()^1
            guid=int.from_bytes(bytes(parts),'little')
            for cache in (self.visible,self.spl,self.stat,self.object_values,self.health,self.max_health,self.npc_flags,self.unit_flags):
                cache.pop(guid,None)
            return
        if op in (0x045a,0x129a):
            r=headless.MvReader(body)
            if op==0x045a:r.bit();count=r.bits(22)
            else:count=r.bits(22);r.bit()
            r.flush()
            if count>10000 or len(body)-r.p<count*4:raise ValueError('Short spellbook packet')
            self.known_spells.update(r.u32() for _ in range(count))
        return super().handle(op,body,t)

    def position(self, guid):
        return self._model_pos(guid,time.time()) or self.visible.get(guid)

    def actors(self, entry, kind='creature', alive=False):
        highs=(0xF13,0xF15) if kind=='creature' else (0xF11,)
        return [(g,self.position(g)) for g in self.visible if g>>52 in highs and (g>>32)&0xfffff == entry and self.position(g) and (not alive or self.health.get(g,0)>0)]


class Workflow:
    def __init__(self, w, db, guid, case, result, mark):
        self.w,self.db,self.guid,self.case,self.result,self.mark = w,db,guid,case,result,mark
        self.tracker = ObservedWorld(w); self.progress={}; self.selected=None; self.completed=False; self.menu_events=[]
        previous = w.auto
        def auto(op,body):
            previous(op,body)
            if op in (0x0244,0x134C,0x02D4,0x074F,0x0277):self.menu_events.append(op)
            try:
                self.tracker.handle(op,body,time.time())
            except (ValueError,IndexError,struct.error,EOFError):
                self.tracker.decode_errors += 1
            if op == 0x1645 and len(body)>=13:
                count,kind,qid,required,entry=struct.unpack_from('<HBIHI',body)
                if qid==case['quest']:
                    self.progress[entry]=count
                    self.result.setdefault('objective_credit',[]).append(dict(entry=entry,count=count,required=required))
        w.auto=auto

    def approach(self, entry, kind='creature', limit=100):
        # Packet-observed positions win for roaming actors. Static route discovery
        # never implies that a phase-hidden actor is actually visible.
        candidates=self.tracker.actors(entry,kind)
        if not candidates:
            if kind=='creature':
                rows=self.db.q('World',f'SELECT c.guid,c.position_x,c.position_y,c.position_z,t.VehicleId FROM creature c JOIN creature_template t ON t.entry=c.id WHERE c.id={entry} AND c.map={int(self.w.map_id)}')
                candidates=[(((0xF15 if int(r[4]) else 0xF13)<<52)|(entry<<32)|int(r[0]),tuple(map(float,r[1:4]))) for r in rows]
            else:
                rows=self.db.q('World',f'SELECT guid,position_x,position_y,position_z FROM gameobject WHERE id={entry} AND map={int(self.w.map_id)}')
                candidates=[((0xF11<<52)|(entry<<32)|int(r[0]),tuple(map(float,r[1:]))) for r in rows]
        if not candidates:
            raise RuntimeError('No actor route or observed summon for '+str(entry))
        guid,pos=min(candidates,key=lambda r:math.dist(self.w.pos,r[1]))
        if math.dist(self.w.pos,pos)>limit:
            raise TimeoutError('Actor outside bounded route')
        self.w.move_walk([pos]);self.w.pump(.5)
        # Fresh movement observers attach before login in the campaign integration.
        if guid not in self.tracker.visible:
            raise TimeoutError('Actor not observed in client phase: '+str(entry))
        self.selected=guid
        return guid

    def execute(self):
        for index,s in enumerate(self.case['steps']):
            op=s['op'];self.mark(f'action_{index:02d}_{op}')
            event=dict(index=index,op=op,status='sent')
            self.result.setdefault('actions',[]).append(event)
            if op=='approach':self.approach(s['entry'],s.get('kind','creature'),s.get('max_distance',100))
            elif op=='gossip':
                self.w.gossip=None;self.menu_events.clear();target=self.target()
                if target>>52==0xF11:self.w.gameobject_use(target)
                else:
                    entry=(target>>32)&0xfffff
                    flags=self.tracker.npc_flags.get(target)
                    if flags is None:
                        rows=self.db.q('World',f'SELECT npcflag FROM creature_template WHERE entry={entry}')
                        assert rows,'No actor interaction flags';flags=int(rows[0][0])
                    if flags&1:self.w.gossip_hello(target)
                    else:self.w.quest_hello(target)
                deadline=time.monotonic()+3
                while not self.menu_events and time.monotonic()<deadline:self.w.pump(.25)
                # A single quest can open details/reward/request-items directly;
                # SMSG_GOSSIP_MESSAGE is not the only legitimate interaction response.
                assert self.menu_events,'No gossip/quest response'
                event['response_opcodes']=[hex(x) for x in self.menu_events]
            elif op=='select':
                options=[o for o in (self.w.gossip or {}).get('options',[]) if s['text'].casefold() in o['text'].casefold()]
                assert len(options)==1,'Gossip option absent or ambiguous'
                self.w.gossip_select(options[0]['id']);self.w.pump(1)
            elif op=='accept':self.w.quest_accept(self.case['quest'],self.target());self.w.pump(1)
            elif op=='turn_in':
                self.w.quest_complete(self.case['quest'],self.target());self.w.pump(1)
                self.w.quest_reward(self.case['quest'],s.get('choice',0),self.target());self.w.pump(1)
                self.completed=True
            elif op=='use_object':self.w.gameobject_use(self.target());self.w.pump(1)
            elif op=='use_item':
                rows=self.db.q('Character',f'SELECT ci.bag,ci.slot,i.guid FROM character_inventory ci JOIN item_instance i ON i.guid=ci.item WHERE ci.guid={self.guid} AND i.itemEntry={s["item"]} ORDER BY ci.bag,ci.slot')
                assert rows,'No persisted owned item; checkpoint_character after receiving new items'
                bag,slot,item_guid=map(int,rows[0]);bag_slot=255
                if bag:
                    slots=self.db.q('Character',f'SELECT slot FROM character_inventory WHERE guid={self.guid} AND item={bag} AND bag=0')
                    assert len(slots)==1,'Bag position unavailable';bag_slot=int(slots[0][0])
                # HandleUseItemOpcode verifies the full typed item GUID against
                # the bag slot; a zero GUID is rejected as ITEM_NOT_FOUND.
                self.w.use_item(bag_slot,slot,(0x400<<52)|item_guid);self.w.pump(1)
            elif op=='loot':
                self.w.loot_open(self.target());self.w.pump(1)
                assert self.w.loot_windows,'No decoded loot response'
                self.w.loot_all();self.w.pump(1)
            elif op=='cast':
                self.w.cast(s['spell'],self.w.char_guid if s.get('target')=='self' else self.target());self.w.pump(1)
            elif op=='checkpoint_character':
                assert self.w.logout(),'Checkpoint logout failed'
                self.tracker.visible.clear();self.tracker.health.clear();self.tracker.npc_flags.clear()
                self.tracker.spl.clear();self.tracker.stat.clear();self.selected=None
                assert self.w.player_login(self.guid),'Checkpoint re-entry failed';self.w.pump(1)
            elif op=='kill':self.kill(s)
            elif op in ('wait_progress','escort','wait_visible'):self.wait(s)
            # Sending a packet is not a standalone mechanics success. Final postconditions
            # and objective observations determine the workflow result.
            event['status']='action_finished'
        self.result['visibility_decode_errors']=self.tracker.decode_errors

    def target(self):
        if self.selected is None:raise ValueError('Action needs a selected actor')
        return self.selected

    def kill(self,s):
        deadline=time.monotonic()+s.get('timeout',90);retry_after={}
        try:
            while time.monotonic()<deadline and self.progress.get(s['entry'],0)<s['count']:
                self.w.pump(.3)
                own=self.w.char_guid;health=self.tracker.health.get(own);maximum=self.tracker.max_health.get(own)
                if health is not None and health<=0:raise AssertionError('Normal character died during combat')
                if health is not None and maximum and health<maximum*.95 and not self.tracker.unit_flags.get(own,0)&0x80000:
                    self.w.attack_stop();start_health=health
                    while time.monotonic()<deadline and self.tracker.health.get(own,0)<maximum*.95:
                        if self.tracker.health.get(own,0)<=0:raise AssertionError('Normal character died during recovery')
                        self.w.pump(1)
                        if self.tracker.unit_flags.get(own,0)&0x80000:break
                    self.result.setdefault('natural_recovery',[]).append(dict(before=start_health,after=self.tracker.health.get(own),method='ordinary_out_of_combat_regeneration',interrupted_by_combat=bool(self.tracker.unit_flags.get(own,0)&0x80000)))
                candidates=[(g,p) for g,p in self.tracker.actors(s['entry'],alive=True) if retry_after.get(g,0)<=time.monotonic() and math.dist(self.w.pos,p)<100]
                if not candidates:
                    self.w.pump(1);continue
                target,pos=min(candidates,key=lambda r:math.dist(self.w.pos,r[1]))
                old=self.progress.get(s['entry'],0);fight_end=min(deadline,time.monotonic()+45)
                while time.monotonic()<fight_end and self.progress.get(s['entry'],0)==old:
                    pos=self.tracker.position(target)
                    if pos is None:break
                    if math.dist(self.w.pos,pos)>100:break
                    # Follow a roaming target in short observed steps. Walking an
                    # entire stale target leg could consume the remaining combat
                    # budget before the loop got a chance to check its deadline.
                    distance=math.dist(self.w.pos,pos)
                    if distance>3:
                        fraction=min(1.0,3.5/distance)
                        step=tuple(a+(b-a)*fraction for a,b in zip(self.w.pos,pos))
                        self.w.move_walk([step],heartbeat_ms=250)
                    self.w.move_face(math.atan2(pos[1]-self.w.pos[1],pos[0]-self.w.pos[0]))
                    self.w.attack(target)
                    if s.get('spell'):self.w.cast(s['spell'],target)
                    for spell in s.get('spells',[]):
                        if spell in self.tracker.known_spells:self.w.cast(spell,target)
                    self.w.pump(.5)
                self.w.attack_stop();self.selected=target
                if self.progress.get(s['entry'],0)>old:
                    # Let the ordinary five-second combat timeout expire before
                    # deciding whether to recover; otherwise every next pull
                    # chains combat and starves a level1 character's regeneration.
                    self.w.pump(5.5)
                # A timed-out target or a respawn with the same GUID is not
                # permanently excluded. Live health still gates every retry.
                retry_after[target]=time.monotonic()+5
                self.result.setdefault('combat_attempts',[]).append(dict(target=target,credit_before=old,credit_after=self.progress.get(s['entry'],0),remaining_health=self.tracker.health.get(target)))
                self.mark('combat_credit_'+str(self.progress.get(s['entry'],0))+'_of_'+str(s['count']))
            assert self.progress.get(s['entry'],0)>=s['count'],'No required server objective credit within combat budget'
        finally:self.w.attack_stop()

    def wait(self,s):
        deadline=time.monotonic()+s.get('timeout',30)
        entry=s.get('credit_entry',s['entry'])
        while time.monotonic()<deadline:
            actors=self.tracker.actors(s['entry'])
            if s['op']=='wait_visible':
                if bool(actors)==s['present']:
                    if not s['present'] and not any((g>>32)&0xfffff==s['entry'] for g in self.tracker.ever_seen):
                        raise TimeoutError('Actor was never seen; cannot certify disappearance')
                    if not s['present'] and self.tracker.decode_errors:
                        raise TimeoutError('Visibility decoder incomplete; cannot certify absence')
                    return
            elif self.progress.get(entry,0)>=s['count']:return
            elif s['op']=='escort' and actors:
                _,pos=min(actors,key=lambda r:math.dist(self.w.pos,r[1]))
                distance=math.dist(self.w.pos,pos)
                if 5<distance<40:self.w.move_walk([pos])
            self.w.pump(.5)
        raise TimeoutError('Observable postcondition not reached: '+s['op'])

    def verify(self,before):
        original=list(self.result['checks'])
        for attempt in range(10):
            try:
                self._verify_once(before)
                return
            except AssertionError:
                self.result['checks'][:]=original
                if attempt==9:raise
                time.sleep(.5)

    def _verify_once(self,before):
        qid=self.case['quest'];post=self.case['postconditions']
        if post.get('quest_state')=='rewarded':
            assert self.db.q('Character',f'SELECT quest FROM character_queststatus_rewarded WHERE guid={self.guid} AND quest={qid}'),'Reward not persisted'
            after=self.db.q('Character',f'SELECT level,xp,money FROM characters WHERE guid={self.guid}')[0]
            for k,v in self.case['expected_reward'].items():
                col=1 if k=='xp_delta' else 2
                assert int(after[col])-int(before[col])==v,'Reward mismatch: '+k
            self.result['checks'].append('quest_accepted_and_rewarded')
        elif post.get('quest_state'):
            state=self.db.q('Character',f'SELECT status FROM character_queststatus WHERE guid={self.guid} AND quest={qid}')
            assert state and int(state[0][0]) in ((1,3) if post['quest_state']=='active' else (1,)),'Unexpected persisted quest state'
        for item in post.get('items',[]):
            count=int(self.db.q('Character',f'SELECT COALESCE(SUM(count),0) FROM item_instance WHERE owner_guid={self.guid} AND itemEntry={item["item"]}')[0][0])
            assert count==item['count'],'Persisted item count mismatch'
        self.result['checks'].append('workflow_declared_postconditions')
