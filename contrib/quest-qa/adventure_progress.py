"""Read-only progression deltas and conservative native-adventure stall signals.

Signals are triage evidence, never proof of a content defect. No resets, retries,
quest grants or gameplay actions are performed here.
"""
import math


def deltas(before, after):
    events = []
    old_character, character = before['character'][0], after['character'][0]
    if old_character[0] != character[0]:
        events.append(dict(event='level_changed', previous=int(old_character[0]), actual=int(character[0])))
    old_quests = dict(before['quests'])
    for quest, status in after['quests']:
        if old_quests.get(quest) != status:
            events.append(dict(event='quest_state_changed', quest=int(quest), previous=old_quests.get(quest), actual=int(status)))
    old_objectives = dict(before['objectives'])
    for objective, amount in after['objectives']:
        if old_objectives.get(objective) != amount:
            events.append(dict(event='objective_changed', objective=int(objective), previous=old_objectives.get(objective), actual=int(amount)))
    old_rewards = {row[0] for row in before['rewarded']}
    for row in after['rewarded']:
        if row[0] not in old_rewards:
            events.append(dict(event='quest_rewarded', quest=int(row[0])))
    old_equipment = {row[0]: row[1:] for row in before['equipment']}
    for slot, item, guid in after['equipment']:
        if old_equipment.get(slot) != [item, guid]:
            events.append(dict(event='equipment_changed', slot=int(slot), previous=old_equipment.get(slot), item=int(item), item_guid=int(guid)))
    return events


class ProgressWatchdog:
    def __init__(self, stationary_seconds=180, active_seconds=600):
        self.stationary_seconds = stationary_seconds
        self.active_seconds = active_seconds
        self.last = None
        self.last_progress = None
        self.last_activity = None
        self.reported = set()

    def observe(self, state, now, in_combat=False):
        if self.last is None:
            self.last = state
            self.last_progress = self.last_activity = now
            return []
        changes = deltas(self.last, state)
        old, current = self.last['character'][0], state['character'][0]
        # Level/XP, quests and gear are progression. Money alone is not: repeated
        # vendor activity must not indefinitely hide a stalled quest.
        progress = bool(changes) or old[:2] != current[:2]
        moved = old[3] != current[3] or math.dist([float(v) for v in old[4:7]], [float(v) for v in current[4:7]]) > 1
        if progress:
            self.last_progress = now
            self.last_activity = now
            self.reported.clear()
        if moved or in_combat or old[7] != current[7]:
            self.last_activity = now
        self.last = state
        if progress:
            return changes
        # A saved zero-health state needs recovery diagnosis, not a claim that a
        # quest is broken. Suppress ordinary travel/interaction stall labels.
        if int(current[7]) == 0:
            label = 'recovery_needed' if now - self.last_activity >= self.stationary_seconds else None
        elif now - self.last_activity >= self.stationary_seconds:
            label = 'stationary_without_progress'
        elif now - self.last_progress >= self.active_seconds:
            label = 'active_without_progress'
        else:
            label = None
        if label and label not in self.reported:
            self.reported.add(label)
            changes.append(dict(event='suspected_stall', classification='unknown_or_insufficient_evidence',
                                reason=label, seconds_without_progress=round(now-self.last_progress),
                                intervention='none; retain state and diagnose existing native activity'))
        return changes
