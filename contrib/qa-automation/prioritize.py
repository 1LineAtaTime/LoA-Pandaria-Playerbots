"""Rank shared repair groups; unknown availability stays unknown."""
import csv
import json
from pathlib import Path

# Reviewed starter-zone sort IDs from current quest definitions. This chooses work
# order only and does not assert every quest in those zones is currently obtainable.
STARTER_SORTS = {12, 154, 188, 215, 363, 3431, 3526, 4714, 4755, 5736}


def rank(quest_rows, groups, classifications=None):
    quests = {int(q['quest']): q for q in quest_rows}
    classifications = classifications or {}
    for ident, value in classifications.items():
        if not str(ident).isdigit() or not isinstance(value,dict) or value.get('status') not in ('active','retired','test','custom','unknown') or not value.get('reason') or not value.get('source'):
            raise ValueError('Availability classification requires an ID, supported status, source and reason')
    output = []
    for group in groups:
        ids = [int(x) for x in group['quests'].split(';') if x]
        affected = [quests[i] for i in ids if i in quests]
        starter = [int(q['quest']) for q in affected if int(q['quest_sort']) in STARTER_SORTS
                   and 0 <= int(q['min_level']) <= 10 and not q['title'].upper().startswith('FLAG')]
        labels = {classifications.get(str(i), {}).get('status', 'unknown') for i in ids}
        if labels and labels <= {'retired', 'test'}:
            tier, reason = 4, 'Explicitly classified retired/test content'
        elif starter and int(group['fixed_spawn_count'] or 0) > 0:
            tier, reason = 0, 'Spawned shared dependency in a racial starter zone'
        elif group['status'] == 'structural_failure':
            tier, reason = 1, 'Missing definition; establish availability before repair'
        elif int(group['fixed_spawn_count'] or 0) > 0:
            tier, reason = 2, 'Spawned shared dependency outside starter priority'
        else:
            tier, reason = 3, 'No fixed spawn; inspect scripted summons/custom availability'
        output.append(dict(group, priority_tier=tier, priority_reason=reason,
                           starter_quest_ids=starter, availability_labels=sorted(labels)))
    return sorted(output, key=lambda r:(r['priority_tier'], -len(r['starter_quest_ids']), -int(r['quest_count']), r['source']))


def generate(directory, output, classifications=None):
    def read(name):
        with (Path(directory)/name).open(encoding='utf-8-sig', newline='') as stream:
            return list(csv.DictReader(stream))
    result = rank(read('quests.csv'), read('repair-queue.csv'), classifications)
    Path(output).write_text(json.dumps(dict(priority_is_not_gameplay_proof=True, groups=result), indent=2)+'\n', encoding='utf-8')
    return result
