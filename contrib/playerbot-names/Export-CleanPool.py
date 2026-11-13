"""Generate static candidate-only SQL from the canonical CSV; never exports live data."""
from pathlib import Path
import csv,hashlib,re
R=Path(__file__).resolve().parents[2];source=R/'modules/mod_playerbots/data/names/playerbot_names.csv';sha=hashlib.sha256(source.read_bytes()).hexdigest();classes={'Warrior':1,'Paladin':2,'Hunter':3,'Rogue':4,'Priest':5,'Death Knight':6,'Shaman':7,'Mage':8,'Warlock':9,'Monk':10,'Druid':11};races={'Human':1,'Orc':2,'Dwarf':3,'Night Elf':4,'Undead':5,'Tauren':6,'Gnome':7,'Troll':8,'Goblin':9,'Blood Elf':10,'Draenei':11,'Worgen':22,'Pandaren':24};rows=list(csv.DictReader(source.open(encoding='utf-8',newline='')));seen=set();values=[]
def quote(s):return "'"+s.replace("'","''")+"'"
for x in rows:
 name=x['name'];assert re.fullmatch('[A-Z][a-z]{1,11}',name) and name.lower() not in seen;seen.add(name.lower());assert 1<=int(x['weight'])<=1000
 enabled=int(not(re.search(r'(.)\1\1',name.lower()) or x['category']=='soft_fantasy'))
 values.append('('+','.join([quote(name),quote(x['category']),quote(x['tier']),str({'neutral':0,'feminine':1,'masculine':2}[x['gender_hint']]),str(classes.get(x['class_hint'],0)),str(races.get(x['race_hint'],0)),str(int(x['weight'])),str(enabled),quote(sha)])+')')
out=R/'modules/mod_playerbots/data/sql/characters/curated_name_candidates.sql';out.parent.mkdir(parents=True,exist_ok=True);header='-- Generated from data/names/playerbot_names.csv by Export-CleanPool.py.\n-- Apply the curated naming character schema first. Candidate metadata only; no accounts, characters, assignments or credentials.\n-- Existing claims are retained. Native reserved-name validation remains authoritative.\n'
insert='INSERT IGNORE INTO playerbot_curated_names(name,category,tier,gender_hint,class_hint,race_hint,weight,enabled,import_sha) VALUES\n'
out.write_text(header+'\n'.join(insert+',\n'.join(values[i:i+400])+';\n' for i in range(0,len(values),400)),encoding='utf-8');print('Generated clean candidate-only SQL:',len(values),'rows')
