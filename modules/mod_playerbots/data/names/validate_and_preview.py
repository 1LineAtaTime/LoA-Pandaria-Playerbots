#!/usr/bin/env python3
"""Validate the delivered WoW playerbot name pool and simulate assigning 5k unique names.

Usage:
  python3 validate_and_preview.py
  python3 validate_and_preview.py --simulate 5000 --existing existing_character_names.txt
  python3 validate_and_preview.py --simulate 5000 --preview /path/to/preview.csv

Reads CSV with the standard library only. This does NOT connect to a live DB.
"""
from __future__ import annotations
import argparse, csv, json, math, random, re
from collections import Counter, defaultdict
from pathlib import Path

HERE = Path(__file__).resolve().parent
NAME = re.compile(r'^[A-Z][a-z]{1,11}$')
CLASSES = ['Warrior','Paladin','Hunter','Rogue','Priest','Death Knight','Shaman','Mage','Warlock','Druid','Monk']
RACES = ['Human','Orc','Dwarf','Night Elf','Undead','Tauren','Gnome','Troll','Blood Elf','Draenei','Worgen','Goblin','Pandaren']


def load_existing(path):
    if not path: return set()
    path=Path(path)
    if path.suffix.lower()=='.csv':
        with path.open(newline='',encoding='utf-8-sig') as handle:
            reader=csv.DictReader(handle)
            if not reader.fieldnames or not any(k.casefold()=='name' for k in reader.fieldnames):
                raise SystemExit('Existing character CSV needs a name column.')
            col=next(k for k in reader.fieldnames if k.casefold()=='name')
            return {row[col].strip().casefold() for row in reader if row.get(col) and row[col].strip()}
    return {line.strip().casefold() for line in path.read_text(encoding='utf-8-sig').splitlines() if line.strip()}


def load_pool():
    with (HERE/'playerbot_names.csv').open(newline='',encoding='utf8') as f:
        return list(csv.DictReader(f))


def validate(rows):
    errors=[];names=set()
    for i,r in enumerate(rows,2):
        n=r['name'];low=n.casefold()
        if not NAME.fullmatch(n): errors.append(f'row {i}: bad WoW ASCII format: {n!r}')
        if low in names: errors.append(f'row {i}: duplicate {n!r}')
        names.add(low)
        if r['gender_hint'] not in ('feminine','masculine','neutral'): errors.append(f'row {i}: unknown gender_hint')
        if r['class_hint'] and r['class_hint'] not in CLASSES: errors.append(f'row {i}: unknown class_hint')
        if r['race_hint'] and r['race_hint'] not in RACES: errors.append(f'row {i}: unknown race_hint')
        if r['tier'] not in ('A','B','C'): errors.append(f'row {i}: unknown tier')
        try:
            if int(r['weight']) <= 0: errors.append(f'row {i}: weight must be positive')
        except ValueError:
            errors.append(f'row {i}: invalid weight')
    if len(rows)<5000: errors.append(f'pool too small: {len(rows)} names')
    return errors


def weighted_shuffle(entries,rng):
    """Weighted random order without replacement using exponential race keys."""
    return sorted(entries,key=lambda r:-math.log(max(rng.random(),1e-12)) / int(r['weight']),reverse=True)


def simulate(rows,number,existing,rng):
    pools=defaultdict(list)
    for r in rows:
        if r['name'].casefold() in existing: continue
        if r['class_hint']: key=('class',r['class_hint'])
        elif r['race_hint']: key=('race',r['race_hint'])
        elif r['gender_hint']!='neutral': key=('gender',r['gender_hint'])
        else: key=('general','')
        pools[key].append(r)
    for key in list(pools):
        pools[key]=weighted_shuffle(pools[key],rng)
    def take(key):
        group=pools.get(key)
        if group: return group.pop()
        return None
    output=[]
    for i in range(number):
        gender=rng.choice(('female','male'))
        cls=rng.choice(CLASSES)
        race=rng.choice(RACES)  # Metadata test only; NOT an authoritative race/class validator.
        p=rng.random()
        if p < .16:
            buckets=[('class',cls),('general','')]
        elif p < .20:
            buckets=[('race',race),('general','')]
        elif p < (.46 if gender=='female' else .36):
            buckets=[('gender','feminine' if gender=='female' else 'masculine'),('general','')]
        else:
            buckets=[('general','')]
        # Fail closed if no candidate. Never invent an emergency gibberish name.
        result=None
        for b in buckets+[('gender','feminine'),('gender','masculine')]:
            result=take(b)
            if result: break
        if result is None:
            for b in list(pools):
                result=take(b)
                if result: break
        if result is None:
            raise RuntimeError(f'pool exhausted at bot {i+1}, after filtering {len(existing)} reserved names')
        output.append({'bot_number':i+1,'name':result['name'],'avatar_gender':gender,
                       'class_example':cls,'race_example':race,'name_category':result['category'],
                       'name_gender_hint':result['gender_hint'],'name_class_hint':result['class_hint'],
                       'name_race_hint':result['race_hint']})
    assert len({r['name'].casefold() for r in output})==len(output)
    return output


def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--simulate',type=int,default=5000)
    ap.add_argument('--existing',help='Text file one name per line, or CSV with name header; reserved names are never assigned')
    ap.add_argument('--preview',default=str(HERE/'preview_5000_assignments.csv'))
    ap.add_argument('--seed',type=int,default=20261111)
    args=ap.parse_args()
    rows=load_pool()
    errors=validate(rows)
    if errors:
        print('\n'.join(errors[:40]))
        raise SystemExit(f'{len(errors)} validation errors')
    existing=load_existing(args.existing)
    colliding=[r['name'] for r in rows if r['name'].casefold() in existing]
    output=simulate(rows,args.simulate,existing,random.Random(args.seed))
    p=Path(args.preview)
    with p.open('w',newline='',encoding='utf8') as f:
        writer=csv.DictWriter(f,fieldnames=list(output[0].keys()))
        writer.writeheader();writer.writerows(output)
    out={'validated_pool':len(rows),'existing_name_collisions_excluded':len(colliding),
         'simulated_bots':len(output),'unique_assigned':len({r['name'].casefold() for r in output}),
         'format_violations':len(errors),'assigned_category_counts':dict(Counter(r['name_category'] for r in output)),
         'assigned_gender_hint_counts':dict(Counter(r['name_gender_hint'] for r in output)),
         'name_preview_path':str(p)}
    (HERE/'qa_preview_results.json').write_text(json.dumps(out,indent=2),encoding='utf8')
    print(json.dumps(out,indent=2))

if __name__=='__main__': main()
