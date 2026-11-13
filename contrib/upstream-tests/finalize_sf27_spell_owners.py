"""Assign each imported script identity once, retaining local-only registrations."""
from pathlib import Path
import json,re,subprocess,collections
ROOT=Path(__file__).resolve().parents[2]
def main():
 metas={p:json.loads(p.read_text(encoding='utf-8')) for p in (ROOT/'doc').glob('sf27-*-native-port.json')};owners=collections.defaultdict(list)
 for d in metas.values():
  for n in d['upstream_names']:owners[n].append(d['module'])
 assert not [n for n,v in owners.items() if len(v)>1], 'Duplicate upstream owner'
 for p,d in metas.items():
  mod=d['module'];path=ROOT/f'src/server/scripts/Spells/spell_{mod}.cpp';r=subprocess.run(['git','show',d['local_baseline']+':'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8',capture_output=True)
  if r.returncode:continue
  source=r.stdout;m=re.search(r'void (AddSC_\w+)\(\)\s*\{',source);assert m
  head=source[:m.start()];tail=source[m.start():];masked=re.sub(r'/\*.*?\*/|//[^\n]*',lambda m:''.join('\n' if x=='\n' else ' ' for x in m[0]),tail,flags=re.S);out=[];kept=[];replaced=[];allnames=[]
  ctors=dict(re.findall(r'(\w+)\s*\([^)]*\)\s*:\s*(?:SpellScriptLoader|PlayerScript|CreatureScript|AreaTriggerEntityScript)\s*\(\s*"([^"]+)"',source))
  for line,active in zip(tail.splitlines(),masked.splitlines()):
   create=re.search(r'new\s+(\w+)(?:<[^;]*?>)?\s*\(([^;]*)\)',active)
   if create:
    names=re.findall(r'"([^"]+)"',create[2]);name=names[0] if names else ctors.get(create[1],create[1]);allnames.append(name)
    if name in owners:replaced.append(name);continue
    kept.append(name)
   out.append(line)
  entries=['AddSC_skyfire27_'+mod]
  if mod=='generic':entries += ['AddSC_skyfire27_'+x for x in ('mastery','archaeology') if any(v['module']==x for v in metas.values())]
  assert out[-1].strip()=='}'
  for entry in entries:out.insert(len(out)-1,'    '+entry+'();')
  path.write_text(head+'\n'.join('void '+x+'();' for x in entries)+'\n\n'+'\n'.join(out)+'\n',encoding='utf-8')
  d.update(local_original_registrations=allnames,local_overridden=replaced,local_only_retained=kept);p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf-8')
 # Shared local spell modules can own identities also present in imported class modules.
 for mod in ('common','enchantment'):
  path=ROOT/f'src/server/scripts/Spells/spell_{mod}.cpp'
  source=subprocess.check_output(['git','show','9c32347f1:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8')
  lines=source.splitlines();out=[]
  for line in lines:
   names=re.findall(r'new\s+(?:spell_script|aura_script)\s*<[^>]+>\s*\(\s*"([^"]+)"',line)
   if names and names[0] in owners:continue
   out.append(line)
  path.write_text('\n'.join(out)+'\n',encoding='utf-8')
 print('Imported modules:',len(metas),'unique upstream identities:',len(owners))
if __name__=='__main__':main()
