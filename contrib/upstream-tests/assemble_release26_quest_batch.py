"""Assemble the reviewed release-26 quest/SmartAI subset for this fork."""
import argparse,pathlib,csv,hashlib,re,json,collections
from sql_batch import statements,comma_values
SMART={f'2026_08_12_world_{n:02d}.sql' for n in (2,5,6,7,8,9)}|{'2026_08_13_world_02.sql','2026_08_13_world_03.sql','2026_08_01_world_00.sql','2026_07_29_world_05.sql','2026_07_29_world_07.sql','2026_08_14_world_05.sql'}
TRAIN={'2026_07_25_world_04.sql','2026_07_25_world_05.sql','2026_07_25_world_07.sql',*[f'2026_07_26_world_{n:02d}.sql' for n in (1,2,3,4)]}
MAP={'OfferRewardText':('quest_offer_reward','RewardText'),'RequestItemsText':('quest_request_items','CompletionText'),'Details':('quest_template','QuestDescription'),'Objectives':('quest_template','LogDescription'),'Title':('quest_template','LogTitle'),'EndText':('quest_template','AreaDescription'),'PrevQuestId':('quest_template_addon','PrevQuestID'),'NextQuestId':('quest_template_addon','NextQuestID'),'NextQuestIdChain':('quest_template','RewardNextQuest'),'RequiredSkillId':('quest_template_addon','RequiredSkillID'),'RequiredSkillPoints':('quest_template_addon','RequiredSkillPoints')}

def split_where(s):
 quote=None;i=0
 while i<len(s):
  c=s[i]
  if quote:
   if c=='\\' and quote!='`':i+=1
   elif c==quote:
    if i+1<len(s) and s[i+1]==quote:i+=1
    else:quote=None
  elif c in "'\"`":quote=c
  elif re.match(r'\sWHERE\s',s[i:],re.I):
   m=re.match(r'\sWHERE\s+',s[i:],re.I);return s[:i],s[i+len(m[0]):]
  i+=1
 raise ValueError('Missing WHERE')

def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--patches',type=pathlib.Path,required=True);ap.add_argument('--output',type=pathlib.Path,required=True);ap.add_argument('--ledger',type=pathlib.Path,required=True);ap.add_argument('--desired',type=pathlib.Path,required=True);a=ap.parse_args()
 root=pathlib.Path(__file__).resolve().parents[2];catalog=list(csv.DictReader((root/'doc/published-sql-compatibility-2026-11-05.csv').open(encoding='utf-8-sig')))
 output=['-- Release-26 published quest and starter AI adaptations for the LoA runtime schema.','-- SkyFire contributors: GPL-3.0; pinned upstream links and original hashes in the paired ledger.','-- No account, character, mail, item catalog, BattlePay or navigation changes.'];ledger=[];desired={};ensured=set()
 for item in catalog:
  p=a.patches/item['patch'];raw=p.read_bytes();assert hashlib.sha256(raw).hexdigest()==item['sha256'],p
  source=raw.decode('utf-8-sig');cmds=statements(source);selected=[];excluded=collections.Counter();original_selected=0
  # Determine the exact source-type ownership before narrowing legacy deletes.
  pairs=collections.defaultdict(set)
  for cmd in cmds:
   m=re.match(r'INSERT INTO `?smart_scripts`?\s*\((.*?)\)\s*VALUES\s*(.*)',cmd,re.I|re.S)
   if m:
    cols=[x.strip(' `\r\n\t').lower() for x in comma_values(m[1])]
    for row in comma_values(m[2]):
     vals=comma_values(row.strip()[1:-1]);pairs[int(vals[cols.index('entryorguid')])].add(int(vals[cols.index('source_type')]))
  for cmd in cmds:
   m=re.match(r'UPDATE\s+`?quest_template`?\s+SET\s+(.*)',cmd,re.I|re.S)
   if m:
    assignments,where=split_where(m[1]);idm=re.fullmatch(r'\s*`?Id`?\s*=\s*(\d+)\s*',where,re.I)
    if not idm:excluded['conditional quest update requires separate evidence']+=1;continue
    qid=int(idm[1]);parts=[];valid=True;prior_ensured=set(ensured);prior_desired=dict(desired)
    for assignment in comma_values(assignments):
     mm=re.fullmatch(r'\s*`?(\w+)`?\s*=\s*(.*)',assignment,re.S)
     if not mm or mm[1] not in MAP:valid=False;break
     col=mm[1]
     if item['patch'] in TRAIN and col not in ('OfferRewardText','RequestItemsText','Details','Objectives','Title','EndText'):
      valid=False;break
     if item['patch'].startswith('2026_07_27_'):valid=False;break
     # Correct the published Night Elf Simple Sigil ID before importing text.
     actualid=3116 if item['patch']=='2026_07_26_world_02.sql' and qid==3106 else qid
     table,field=MAP[col];value=mm[2]
     if table!='quest_template' and (table,actualid) not in ensured:
      parts.append(f'INSERT INTO `{table}` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`={actualid} AND NOT EXISTS (SELECT 1 FROM `{table}` WHERE `ID`={actualid});');ensured.add((table,actualid))
     parts.append(f'UPDATE `{table}` SET `{field}`={value} WHERE `ID`={actualid};');desired[(table,actualid,field)]=value
    if valid:selected.extend(parts);original_selected+=1
    else:
     ensured=prior_ensured;desired=prior_desired;excluded['numeric/training/cleanup update retained for separate adaptation']+=1
    continue
   if item['patch'] in SMART and re.match(r'(?:UPDATE `?(?:creature_template|gameobject_template)`?|DELETE FROM `?(?:creature_text|smart_scripts)`?|INSERT INTO `?(?:creature_text|smart_scripts)`?)\b',cmd,re.I):
    if re.match(r'INSERT INTO `?creature_text`?',cmd,re.I):
     mm=re.match(r'(INSERT INTO `?creature_text`?\s*)\((.*?)\)\s*VALUES\s*(.*)',cmd,re.I|re.S);cols=comma_values(mm[2]);cols.insert(1,'`CreatureID`');rows=[]
     for row in comma_values(mm[3]):
      vals=comma_values(row.strip()[1:-1]);vals.insert(1,vals[0]);rows.append('('+', '.join(vals)+')')
     cmd=mm[1]+'('+', '.join(cols)+') VALUES\n'+',\n'.join(rows)
    if re.match(r'DELETE FROM `?smart_scripts`?',cmd,re.I) and not re.search(r'\bsource_type\b',cmd,re.I):
     mid=int(re.search(r'entryorguid`?\s*=\s*(\d+)',cmd,re.I)[1]);types=sorted(pairs[mid]);assert len(types)==1,(item['patch'],mid,types);cmd+=' AND `source_type`='+str(types[0])
    if item['patch']=='2026_08_12_world_06.sql' and re.match(r'INSERT INTO `?smart_scripts`?',cmd,re.I):
     cmd=cmd.replace('(946, 0, 1, 1, 4,','(946, 0, 1, 2, 4,')
    if item['patch']=='2026_07_29_world_05.sql' and re.match(r'INSERT INTO `?smart_scripts`?',cmd,re.I):
     mm=re.match(r'(INSERT INTO `?smart_scripts`?\s*)\((.*?)\)\s*VALUES\s*(.*)',cmd,re.I|re.S);names=[x.strip(' `').lower() for x in comma_values(mm[2])];rows=[]
     for row in comma_values(mm[3]):
      values=comma_values(row.strip()[1:-1])
      if values[names.index('entryorguid')]=='4264500' and values[names.index('id')]=='1':
       assert values[names.index('action_type')]=='46' and values[names.index('action_param1')]=='2'
       values[names.index('action_type')]='114';values[names.index('action_param1')]='0';values[names.index('target_y')]='2'
      rows.append('('+', '.join(values)+')')
     cmd=mm[1]+'('+mm[2]+') VALUES\n'+',\n'.join(rows)
    selected.append(cmd+';');original_selected+=1
   else:excluded['outside reviewed quest/starter AI subset']+=1
  if selected:output+=['\n-- '+item['patch']+' | '+item['source'],*selected]
  ledger.append(dict(patch=item['patch'],sha256=item['sha256'],source=item['source'],source_statements=len(cmds),selected_statements=original_selected,status='selected full data patch' if original_selected==len(cmds) else ('selected subset' if original_selected else 'not in this batch'),remaining_reasons=dict(excluded)))
 a.output.write_text('\n'.join(output)+'\n',encoding='utf-8');a.ledger.write_text(json.dumps(ledger,indent=2)+'\n',encoding='utf-8');a.desired.write_text(json.dumps([dict(table=t,id=i,field=f,sql_value=v) for (t,i,f),v in sorted(desired.items())],indent=2)+'\n',encoding='utf-8')
 print('Sources',sum(bool(x['selected_statements']) for x in ledger),'selected source statements',sum(x['selected_statements'] for x in ledger),'desired fields',len(desired));print(collections.Counter(x['status'] for x in ledger))
if __name__=='__main__':main()
