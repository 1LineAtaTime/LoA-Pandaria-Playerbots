"""Assemble pinned SF27 quest/text/loot/travel changes against the active LoA schema."""
import argparse,pathlib,json,csv,hashlib,re,collections
from sql_batch import statements,comma_values
from assemble_release26_quest_batch import MAP,split_where
FIELDS=dict(MAP,RequiredRaces=('quest_template','AllowableRaces'),RequiredClasses=('quest_template_addon','AllowableClasses'))
FULL={'2026_08_20_world_05.sql','2026_08_26_world_04.sql','2026_08_26_world_06.sql','2026_10_04_world_01.sql','2026_10_04_world_03.sql','2026_10_04_world_00.sql','2026_09_21_world_00.sql','2026_09_21_world_10.sql'}
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--patches',type=pathlib.Path,required=True);a=ap.parse_args();root=pathlib.Path(__file__).resolve().parents[2]
 inventory=list(csv.DictReader((root/'doc/skyfire27-integration-inventory-2026-11-06.csv').open(encoding='utf-8')));out=['-- SF27 quest text, progression, race eligibility, turn-ins, loot and LFG travel.','-- Source revision df19329b209fa1d1ae80444b43f6201d5dc124f0; SkyFire contributors, GPL-3.0.','-- Adapt to active split quest and LFG tables; preserve custom unrelated keys.'];ledger=[];desired=[]
 def emit_insert(t,names,rows):
  keys={'creature_questender':['id','quest'],'gameobject_questender':['id','quest'],'creature_loot_template':['entry','item','lootmode'],'item_loot_template':['entry','item','lootmode'],'skinning_loot_template':['entry','item','lootmode'],'lfg_dungeon_template':['dungeonId']}[t]
  out.append('INSERT INTO `'+t+'` ('+','.join('`'+x+'`' for x in names)+') VALUES\n'+',\n'.join('('+','.join(v)+')' for v in rows)+'\nON DUPLICATE KEY UPDATE '+','.join('`'+x+'`=VALUES(`'+x+'`)' for x in names if x.lower() not in {k.lower() for k in keys})+';') if len(names)>len(keys) else out.append('INSERT IGNORE INTO `'+t+'` ('+','.join('`'+x+'`' for x in names)+') VALUES\n'+',\n'.join('('+','.join(v)+')' for v in rows)+';')
  for vals in rows:desired.append(dict(table=t,keys=keys,values=dict(zip(names,vals))))
 for item in inventory:
  n=pathlib.Path(item['path']).name
  if '/old/' not in item['path'] or not (item['status'].startswith('pending') or item['status'].startswith('adapted quest/loot/travel subset in SQL42')):continue
  f=a.patches/n;assert hashlib.sha256(f.read_bytes()).hexdigest()==item['sha256'];cmds=statements(f.read_text(encoding='utf-8-sig'));selected=0;before=len(out);reasons=collections.Counter()
  out.append('-- '+n)
  for cmd in cmds:
   m=re.fullmatch(r'UPDATE `quest_template`\s+SET\s+(.*)',cmd,re.S|re.I)
   if m:
    body,where=split_where(m[1]);ids=re.fullmatch(r'`?Id`?\s*(?:=\s*(\d+)|IN\s*\(([^)]+)\))',where,re.I)
    if not ids:reasons['conditional quest update requires separate mapping']+=1;continue
    qids=[int(ids[1])] if ids[1] else [int(x.strip()) for x in ids[2].split(',')];fields=[]
    for v in comma_values(body):
     mm=re.fullmatch(r'`?(\w+)`?\s*=\s*(.*)',v.strip(),re.S)
     if not mm or mm[1] not in FIELDS or re.search(r'`\w+`',mm[2]):break
     fields.append((mm[1],mm[2]))
    else:
     for qid in qids:
      for key,value in fields:
       t,c=FIELDS[key]
       if t!='quest_template':out.append(f'INSERT IGNORE INTO `{t}` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`={qid};')
       out.append(f'UPDATE `{t}` SET `{c}`={value} WHERE `ID`={qid};');desired.append(dict(table=t,keys=['ID'],values={'ID':str(qid),c:value}))
     selected+=1;continue
    reasons['objective/native dependent quest fields']+=1;continue
   if n=='2026_08_20_world_00.sql':
    m=re.fullmatch(r'INSERT INTO `lfg_dungeon_template`\s*\((.*?)\)\s*VALUES\s*(.*)',cmd,re.S)
    if m:emit_insert('lfg_dungeon_template',[x.strip(' `') for x in comma_values(m[1])],[comma_values(x.strip()[1:-1]) for x in comma_values(m[2])]);selected+=1
    else:reasons['legacy LFG mirror table or broad delete unnecessary']+=1
    continue
   if n in FULL:
    m=re.fullmatch(r'INSERT INTO `(\w+)`\s*\((.*?)\)\s*VALUES\s*(.*)',cmd,re.S)
    if m:emit_insert(m[1],[x.strip(' `') for x in comma_values(m[2])],[comma_values(x.strip()[1:-1]) for x in comma_values(m[3])]);selected+=1;continue
    if cmd.startswith('DELETE FROM') and n!='2026_09_21_world_10.sql':reasons['preserve extra custom rows; keyed additions replace broad delete']+=1;continue
    if cmd.startswith('UPDATE') or n=='2026_09_21_world_10.sql':
     if n=='2026_09_21_world_10.sql':
      cmd+=' AND NOT (`entry`=200011 AND `spell`=106839)';reasons['retain local Skull Bash training until specialization-learning reconciliation']+=1
     out.append(cmd+';');selected+=1;continue
   reasons['outside this matched data module']+=1
  if not selected:out=out[:before]
  ledger.append(dict(file=n,source_path=item['path'],sha256=item['sha256'],selected_statements=selected,source_statements=len(cmds),remaining_reasons=dict(reasons)))
 assert desired,'Refusing to replace the migration with an empty selection'
 migration=root/'sql/updates/world/2026_11_06_42_world_release27_quests_loot_travel.sql';migration.write_text('\n\n'.join(out)+'\n',encoding='utf-8');(root/'doc/release27-world-ledger-2026-11-06.json').write_text(json.dumps(dict(upstream='df19329b209fa1d1ae80444b43f6201d5dc124f0',sources=ledger,desired=desired,migration_sha256=hashlib.sha256(migration.read_bytes()).hexdigest()),indent=2)+'\n',encoding='utf-8');print('sources',sum(x['selected_statements']>0 for x in ledger),'source statements',sum(x['selected_statements'] for x in ledger),'desired',len(desired))
if __name__=='__main__':main()
