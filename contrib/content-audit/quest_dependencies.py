"""Read-only quest dependency graph. Structural checks never certify gameplay."""
import argparse
import hashlib
import json
import re
import subprocess
from collections import Counter, defaultdict
from datetime import datetime, timezone
from pathlib import Path

from catalogue import ROOT, Table, World, write_csv

OBJECT_TYPES = {0: 'creature', 1: 'item', 2: 'gameobject', 3: 'creature',
                4: 'currency', 5: 'spell', 6: 'faction', 7: 'faction'}


def digest(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True, ensure_ascii=False,
                                    separators=(',', ':')).encode()).hexdigest()


def analyse(quests, addons, objectives, relations, items, stores, spawns, scripts=None, missing_scripts=None):
    """Pure graph builder; input IDs/amounts are integers, names remain strings."""
    edges, findings = [], []
    counts = defaultdict(Counter)
    scripts, missing_scripts = scripts or {}, missing_scripts or set()

    def edge(qid, role, kind, ident, source, amount=0):
        if not ident:
            return
        target = abs(ident) if role == 'previous_quest' else ident
        exists = target in stores[kind] if kind in stores else None
        row = dict(quest=qid, role=role, target_type=kind, target_id=target,
                   signed_value=ident, amount=amount, source=source,
                   target_present=exists, spawn_count=spawns.get(kind, {}).get(target, 0)
                   if kind in ('creature', 'gameobject') else '')
        edges.append(row)
        counts[qid][role] += 1
        if qid not in quests:
            findings.append(dict(quest=qid, issue='orphan_quest_reference', role=role,
                                 target_type=kind, target_id=target, source=source,
                                 status='structural_failure'))
        if exists is False:
            findings.append(dict(quest=qid, issue='missing_target_definition', role=role,
                                 target_type=kind, target_id=target, source=source,
                                 status='structural_failure' if kind in ('quest', 'item', 'creature', 'gameobject')
                                 else 'client_store_review'))
        script = scripts.get(kind, {}).get(target, '')
        if script and script in missing_scripts:
            findings.append(dict(quest=qid, issue='reported_unregistered_native_handler', role=role,
                                 target_type=kind, target_id=target, source='startup_script_registration:' + script,
                                 status='native_binding_review'))

    for qid, q in sorted(quests.items()):
        # These fields come from addon rows in the runtime loader, NOT legacy aliases.
        a = addons.get(qid, {})
        for field, role, kind in [('PrevQuestID', 'previous_quest', 'quest'),
                                  ('NextQuestID', 'next_quest', 'quest'),
                                  ('SourceSpellID', 'accept_spell', 'spell'),
                                  ('RequiredSkillID', 'required_skill', 'skill'),
                                  ('RequiredMinRepFaction', 'minimum_reputation', 'faction'),
                                  ('RequiredMaxRepFaction', 'maximum_reputation', 'faction'),
                                  ('RewardMailTemplateID', 'reward_mail_template', 'mail_template')]:
            edge(qid, role, kind, a.get(field, 0), 'quest_template_addon.' + field)
        for field, role, kind in [('RewardNextQuest', 'reward_next_quest', 'quest'),
                                  ('StartItem', 'provided_item', 'item'),
                                  ('RewardSpell', 'reward_spell', 'spell'),
                                  ('RewardDisplaySpell', 'display_spell', 'spell'),
                                  ('RewardTitle', 'reward_title', 'title'),
                                  ('RewardSkillLineID', 'reward_skill', 'skill')]:
            edge(qid, role, kind, q.get(field, 0), 'quest_template.' + field)
        for prefix, quantity, n, role, kind in [
                ('RewardItem', 'RewardAmount', 4, 'reward_item', 'item'),
                ('RewardChoiceItemID', 'RewardChoiceItemQuantity', 6, 'choice_item', 'item'),
                ('ItemDrop', 'ItemDropQuantity', 4, 'objective_drop', 'item'),
                ('RewardCurrencyID', 'RewardCurrencyQty', 4, 'reward_currency', 'currency'),
                ('RewardFactionID', 'RewardFactionOverride', 5, 'reward_reputation', 'faction')]:
            for slot in range(1, n + 1):
                edge(qid, role, kind, q.get(prefix + str(slot), 0),
                     'quest_template.' + prefix + str(slot), q.get(quantity + str(slot), 0))
    for o in objectives:
        kind = OBJECT_TYPES.get(o['type'])
        if kind:
            edge(o['questId'], 'objective_' + str(o['type']), kind, o['objectId'],
                 'quest_objective:' + str(o['id']), o['amount'])
        else:
            # Money/playerkills/area triggers/pet objectives need dedicated semantics.
            counts[o['questId']]['unmodelled_objective'] += 1
    for table, kind, role, rows in relations:
        for ident, qid in rows:
            edge(qid, role, kind, ident, table)
    for ident, startquest in items:
        if startquest:
            edge(startquest, 'item_starter', 'item', ident, 'item_template.startquest')
    for qid in sorted(set(addons) - set(quests)):
        findings.append(dict(quest=qid, issue='orphan_quest_addon', role='addon',
                             target_type='quest', target_id=qid, source='quest_template_addon',
                             status='structural_failure'))
    summaries = []
    byquest = Counter(f['quest'] for f in findings)
    for qid, q in sorted(quests.items()):
        c = counts[qid]
        summaries.append(dict(quest=qid, title=q['LogTitle'], min_level=q['MinLevel'],
                             quest_level=q['QuestLevel'], quest_sort=q['QuestSortID'],
                             allowable_races=q['AllowableRaces'],
                             allowable_classes=addons.get(qid, {}).get('AllowableClasses', 0),
                             exclusive_group=addons.get(qid, {}).get('ExclusiveGroup', 0),
                             quest_script=addons.get(qid, {}).get('ScriptName', ''),
                             giver_links=c['giver'] + c['item_starter'], turnin_links=c['turnin'],
                             objective_links=sum(c['objective_' + str(i)] for i in OBJECT_TYPES),
                             unmodelled_objectives=c['unmodelled_objective'],
                             dependency_edges=sum(c.values()) - c['unmodelled_objective'],
                             findings=byquest[qid], behavior='untested_by_this_audit'))
    return edges, findings, summaries


def run(a):
    a.output.mkdir(parents=True, exist_ok=True)
    a.artifacts.mkdir(parents=True, exist_ok=True)
    db = World(a.config, a.client)
    inputs = {}

    def rows(table, fields, text=()):
        data = db.q('SELECT ' + ','.join('`' + f + '`' for f in fields) +
                    ' FROM `' + table + '` ORDER BY ' + ','.join('`' + f + '`' for f in fields))
        inputs[table] = dict(records=len(data), sha256=digest(data), fields=fields)
        return [dict(zip(fields, (v if f in text else int(v) for f, v in zip(fields, r)))) for r in data]

    qfields = ['ID', 'LogTitle', 'MinLevel', 'QuestLevel', 'QuestSortID', 'AllowableRaces',
               'RewardNextQuest', 'StartItem', 'RewardSpell', 'RewardDisplaySpell', 'RewardTitle',
               'RewardSkillLineID']
    for prefix, n in [('RewardItem', 4), ('RewardAmount', 4), ('RewardChoiceItemID', 6),
                      ('RewardChoiceItemQuantity', 6), ('ItemDrop', 4), ('ItemDropQuantity', 4),
                      ('RewardCurrencyID', 4), ('RewardCurrencyQty', 4),
                      ('RewardFactionID', 5), ('RewardFactionOverride', 5)]:
        qfields += [prefix + str(i) for i in range(1, n + 1)]
    quests = {r['ID']: r for r in rows('quest_template', qfields, ('LogTitle',))}
    afields = ['ID', 'PrevQuestID', 'NextQuestID', 'ExclusiveGroup', 'SourceSpellID',
               'RequiredSkillID', 'RequiredMinRepFaction', 'RequiredMaxRepFaction',
               'RewardMailTemplateID', 'AllowableClasses', 'ScriptName']
    addons = {r['ID']: r for r in rows('quest_template_addon', afields, ('ScriptName',))}
    objectives = rows('quest_objective', ['questId', 'id', 'type', 'objectId', 'amount'])
    items = rows('item_template', ['entry', 'startquest'])
    stores = {'quest': set(quests), 'item': {r['entry'] for r in items}}
    scripts = {}
    for kind, table, field in [('creature', 'creature_template', 'entry'),
                                ('gameobject', 'gameobject_template', 'entry')]:
        definitions = rows(table, [field, 'ScriptName'], ('ScriptName',))
        stores[kind] = {r[field] for r in definitions}
        scripts[kind] = {r[field]: r['ScriptName'] for r in definitions}
    client_sources = []
    for kind, filename in [('spell', 'Spell.dbc'), ('currency', 'CurrencyTypes.dbc'),
                           ('faction', 'Faction.dbc'), ('skill', 'SkillLine.dbc'),
                           ('title', 'CharTitles.dbc'), ('mail_template', 'MailTemplate.dbc')]:
        t = Table(a.data / filename)
        stores[kind] = set(t.labels())
        client_sources.append(t.meta)
    # LoadDBC adds database records to Spell.dbc; missing on disk alone is not missing at runtime.
    spell_overrides = {r['Id'] for r in rows('spell_dbc', ['Id'])}
    override_only = spell_overrides - stores['spell']
    stores['spell'].update(spell_overrides)
    errors = a.errors.read_text(encoding='utf-8', errors='replace')
    missing_scripts = set(re.findall(r"ScriptName '([^']+)' exists in database, but no core script found", errors))
    spawns = {}
    for kind, table in [('creature', 'creature'), ('gameobject', 'gameobject')]:
        data = db.q('SELECT id,COUNT(*) FROM ' + table + ' GROUP BY id ORDER BY id')
        inputs[table] = dict(sha256=digest(data), grouped_rows=len(data))
        spawns[kind] = {int(i): int(n) for i, n in data}
    relations = []
    for kind in ('creature', 'gameobject'):
        for suffix, role in [('starter', 'giver'), ('ender', 'turnin')]:
            table = kind + '_quest' + suffix
            relations.append((table, kind, role, [(r['id'], r['quest']) for r in rows(table, ['id', 'quest'])]))
    edges, findings, summaries = analyse(quests, addons, objectives, relations,
                                         [(r['entry'], r['startquest']) for r in items], stores, spawns,
                                         scripts, missing_scripts)
    # Capture start/end agreement for quest records, not a claim of a transactional DB snapshot.
    stable = rows('quest_template', qfields, ('LogTitle',))
    if {r['ID']: r for r in stable} != quests:
        raise RuntimeError('Quest definitions changed during audit; rerun after edits finish')
    write_csv(a.artifacts / 'edges.csv', edges, list(edges[0]) if edges else
              ['quest', 'role', 'target_type', 'target_id', 'signed_value', 'amount', 'source', 'target_present', 'spawn_count'])
    write_csv(a.output / 'quests.csv', summaries, list(summaries[0]))
    write_csv(a.output / 'findings.csv', findings, ['quest', 'issue', 'role', 'target_type', 'target_id', 'source', 'status'])
    # Deduplicate shared actor/script failures so one repair can unblock many quests.
    groups = defaultdict(list)
    for finding in findings:
        groups[(finding['status'], finding['target_type'], finding['target_id'], finding['source'])].append(finding)
    queue = []
    for (status, kind, ident, source), group in sorted(groups.items()):
        affected = sorted({f['quest'] for f in group})
        queue.append(dict(status=status, target_type=kind, target_id=ident, source=source,
                          quest_count=len(affected), quests=';'.join(map(str, affected)),
                          roles=';'.join(sorted({f['role'] for f in group})),
                          fixed_spawn_count=spawns.get(kind, {}).get(ident, 0) if kind in ('creature', 'gameobject') else '',
                          disposition='open_requires_source_and_behavior_review'))
    write_csv(a.output / 'repair-queue.csv', queue, ['status', 'target_type', 'target_id', 'source',
              'quest_count', 'quests', 'roles', 'fixed_spawn_count', 'disposition'])
    revision = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip()
    report = dict(version=1, generated_utc=datetime.now(timezone.utc).isoformat(), revision=revision,
                  generator_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
                  reader_sha256=hashlib.sha256((Path(__file__).parent / 'catalogue.py').read_bytes()).hexdigest(),
                  schema=db.schema, read_only=True, gameplay_certified=False,
                  quests=len(quests), objective_rows=len(objectives), edges=len(edges), findings=len(findings),
                  spell_database_only_ids=sorted(override_only),
                  spell_database_only_dependency_links=sum(e['target_type']=='spell' and e['target_id'] in override_only for e in edges),
                  diagnostic_source=dict(file=a.errors.name, sha256=hashlib.sha256(a.errors.read_bytes()).hexdigest(),
                                         reported_unregistered_names=len(missing_scripts),
                                         caveat='Startup log evidence, not an in-process registry snapshot; may include historical records.'),
                  findings_by_status=dict(Counter(f['status'] for f in findings)),
                  findings_by_role=dict(Counter(f['role'] for f in findings)),
                  quests_with_findings=len({f['quest'] for f in findings if f['quest'] in quests}),
                  repair_queue_entries=len(queue),
                  quests_without_explicit_giver=sum(not q['giver_links'] for q in summaries),
                  quests_without_explicit_turnin=sum(not q['turnin_links'] for q in summaries),
                  inputs=inputs, client_sources=client_sources,
                  output_hashes={p.name: hashlib.sha256(p.read_bytes()).hexdigest()
                                 for p in [a.artifacts / 'edges.csv', a.output / 'quests.csv', a.output / 'findings.csv', a.output / 'repair-queue.csv']},
                  limitations=['Sequential SELECTs; quest definitions checked again at end, other tables not transactionally snapshotted.',
                               'No fixed giver/turnin/spawn is a review case, not proof of a broken quest.',
                               'Spell/client-store absence requires server override and version review.',
                               'Conditions, phase/visibility evaluation, scripted summons, item acquisition and native execution remain unmodelled.',
                               'No independent retail expectations or gameplay passes inferred from server definitions.'])
    (a.output / 'summary.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(json.dumps({k: report[k] for k in ['quests', 'objective_rows', 'edges', 'findings', 'findings_by_status', 'findings_by_role', 'quests_with_findings']}))


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--config', type=Path, default=ROOT / 'exports/worldserver.conf')
    p.add_argument('--client', type=Path, default=ROOT / 'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
    p.add_argument('--data', type=Path, default=ROOT / 'exports/dbc')
    p.add_argument('--output', type=Path, default=ROOT / 'doc/quest-dependency-audit-548')
    p.add_argument('--artifacts', type=Path, default=ROOT / 'out/quest-dependency-audit-548')
    p.add_argument('--errors', type=Path, default=ROOT / 'exports/logs/Errors.txt')
    run(p.parse_args())
