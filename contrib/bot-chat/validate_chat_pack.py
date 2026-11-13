"""Offline chat-pack checks; no server or credentials required."""
import json,re,collections,hashlib
from pathlib import Path
R=Path(__file__).resolve().parents[2]
rows=json.loads((R/'modules/mod_playerbots/data/chat/ambient_en_10000.json').read_text())
allowed={
'suggest_something':{'zone_name','my_role'},'suggest_trade':{'category'},
'suggest_sell':{'item_formatted_link','cost_gold'},'suggest_instance':{'instance_name','my_role'},
'suggest_quest':{'quest_link'},'suggest_faction':{'faction'},
'broadcast_quest_turned_in':{'quest_link'},'broadcast_looting_item_normal':{'item_link'},
'broadcast_quest_accepted_generic':{'quest_link'},'party_flavor':set()}
assert len(rows)==10000 and len({(r['name'],r['text']) for r in rows})==10000
for r in rows:
 assert r['name'] in allowed and r['text'].isascii() and len(r['text'])<=180
 assert set(re.findall(r'%([a-z_]+)',r['text']))<=allowed[r['name']],r
 assert not any(c in r['text'] for c in ('\n','\r','\x00','|H','|c'))
counts=collections.Counter(r['name'] for r in rows);assert set(counts.values())=={1000}
print(json.dumps(dict(passed=True,variants=len(rows),categories=dict(counts),longest=max(len(r['text']) for r in rows))))
