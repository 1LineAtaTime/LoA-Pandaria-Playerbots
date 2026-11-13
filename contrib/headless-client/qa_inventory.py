"""Generate an honest quest coverage queue; definitions are candidates, not an independent oracle."""
import json
from collections import defaultdict

def inventory(db,output,results):
    quests={int(r[0]):dict(quest=int(r[0]),status='untested',objectives=[]) for r in db.q('World','SELECT ID FROM quest_template ORDER BY ID')}
    for qid,oid,kind,target,amount in db.q('World','SELECT questId,id,type,objectId,amount FROM quest_objective ORDER BY questId,id'):
        if int(qid) in quests:quests[int(qid)]['objectives'].append(dict(id=int(oid),type=int(kind),target=int(target),amount=int(amount)))
    for r in results:
        qid=r.get('quest')
        if qid not in quests:continue
        # Acceptance-only does not count as a complete quest journey.
        full='quest_accepted_and_rewarded' in r.get('checks',[]) and r['status']=='passed'
        # A rewarded bounded journey does not cover every mechanic, eligibility
        # case, reward type and follow-up. Preserve assertion success separately.
        quests[qid].update(status='partial' if r['status']=='passed' else r['status'],scenario=r['id'],
                          bounded_journey_rewarded=full,assertion_status=r['status'],
                          full_gameplay_verified=False,overall_quest_status='UNVERIFIED')
    (output/'quest-coverage.json').write_text(json.dumps(dict(expectations_source='Current database only; route/objective candidates require independently reviewed outcomes',quests=list(quests.values())),indent=2)+'\n')
    counts=defaultdict(int)
    for q in quests.values():counts[q['status']]+=1
    return dict(total=len(quests),counts=dict(counts),report='quest-coverage.json',spell_and_boss_coverage='not yet enumerated or validated')
