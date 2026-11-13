"""Extract only required reference groups from the pinned public SF26.002 package."""
import argparse,pathlib,zipfile,hashlib,json,re,collections
from sql_batch import statements,comma_values
DIGEST='d12ba9ecad381f3b5460612af2a172cd373afdd796c289e2ccc11315db21dd5f'
URL='https://github.com/ProjectSkyfire/SkyFire_548/releases/download/sf_db_26/SFDB_full_548_26.002_2026_008_18_Release.zip'
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--package',type=pathlib.Path,required=True);ap.add_argument('--ledger',type=pathlib.Path,required=True);a=ap.parse_args();assert hashlib.sha256(a.package.read_bytes()).hexdigest()==DIGEST
 root=pathlib.Path(__file__).resolve().parents[2];rows=collections.defaultdict(list)
 with zipfile.ZipFile(a.package) as z:
  assert len(z.namelist())==1
  with z.open(z.namelist()[0]) as f:
   for line in f:
    if not line.startswith(b'INSERT INTO `reference_loot_template`'):continue
    s=line.decode('utf-8').strip().rstrip(';');values=re.split(r'\bVALUES\b',s,maxsplit=1)[1]
    for v in comma_values(values):
     r=comma_values(v.strip()[1:-1]);rows[int(r[0])].append(r)
 needed={24036,24116,24161,24736,44011};queue=list(needed)
 while queue:
  mid=queue.pop();assert rows[mid],mid
  for r in rows[mid]:
   if int(r[5])<0 and -int(r[5]) not in needed:needed.add(-int(r[5]));queue.append(-int(r[5]))
 # Reject a cycle even if all referenced groups exist.
 def visit(mid,stack):
  assert mid not in stack,stack+[mid]
  for r in rows[mid]:
   if int(r[5])<0:visit(-int(r[5]),stack+[mid])
 for mid in needed:visit(mid,[])
 normalizations=[]
 for mid in sorted(needed):
  groups=collections.defaultdict(list)
  for r in rows[mid]:
   if int(r[4]):groups[int(r[4])].append(r)
  for gid,group in groups.items():
   total=sum(max(float(r[2]),0) for r in group)
   if total>100.01:
    # This pinned source has a 110% group. Keep its relative published weights.
    assert (mid,gid,total)==(24161,1,110.0),(mid,gid,total)
    for r in group:r[2]=format(float(r[2])*100/total,'.8f')
    normalizations.append(dict(source_entry=mid,group=gid,original_total=total,method='scale positive weights proportionally to total100'))
 mapped=[]
 for mid in sorted(needed):
  for src in rows[mid]:
   r=list(src);r[0]=str(mid+6000000)
   if int(r[5])<0:r[5]=str(int(r[5])-6000000)
   mapped.append(r)
 output=['-- Scoped SkyFire26.002 reference dependencies; GPL-3.0 upstream contributors.','-- Source: '+URL,'-- Asset SHA256: '+DIGEST,'-- Map IDs by +6000000 to preserve existing local groups.','-- Source24161/group1 totals110%; normalize to100 while preserving relative weights.']
 output.append('INSERT INTO `reference_loot_template` (`entry`,`item`,`ChanceOrQuestChance`,`lootmode`,`groupid`,`mincountOrRef`,`maxcount`) VALUES\n'+',\n'.join('('+','.join(r)+')' for r in mapped)+'\nON DUPLICATE KEY UPDATE `ChanceOrQuestChance`=VALUES(`ChanceOrQuestChance`),`groupid`=VALUES(`groupid`),`mincountOrRef`=VALUES(`mincountOrRef`),`maxcount`=VALUES(`maxcount`);')
 (root/'sql/updates/world/2026_11_06_38_world_release26_loot_references.sql').write_text('\n'.join(output)+'\n',encoding='utf-8')
 ledger=json.loads(a.ledger.read_text(encoding='utf-8'));ledger['reference_source']=dict(url=URL,name=a.package.name,bytes=a.package.stat().st_size,sha256=DIGEST);ledger['reference_mapping']={str(x):x+6000000 for x in sorted(needed)};ledger['reference_normalizations']=normalizations
 ledger['desired']=[x for x in ledger['desired'] if x['table']!='reference_loot_template'];cols=['entry','item','ChanceOrQuestChance','lootmode','groupid','mincountOrRef','maxcount'];ledger['desired'] += [dict(table='reference_loot_template',values=dict(zip(cols,r)),keys=['entry','item','lootmode']) for r in mapped];a.ledger.write_text(json.dumps(ledger,indent=2)+'\n',encoding='utf-8')
 print('Reference groups',len(needed),'rows',len(mapped),'normalizations',normalizations)
if __name__=='__main__':main()
