"""Versioned observed quest offer packets; unknown layouts fail, never imply an offer."""
import struct
import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[2]/'contrib/headless-client'))
import headless


def quest_list(body):
    r=headless.MvReader(body);r.raw(8);mask=[0]*8
    mask[2]=r.bit();title_len=r.bits(11);mask[6]=r.bit();mask[0]=r.bit();count=r.bits(19)
    if count>50:raise ValueError('Unexpected quest menu count')
    titles=[]
    for _ in range(count):r.bit();titles.append(r.bits(9))
    for i in (1,3,4,5,7):mask[i]=r.bit()
    r.flush()
    for i in (1,0,6,7):
        if mask[i]:r.raw(1)
    quests=[]
    for length in titles:
        flags=r.u32();qid=r.u32();title=r.raw(length).decode('utf-8');extra=r.raw(12)
        quests.append(dict(id=qid,title=title))
    for i in (5,3,2):
        if mask[i]:r.raw(1)
    r.raw(title_len)
    if mask[4]:r.raw(1)
    if r.p!=len(body):raise ValueError('Quest list packet has undecoded bytes')
    return quests


class QuestPackets:
    def __init__(self):self.offered=set();self.events=[]
    def observe(self,w,op,body):
        ids=[]
        if op==0x134c:
            if len(body)<92:raise ValueError('Short quest details packet')
            ids=[struct.unpack_from('<I',body,88)[0]]
        elif op==0x02d4:ids=[q['id'] for q in quest_list(body)]
        elif op==0x0244:ids=[q['id'] for q in (w.gossip or {}).get('quests',[])]
        if ids:
            self.offered.update(ids);self.events.append(dict(opcode=hex(op),quest_ids=ids,bytes=len(body)))
