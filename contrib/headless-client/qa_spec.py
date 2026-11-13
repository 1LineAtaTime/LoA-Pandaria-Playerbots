"""Versioned gameplay contracts. Unsupported mechanics remain explicitly untested."""
import json
from pathlib import Path
SUPPORTED = {'create', 'creation_rejected', 'accept', 'delivery', 'gossip_item', 'invalid_accept', 'workflow'}

def load_suite(path):
    suite = json.loads(Path(path).read_text(encoding='utf-8'))
    if suite.get('version') != 1 or not isinstance(suite.get('scenarios'), list):
        raise ValueError('Expected version 1 and scenarios array')
    ids = set()
    for case in suite['scenarios']:
        ident = case.get('id')
        if not isinstance(ident,str) or not ident or ident in ids:
            raise ValueError('Scenario IDs must be nonempty and unique')
        ids.add(ident)
        if case.get('mode') not in SUPPORTED:
            if not case.get('reason'): raise ValueError(ident + ': unsupported scenario needs a coverage reason')
            continue
        for key in ('race','class_id'):
            if type(case.get(key)) is not int or not 1 <= case[key] <= 255:
                raise ValueError(ident + ': invalid ' + key)
        if case['mode'] in ('accept','delivery','gossip_item','workflow'):
            if type(case.get('quest')) is not int or not 0 < case['quest'] <= 0xffffffff:
                raise ValueError(ident + ': positive quest required')
        if case['mode'] not in ('accept','delivery','gossip_item','workflow') and case.get('quest') is not None:
            raise ValueError(ident + ': quest only allowed for quest modes')
        if case['mode']=='creation_rejected' and (type(case.get('response')) is not int or not 0 <= case['response'] <= 255):
            raise ValueError(ident + ': expected creation response required')
        if case['mode']=='invalid_accept' and (type(case.get('request_quest')) is not int or not 0 <= case['request_quest'] <= 0xffffffff):
            raise ValueError(ident + ': uint32 request_quest required')
        if case['mode']=='gossip_item':
            for key in ('npc','item'):
                if type(case.get(key)) is not int or case[key]<=0:raise ValueError(ident+': positive '+key+' required')
            if type(case.get('expected_count')) is not int or case['expected_count']<0:raise ValueError(ident+': expected count required')
            if type(case.get('seed_quest',True)) is not bool:raise ValueError(ident+': seed_quest must be boolean')
            if type(case.get('fixture_level',85)) is not int or not 1<=case.get('fixture_level',85)<=90:raise ValueError(ident+': fixture level out of range')
            if not isinstance(case.get('option_text'),str) or not case['option_text']:raise ValueError(ident+': option text required')
        if case['mode']=='delivery' and not case.get('expected_reward'):
            raise ValueError(ident + ': independently specified expected reward required')
        if case['mode']=='workflow':
            from qa_actions import validate_workflow
            validate_workflow(case)
        for key,value in case.get('expected_reward',{}).items():
            if key not in ('xp_delta','money_delta') or type(value) is not int or value < 0:
                raise ValueError(ident + ': invalid reward expectation')
    return suite
