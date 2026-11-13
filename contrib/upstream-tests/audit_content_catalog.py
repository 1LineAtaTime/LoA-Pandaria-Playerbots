"""Read-only quest/item reference census; does not claim gameplay coverage."""
import argparse
import collections
import json
import os
from pathlib import Path
import re
import struct
import subprocess


def dbc_ids(path):
    data = path.read_bytes()
    magic, count, fields, size, _ = struct.unpack_from("<4s4I", data)
    if magic != b"WDBC" or fields * 4 != size:
        raise ValueError("Unsupported DBC layout: " + path.name)
    return {struct.unpack_from("<I", data, 20 + index * size)[0] for index in range(count)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--config", type=Path, required=True)
    parser.add_argument("--client", type=Path, required=True)
    parser.add_argument("--data", type=Path, required=True)
    parser.add_argument("--report", type=Path, required=True)
    args = parser.parse_args()
    match = re.search(r'^WorldDatabaseInfo\s*=\s*"([^"]+)"',
                      args.config.read_text(encoding="utf-8-sig"), re.M)
    if not match:
        raise ValueError("WorldDatabaseInfo missing")
    host, port, username, password, database = match[1].split(";")
    command = [str(args.client.resolve()), "--no-defaults", "--protocol=TCP",
               "--host=" + host, "--port=" + port, "--user=" + username,
               "--database=" + database, "--batch", "--skip-column-names",
               "--default-character-set=utf8mb4"]

    def select(sql):
        if not re.match(r"^\s*(SELECT|SHOW)\b", sql, re.I) or ";" in sql.rstrip().rstrip(";"):
            raise ValueError("Only one generated read-only query is allowed")
        result = subprocess.run(command, input=sql, text=True, encoding="utf-8",
                                capture_output=True, timeout=120,
                                env=dict(os.environ, MYSQL_PWD=password))
        if result.returncode:
            raise RuntimeError(result.stderr.replace(password, "[REDACTED]")[:1000])
        return [line.split("\t") for line in result.stdout.splitlines()]

    quests = {int(r[0]) for r in select("SELECT ID FROM quest_template;")}
    creatures = {int(r[0]) for r in select("SELECT entry FROM creature_template;")}
    objects = {int(r[0]) for r in select("SELECT entry FROM gameobject_template;")}
    items = select("SELECT entry,spellid_1,spellid_2,spellid_3,spellid_4,spellid_5,RequiredLevel FROM item_template;")
    item_ids = {int(r[0]) for r in items}
    spells = dbc_ids(args.data / "dbc" / "Spell.dbc")
    # This fork augments client data with server-only/custom spells. Omitting
    # these would label legitimate custom definitions as missing.
    spells |= {int(r[0]) for r in select("SELECT Id FROM spell_dbc;")}
    currencies = dbc_ids(args.data / "dbc" / "CurrencyTypes.dbc")
    references = {0: ("creature", creatures), 1: ("item", item_ids),
                  2: ("gameobject", objects), 3: ("talk_creature", creatures),
                  4: ("currency", currencies), 5: ("learn_spell", spells)}
    findings = []
    objectives = select("SELECT questId,id,type,objectId FROM quest_objective;")
    for quest, objective, kind, target in objectives:
        quest, objective, kind, target = map(int, (quest, objective, kind, target))
        if quest not in quests:
            findings.append({"kind": "objective_missing_quest", "quest": quest, "objective": objective})
        if kind in references and target > 0:
            label, ids = references[kind]
            if target not in ids:
                findings.append({"kind": "objective_missing_" + label, "quest": quest,
                                 "objective": objective, "target": target})
    for row in items:
        for slot, spell in enumerate(row[1:6], 1):
            spell = int(spell)
            if spell > 0 and spell not in spells:
                findings.append({"kind": "item_missing_spell", "item": int(row[0]),
                                 "slot": slot, "spell": spell, "required_level": int(row[6]),
                                 "quarantined_level_100_or_above": int(row[6]) >= 100})
    for table, entity_ids in [("creature_queststarter", creatures), ("creature_questender", creatures),
                              ("gameobject_queststarter", objects), ("gameobject_questender", objects)]:
        for entity, quest in select("SELECT id,quest FROM " + table + ";"):
            entity, quest = int(entity), int(quest)
            if entity not in entity_ids or quest not in quests:
                findings.append({"kind": "invalid_quest_relation", "table": table,
                                 "entity": entity, "quest": quest})
    report = {"read_only": True,
              "catalog_totals": {"quests": len(quests), "spells": len(spells), "items": len(item_ids),
                                 "quest_objectives": len(objectives)},
              "checks": ["objective references for types0–5", "item spell references including custom spells",
                         "quest starter/ender references"],
              "behavioral_tests": {"quest_playthroughs": 0, "spell_casts": 0, "item_use_tests": 0},
              "all_gameplay_passed": False,
              "finding_counts": dict(collections.Counter(r["kind"] for r in findings)),
              "findings": findings,
              "coverage_limits": ["No cast/damage/loot/phase/escort/vehicle/client UI validation",
                                  "A reference existing does not establish correct behavior",
                                  "Runtime spell corrections/difficulty selection are not executed here"]}
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({k: v for k, v in report.items() if k != "findings"}), flush=True)


if __name__ == "__main__":
    main()
