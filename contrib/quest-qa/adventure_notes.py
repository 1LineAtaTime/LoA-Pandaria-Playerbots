"""Human-readable notes derived only from observed native QA events."""
import re


def describe(event):
    kind=event['event']
    if kind=='level_changed':return f"Level increased: {event['previous']} -> {event['actual']}."
    if kind=='quest_state_changed':
        status={1:'objectives complete; awaiting turn-in',3:'in progress'}.get(event['actual'],str(event['actual']))
        return f"Quest {event['quest']}: {status}."
    if kind=='objective_changed':return f"Objective {event['objective']}: {event.get('previous') or '0'} -> {event['actual']}."
    if kind=='quest_rewarded':return f"Quest {event['quest']} turned in; reward persisted."
    if kind=='equipment_changed':
        slot={4:'chest',6:'legs',7:'feet',9:'hands',15:'main hand'}.get(event['slot'],'slot '+str(event['slot']))
        return f"Equipped item {event['item']} in {slot}; previous={event.get('previous') or 'empty'}."
    if kind=='spells_learned':return 'Newly observed spells: '+', '.join(map(str,event['actual']))+'.'
    if kind=='initial_spellbook':return f"Observed {len(event['actual'])} known spells at login."
    if kind=='suspected_stall':return f"REVIEW: {event['reason']} ({event['seconds_without_progress']} seconds without progress). Evidence retained; no automatic quest grants or resets."
    if kind=='failure':return 'FAILURE: '+event['error']
    if kind=='stop_requested':return 'Stop requested; preparing normal logout and reconnect verification.'
    if kind=='finished':return 'Episode ended; saved quests, gear and observed spells survived normal reconnect.'
    if kind=='chat' and 'player botAI' in event['message']:
        return 'Native Playerbots AI '+('enabled.' if 'Enable' in event['message'] else 'disabled.')
    if kind=='native_activity' and '[QUEST_VENDOR]' in event.get('message',''):
        return 'Native vendor recovery: '+event['message'].split('[QUEST_VENDOR]',1)[1].strip()
    return None


class Notes:
    def __init__(self,path,run,guid):
        self.stream=path.open('x',encoding='utf-8');self.last_state=None
        self.stream.write(f'# Live native QA notes\n\nRun: {run}; character GUID: {guid}.\nObserved facts only; no AI service or token usage.\n\n')
        self.stream.flush()

    def record(self,event):
        message=describe(event)
        if event['event'] in ('initial_state','state'):
            # One position/status note per minute; typed progression events are
            # immediate. Do not pretend position alone establishes AI intent.
            stamp=event['timestamp'][:16]
            if stamp!=self.last_state:
                self.last_state=stamp;c=event['actual']['character'][0]
                message=f"Level {c[0]}, XP {c[1]}, health {c[7]}; map {c[3]}, position ({float(c[4]):.1f}, {float(c[5]):.1f}, {float(c[6]):.1f})."
        if message:
            # Keep one event per line, even if a diagnostic has embedded lines.
            message=re.sub(r'[\r\n]+',' ',message)
            self.stream.write(f"- {event['timestamp']}: {message}\n");self.stream.flush()

    def close(self):self.stream.close()
