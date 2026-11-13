"""Single-reward diagnostic; deliberately excludes duplicate-reward acceptance."""
import json
import math
import time
from pool import Pool,ROOT
from qa_actions import ObservedWorld
from evidence import write_json


def main():
    p=Pool();c=p.ensure(24,10);a=next(a for a in p.state['accounts'] if a['id']==c['account_id']);lease=p.lease(a);w=None;r=dict(execution_profile='diagnostic',acceptance_eligible=False,coinage=[])
    try:
        p.reset(c);w=p.connect(a);w.char_enum();t=ObservedWorld(w);old=w.auto
        def observe(op,b):
            old(op,b)
            try:t.handle(op,b,time.time())
            except Exception:pass
        w.auto=observe;assert w.player_login(c['guid']);w.pump(2);r['coinage'].append(['login',t.coinage])
        g,pos=min(t.actors(53566),key=lambda pair:math.dist(w.pos,pair[1]));w.move_walk([pos]);w.pump(1)
        assert math.dist(w.pos,pos)<6
        w.quest_hello(g);w.pump(1);r['coinage'].append(['hello',t.coinage]);w.quest_complete(30039,g);w.pump(1);r['coinage'].append(['complete_request',t.coinage]);w.quest_reward(30039,0,g);w.pump(2);r['coinage'].append(['one_reward_request',t.coinage]);w.logout()
        r['persisted']=p.db.q('Character',f"SELECT xp,money FROM characters WHERE guid={c['guid']}")
        assert p.db.q('Character',f"SELECT quest FROM character_queststatus_rewarded WHERE guid={c['guid']} AND quest=30039"),'Diagnostic did not actually reward the quest'
    finally:
        if w:w.s.close()
        p.release(a,lease);write_json(ROOT/'out/quest-qa/pandaren-single-reward.json',r)
    print(json.dumps(r))


if __name__=='__main__':main()
