"""Read-only-in-world normal Pandaren packet observation after an owned baseline reset."""
import json
import time
import traceback
from pool import Pool,ROOT
from qa_actions import ObservedWorld
from evidence import write_json


def main():
    p=Pool();c=p.ensure(24,10);a=next(a for a in p.state['accounts'] if a['id']==c['account_id']);lease=p.lease(a);w=None
    result=dict(character_guid=c['guid'],errors=[],visible_givers=[]);folder=ROOT/'out/quest-qa/pandaren-visibility';folder.mkdir(exist_ok=True)
    try:
        p.reset(c);w=p.connect(a);w.char_enum();tracker=ObservedWorld(w);old=w.auto
        def observe(op,body):
            old(op,body)
            try:tracker.handle(op,body,time.time())
            except Exception as e:
                result['errors'].append(dict(opcode=hex(op),error=str(e),traceback=traceback.format_exc()))
                if len(result['errors'])<4:(folder/f'packet-{len(result["errors"])}.bin').write_bytes(body)
        w.auto=observe;assert w.player_login(c['guid']);w.pump(4)
        result.update(position=w.pos,visible_givers=tracker.actors(53566),nearby_entries=sorted({(g>>32)&0xfffff for g in tracker.visible if g>>52 in (0xf13,0xf15)}),raw_char_guid=w.char_guid)
        w.logout()
    finally:
        if w:w.s.close()
        p.release(a,lease);write_json(folder/'report.json',result)
    print(json.dumps(result))


if __name__=='__main__':main()
