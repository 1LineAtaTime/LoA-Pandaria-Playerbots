"""Reconcile historical SF quest fields with the final SF27 full-dump values.

Read-only assembly. Explicit column mapping; no raw old SQL execution. Preserve
newer SF/LoA overrides, quest identities and custom service tables.
"""
import argparse
import collections
import hashlib
import json
from pathlib import Path
import re
import sys
import sqlite3

from inventory_full_history import tree, blobs, git
from sql_batch import statements, comma_values
from assemble_release26_quest_batch import MAP, split_where

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'contrib/content-audit'))
from catalogue import World as ReadOnlyWorld


class World(ReadOnlyWorld):
    def q(self,sql):
        # The client occasionally exits successfully with no completion marker.
        # This base reader permits SELECT only; retry no mutations or SQL errors.
        for attempt in range(3):
            try:return super().q(sql)
            except RuntimeError as exc:
                if str(exc)!='Incomplete database read' or attempt==2:raise

FIELDS=dict(MAP)
FIELDS.update({old:('quest_template',new) for old,new in {
    'Level':'QuestLevel','MinLevel':'MinLevel','ZoneOrSort':'QuestSortID',
    'Type':'QuestInfoID','SuggestedPlayers':'SuggestedGroupNum','LimitTime':'TimeAllowed',
    'RewardXPId':'RewardXPDifficulty','RewardMoney':'RewardMoney',
    'RewardMoneyMaxLevel':'RewardBonusMoney','RewardTitleId':'RewardTitle',
    'RewardSkillId':'RewardSkillLineID','RewardSkillPoints':'RewardNumSkillUps',
    'CompletedText':'QuestCompletionLog','PointMapId':'POIContinent',
    'PointX':'POIx','PointY':'POIy','PointOption':'POIPriority',
    'QuestGiverTextWindow':'PortraitGiverText','QuestGiverTargetName':'PortraitGiverName',
    'QuestTurnTextWindow':'PortraitTurnInText','QuestTurnTargetName':'PortraitTurnInName',
}.items()})
FIELDS.update({old:('quest_template_addon',old) for old in ('MaxLevel','ExclusiveGroup','RequiredMinRepFaction','RequiredMaxRepFaction','RequiredMinRepValue','RequiredMaxRepValue')})
for n in range(1,7):
    FIELDS['RewardChoiceItemId'+str(n)]=('quest_template','RewardChoiceItemID'+str(n))
    FIELDS['RewardChoiceItemCount'+str(n)]=('quest_template','RewardChoiceItemQuantity'+str(n))
for n in range(1,5):
    FIELDS['RewardItemId'+str(n)]=('quest_template','RewardItem'+str(n))
    FIELDS['RewardItemCount'+str(n)]=('quest_template','RewardAmount'+str(n))
    FIELDS['RewardCurrencyId'+str(n)]=('quest_template','RewardCurrencyID'+str(n))
    FIELDS['RewardCurrencyCount'+str(n)]=('quest_template','RewardCurrencyQty'+str(n))
FIELD_CASE={key.lower():key for key in FIELDS}
EXTENDED_FIELDS={
    'RequiredClasses':('quest_template_addon','AllowableClasses'),
    'RequiredRaces':('quest_template','AllowableRaces'),
    'Method':('quest_template','QuestType'),'Flags':('quest_template','Flags'),
    'Flags2':('quest_template','FlagsEx'),'SpecialFlags':('quest_template_addon','SpecialFlags'),
    'SourceItemId':('quest_template','StartItem'),
    'SourceItemCount':('quest_template_addon','ProvidedItemCount'),
    'SourceSpellId':('quest_template_addon','SourceSpellID'),
    'RewardSpell':('quest_template','RewardDisplaySpell'),
    'RewardSpellCast':('quest_template','RewardSpell'),
    'EmoteOnComplete':('quest_request_items','EmoteOnComplete'),
    'EmoteOnIncomplete':('quest_request_items','EmoteOnIncomplete'),
}
for n in range(1,5):
    EXTENDED_FIELDS.update({
        'RequiredSourceItemId'+str(n):('quest_template','ItemDrop'+str(n)),
        'RequiredSourceItemCount'+str(n):('quest_template','ItemDropQuantity'+str(n)),
        'DetailsEmote'+str(n):('quest_details','Emote'+str(n)),
        'DetailsEmoteDelay'+str(n):('quest_details','EmoteDelay'+str(n)),
        'OfferRewardEmote'+str(n):('quest_offer_reward','Emote'+str(n)),
        'OfferRewardEmoteDelay'+str(n):('quest_offer_reward','EmoteDelay'+str(n)),
    })


def ids(where):
    match=re.fullmatch(r'\s*`?Id`?\s*(?:=\s*(\d+)|IN\s*\(([\d,\s]+)\))\s*',where,re.I)
    return [int(match[1])] if match and match[1] else [int(v) for v in match[2].split(',')] if match else []


def mentioned_ids(where):
    where=re.sub(r"'(?:''|\\.|[^'\\])*'",lambda m:m[0] if re.fullmatch(r"'\d+'",m[0]) else ' '*len(m[0]),where)
    result=set(ids(where))
    for match in re.finditer(r'(?<!\w)`?Id`?\s*=\s*\x27?(\d+)\x27?',where,re.I):result.add(int(match[1]))
    for match in re.finditer(r'(?<!\w)`?Id`?\s+IN\s*\(([\d,\s\x27]+)\)',where,re.I):
        result.update(int(v.strip(" '\r\n\t")) for v in match[1].split(','))
    for match in re.finditer(r'(?<!\w)`?Id`?\s+BETWEEN\s+(\d+)\s+AND\s+(\d+)',where,re.I):
        low,high=map(int,match.groups())
        if 0<=high-low<=10000:result.update(range(low,high+1))
    tuple_match=re.search(r'\(\s*`?Id`?\s*,[^)]*\)\s+IN\s*\((.*)',where,re.I|re.S)
    if tuple_match:result.update(int(v) for v in re.findall(r'\(\s*(\d+)\s*,',tuple_match[1]))
    return sorted(result)


def narrows_authored_mask(current,incoming):
    # A zero mask means unrestricted, not an empty set of playable classes/races.
    return bool(current and incoming and (incoming & current)!=current)


def decode(value):
    if value.upper()=='NULL':return None
    if not value.startswith("'"):return value
    if not value.endswith("'"):raise ValueError('Invalid source literal')
    text=value[1:-1].replace("''", "'")
    return re.sub(r'\\(.)',lambda m:{'n':'\n','r':'\r','t':'\t','0':'\0','b':'\b','Z':'\x1a'}.get(m[1],m[1]),text,flags=re.S)


def source_records(text,extended=False):
    if not re.search(r'UPDATE\s+`?quest_template`?\s+SET\b',text,re.I):
        return
    variables={}
    for index,cmd in enumerate(statements(text),1):
        setting=re.fullmatch(r'SET\s+(@\w+)\s*(?::=|=)\s*(\d+)',cmd,re.I)
        if setting:variables[setting[1].lower()]=setting[2]
        match=re.match(r'UPDATE\s+`?quest_template`?\s+SET\s+(.*)',cmd,re.I|re.S)
        if not match:continue
        try:body,where=split_where(match[1])
        except ValueError:body,where=match[1],''
        where=re.sub(r'@\w+',lambda m:variables.get(m[0].lower(),m[0]),where)
        fields=[]
        for part in comma_values(body):
            field=re.match(r'\s*`?(\w+)`?\s*=',part)
            if field:fields.append(FIELD_CASE.get(field[1].lower(),field[1]))
        yield index,mentioned_ids(where) if extended else ids(where),fields,where


def source_updates(text):
    for index,qids,fields,where in source_records(text):yield index,qids,fields


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--reference',type=Path,default=ROOT/'out/skyfire27-reference/base.sql')
    ap.add_argument('--output',type=Path,required=True)
    ap.add_argument('--extended',action='store_true')
    ap.add_argument('--allow-signed-next',action='store_true',help='Requires matching signed native/schema migration before deployment')
    a=ap.parse_args();a.output.mkdir(parents=True,exist_ok=True)
    if a.extended:
        FIELDS.update(EXTENDED_FIELDS);FIELD_CASE.update({k.lower():k for k in FIELDS})
    pin=git('rev-parse','skyfire/main').decode().strip();files=tree(pin)
    paths=[p for p in files if p.startswith('sql/old/5.4.8/world/') or p.startswith('sql/updates/world/')]
    data=blobs([files[p] for p in paths]);desired=collections.defaultdict(list);unsupported=collections.Counter();post_dump=set()
    # Never undo newer SF updates or the latest merged LoA data batch.
    loa=git('diff','--name-only','8a26cd41fe..68f8098431','--','sql/updates/world').decode().splitlines()
    for path in loa:
        text=git('show','68f8098431:'+path).decode('utf-8-sig')
        # Include IDs from all statement types referencing quest tables.
        for cmd in statements(text):
            if re.search(r'\bquest_(?:template|objective|offer_reward|request_items)\b',cmd,re.I):
                post_dump.update(int(x) for x in re.findall(r'(?<![\w.])-?\d+(?![\w.])',cmd))
    parse_errors=[];global_predicates=[]
    for path in sorted(paths):
        text=data[files[path]].decode('utf-8-sig',errors='replace')
        late=Path(path).name[:10]>'2026_11_03' and Path(path).name.startswith('2026_')
        try:updates=list(source_records(text,a.extended))
        except ValueError as exc:
            parse_errors.append(dict(path=path,error=str(exc)));continue
        for ordinal,qids,fields,where in updates:
            if late:post_dump.update(qids)
            if not qids:
                if a.extended:global_predicates.append(dict(path=path,statement=ordinal,fields=fields,where=where))
                else:unsupported['conditional_predicate']+=1
                continue
            for field in fields:
                if field not in FIELDS:unsupported['unmapped_field:'+field]+=1;continue
                fill=bool(re.search(r'(OfferRewardText|RequestItemsText)\s*`?\s+IS\s+NULL',where,re.I))
                for qid in qids:desired[(qid,field)].append(dict(path=path,statement=ordinal,fill_only=fill))
    raw=a.reference.read_bytes();ref_hash=hashlib.sha256(raw).hexdigest()
    cache=ROOT/'out/upstream-reference-cache'/('quest-template-v1-'+ref_hash+'.json')
    reference={}
    if cache.exists():
        cached=json.loads(cache.read_text(encoding='utf8'))
        if cached['source_sha256']!=ref_hash:raise RuntimeError('Stale reference cache')
        reference={int(qid):row for qid,row in cached['quests'].items()}
    else:
        text=raw.decode('utf-8-sig');start=text.index('CREATE TABLE `quest_template`')
        end=text.find('/*Table structure',start+1);segment=text[start:end if end!=-1 else len(text)]
        for cmd in statements(segment):
            match=re.match(r'INSERT\s+INTO\s+`quest_template`\s*\((.*?)\)\s*VALUES\s*(.*)',cmd,re.I|re.S)
            if not match:continue
            columns=[v.strip(' `\n\r') for v in comma_values(match[1])]
            for row in comma_values(match[2]):
                values=comma_values(row.strip()[1:-1]);assert len(values)==len(columns)
                item=dict(zip(columns,values));reference[int(item['Id'])]=item
        if len(reference)>=15000:
            cache.parent.mkdir(parents=True,exist_ok=True);temporary=cache.with_suffix('.tmp')
            temporary.write_text(json.dumps(dict(source_sha256=ref_hash,quests=reference)),encoding='utf8');temporary.replace(cache)
    if len(reference)<15000:raise RuntimeError('Incomplete reference quest table')
    predicate_dispositions=[]
    if global_predicates:
        live_reader=World(ROOT/'exports/worldserver.conf',ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
        live_view='(SELECT q.ID,q.QuestSortID AS ZoneOrSort,q.Flags,COALESCE(a.SpecialFlags,0) AS SpecialFlags,q.StartItem AS SourceItemId,COALESCE(a.ProvidedItemCount,0) AS SourceItemCount,q.MinLevel,q.QuestLevel AS Level,q.RewardSpell AS RewardSpellCast,q.RewardDisplaySpell AS RewardSpell,q.RewardSkillLineID AS RewardSkillId FROM quest_template q LEFT JOIN quest_template_addon a ON a.ID=q.ID) current_quest'
        model=sqlite3.connect(':memory:')
        cols=list(next(iter(reference.values())))
        model.execute('CREATE TABLE quest_template ('+','.join('`'+c+'`' for c in cols)+')')
        def sqlite_value(value):
            v=decode(value)
            return int(v) if v is not None and re.fullmatch(r'-?\d+',v) else float(v) if v is not None and re.fullmatch(r'-?\d+\.\d+',v) else v
        model.executemany('INSERT INTO quest_template VALUES ('+','.join('?' for _ in cols)+')',([sqlite_value(row[c]) for c in cols] for row in reference.values()))
        for entry in global_predicates:
            if 'RewardOrRequiredMoney' in entry['where']:
                predicate_dispositions.append(dict(entry,status='retired_schema_field_not_replayed'));continue
            try:qids=[int(row[0]) for row in model.execute('SELECT Id FROM quest_template WHERE '+entry['where'])]
            except sqlite3.Error as exc:
                if 'SourceItemCount' in entry['where']:qids=[]
                else:
                    predicate_dispositions.append(dict(entry,status='reference_schema_expression_not_supported',reason=str(exc)));continue
            current_ids=[int(row[0]) for row in live_reader.q('SELECT ID FROM '+live_view+' WHERE '+entry['where'])]
            qids=sorted(set(qids)|set(current_ids))
            predicate_dispositions.append(dict(entry,status='final_reference_scoped' if qids else 'no_match_in_final_reference',quests=qids))
            for field in entry['fields']:
                if field not in FIELDS:unsupported['unmapped_global_field:'+field]+=1;continue
                for qid in qids:desired[(qid,field)].append(dict(path=entry['path'],statement=entry['statement'],final_reference_predicate=entry['where']))
        model.close()
    # A corrected prerequisite can expose a stale prerequisite on its parent.
    # Include canonical ancestor fields so a partial import cannot create a loop.
    for qid,field in list(desired):
        if field!='PrevQuestId' or qid in post_dump or qid not in reference:continue
        visited={qid};parent=int(decode(reference[qid]['PrevQuestId']))
        while parent>0 and parent in reference and parent not in visited and parent not in post_dump:
            visited.add(parent)
            if (parent,'PrevQuestId') not in desired:
                desired[(parent,'PrevQuestId')].append(dict(reference='SFDB27 quest_template',dependency_for=qid))
            parent=int(decode(reference[parent]['PrevQuestId']))
    db=World(ROOT/'exports/worldserver.conf',ROOT/'out/build/database-tests/mariadb-10.5.29-winx64/bin/mariadb.exe')
    needed=collections.defaultdict(set)
    for qid,field in desired:table,column=FIELDS[field];needed[table].add(column)
    needed['quest_template'].add('LogTitle')
    if a.extended:needed['quest_template'].add('StartItem')
    active={};types={};column_types={}
    for table,columns in needed.items():
        columns=sorted(columns)
        info=db.q("SELECT COLUMN_NAME,DATA_TYPE,COLUMN_TYPE FROM information_schema.COLUMNS WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME='"+table+"'")
        types[table]={r[0].lower():r[1] for r in info}
        column_types[table]={r[0].lower():r[2] for r in info}
        if a.allow_signed_next and table=='quest_template_addon':column_types[table]['nextquestid']='int'
        if any(c.lower() not in types[table] for c in columns):raise RuntimeError('Missing mapped column in '+table)
        query='SELECT ID,'+','.join("IF(`"+c+"` IS NULL,'N',CONCAT('H',HEX(CAST(`"+c+'` AS BINARY))))' for c in columns)+' FROM `'+table+'`'
        active[table]={int(row[0]):dict(zip(columns,row[1:])) for row in db.q(query)}
    output=['-- Historical SF quest field reconciliation; source paths and field provenance in paired report.',
        '-- Final SFDB27 values supersede earlier patch values. Preserve newer SF/LoA quest overrides.',
        '-- Extended mode includes supported quest flags/masks/emotes; source identity and existing access are protected.' if a.extended else '-- No script/phase flags, account/character data, BattlePay, AH or playerbot tables.']
    reward_items={int(v[0]) for v in db.q('SELECT entry FROM item_template')}
    client_spells=set();emotes=set()
    if a.extended:
        from catalogue import Table
        for file,target in [('Spell.dbc',client_spells),('Emotes.dbc',emotes)]:
            dbc=Table(ROOT/'exports/dbc'/file);target.update(dbc.value(i,0) for i in range(dbc.count))
        client_spells.update(int(row[0]) for row in db.q('SELECT Id FROM spell_dbc'))
    results=[];counts=collections.Counter()
    for (qid,field),sources in sorted(desired.items()):
        row=dict(quest=qid,source_field=field,sources=sources)
        if qid in post_dump:row['status']='preserved_newer_override';results.append(row);counts[row['status']]+=1;continue
        if qid not in reference or qid not in active['quest_template']:
            row['status']='preserved_local_only_quest' if qid in active['quest_template'] and qid not in reference else 'source_placeholder_not_imported' if qid in reference and not decode(reference[qid]['Title']) else 'missing_quest_dependency';results.append(row);counts[row['status']]+=1;continue
        if field!='Title':
            title=active['quest_template'][qid]['LogTitle'];local_title=bytes.fromhex(title[1:]).decode('utf8') if title.startswith('H') else ''
            normal=lambda v:re.sub(r'\W+','',v.casefold())
            if normal(local_title)!=normal(decode(reference[qid]['Title'])):
                row.update(status='preserved_local_quest_identity',local_title=local_title,reference_title=decode(reference[qid]['Title']));results.append(row);counts[row['status']]+=1;continue
        table,column=FIELDS[field]
        if field not in reference[qid]:
            if field=='SourceItemCount':value='1' if int(decode(reference[qid]['SourceItemId'])) else '0'
            else:row['status']='field_absorbed_by_schema_evolution';results.append(row);counts[row['status']]+=1;continue
        else:value=reference[qid][field]
        decoded=decode(value)
        if field in ('RequiredRaces','RequiredClasses') and decoded=='-1':decoded=str(0xffffffff)
        if field=='NextQuestId' and decoded==str(-qid):
            row.update(status='preserved_local_successor_over_legacy_self_link',table=table,column=column);results.append(row);counts[row['status']]+=1;continue
        if decoded is None:row['status']='null_reference_skipped';results.append(row);counts[row['status']]+=1;continue
        numeric=types[table][column.lower()] not in ('char','varchar','text','mediumtext','longtext')
        if numeric and not re.fullmatch(r'-?\d+(?:\.\d+)?',decoded):raise ValueError('Not a numeric reference literal')
        if numeric and float(decoded)<0 and 'unsigned' in column_types[table][column.lower()]:
            row.update(status='requires_signed_field_semantics',table=table,column=column,target=decoded);results.append(row);counts[row['status']]+=1;continue
        if field in ('PrevQuestId','NextQuestId','NextQuestIdChain') and int(decoded) and abs(int(decoded)) not in active['quest_template']:
            row.update(status='missing_link_dependency',target=int(decoded));results.append(row);counts[row['status']]+=1;continue
        if re.fullmatch(r'Reward(?:Choice)?ItemId\d+',field) and int(decoded) and int(decoded) not in reward_items:
            row.update(status='missing_item_dependency',target=int(decoded));results.append(row);counts[row['status']]+=1;continue
        if a.extended and (re.fullmatch(r'RequiredSourceItemId\d+',field) or field=='SourceItemId') and int(decoded) and int(decoded) not in reward_items:
            row.update(status='missing_item_dependency',target=int(decoded));results.append(row);counts[row['status']]+=1;continue
        if a.extended and field in ('SourceSpellId','RewardSpell','RewardSpellCast') and int(decoded) and abs(int(decoded)) not in client_spells:
            row.update(status='missing_spell_dependency',target=int(decoded));results.append(row);counts[row['status']]+=1;continue
        if a.extended and 'Emote' in field and 'Delay' not in field and int(decoded) not in emotes:
            row.update(status='missing_emote_dependency',target=int(decoded));results.append(row);counts[row['status']]+=1;continue
        current=active[table].get(qid,{}).get(column)
        if a.extended and field in ('RequiredClasses','RequiredRaces') and current and current.startswith('H'):
            old_mask=int(bytes.fromhex(current[1:]).decode());new_mask=int(decoded)
            if narrows_authored_mask(old_mask,new_mask):
                row.update(status='preserved_existing_class_race_access',before=current,reference_mask=new_mask);results.append(row);counts[row['status']]+=1;continue
        if a.extended and field=='SourceItemCount':
            item=active['quest_template'][qid].get('StartItem','H30')
            if int(bytes.fromhex(item[1:]).decode())!=int(decode(reference[qid]['SourceItemId'])) or int(decoded)==0:
                row['status']='preserved_local_start_item_policy';results.append(row);counts[row['status']]+=1;continue
        if sources and all(s.get('fill_only',False) for s in sources) and current not in (None,'N','H'):
            row['status']='preserved_nonempty_authored_text';results.append(row);counts[row['status']]+=1;continue
        new='H'+decoded.encode().hex().upper()
        equal=current==new
        if numeric and current and current.startswith('H'):
            equal=abs(float(bytes.fromhex(current[1:]).decode())-float(decoded))<=max(.00001,abs(float(decoded))*.000001)
        row.update(table=table,column=column,before=current,after=new)
        if equal:row['status']='already_matches'
        else:
            if current is None:
                output.append(f'INSERT IGNORE INTO `{table}` (`ID`) SELECT ID FROM quest_template WHERE ID={qid};')
                guard='1'
            else:guard="IF(`"+column+"` IS NULL,'N',CONCAT('H',HEX(CAST(`"+column+"` AS BINARY))))='"+current+"'"
            literal=decoded if numeric else "CONVERT(X'"+new[1:]+"' USING utf8mb4)"
            output.append(f'UPDATE `{table}` SET `{column}`={literal} WHERE ID={qid} AND '+guard+';')
            row['status']='prepared_change'
        results.append(row);counts[row['status']]+=1
    migration=a.output/'historical-quest-fields.sql';migration.write_text('\n'.join(output)+'\n',encoding='utf-8')
    report=dict(pin=pin,source_sql_files=len(paths),reference_sha256=hashlib.sha256(raw).hexdigest(),
        assembler_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        reference_quests=len(reference),counts=dict(counts),excluded=dict(unsupported),extended=a.extended,requires_signed_next=a.allow_signed_next,predicate_dispositions=predicate_dispositions,
        sql_sha256=hashlib.sha256(migration.read_bytes()).hexdigest(),results=results,parse_errors=parse_errors,
        status='PREPARED_NOT_APPLIED',scope='Mapped quest fields addressed by explicit-ID historical UPDATEs; not all history or gameplay.')
    (a.output/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({k:v for k,v in report.items() if k not in ('results','excluded','predicate_dispositions')}))


if __name__=='__main__':main()
