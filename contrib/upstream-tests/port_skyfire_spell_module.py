"""Port a pinned SF spell module as a separate namespace, preserving local-only registrations."""
from skyfire_spell_api import adapt
import argparse,pathlib,re,json,hashlib,subprocess
ROOT=pathlib.Path(__file__).resolve().parents[2]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--source',type=pathlib.Path,required=True);p.add_argument('--module',required=True);p.add_argument('--base-ref',default='9c32347f1');a=p.parse_args();mod=a.module;assert re.fullmatch(r'[a-z_]+',mod)
 original=ROOT/f'src/server/scripts/Spells/spell_{mod}.cpp';raw=a.source.read_text(encoding='utf-8-sig');prior=subprocess.run(['git','show',a.base_ref+':'+original.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8',capture_output=True);local=prior.stdout if prior.returncode==0 else '';assert local or mod in ('mastery','archaeology');function=re.search(r'void (AddSC_\w+)\(\)\s*\{',raw);assert function
 namespace='SkyFire27_'+mod;entry='AddSC_skyfire27_'+mod
 ctors=dict(re.findall(r'(\w+)\s*\([^)]*\)\s*:\s*(?:SpellScriptLoader|PlayerScript|CreatureScript|AreaTriggerEntityScript)\s*\(\s*"([^"]+)"',raw))
 tail=re.sub(r'/\*.*?\*/|//[^\n]*','',raw[function.end():],flags=re.S)
 names=set(re.findall(r'\bRegister(?:Spell|Aura)Script\(\s*(\w+)\s*\)',tail))
 for cls,args in re.findall(r'\bnew\s+(\w+)(?:<[^;]*?>)?\s*\(([^;\n]*)\)',tail):
  quoted=re.findall(r'"([^"]+)"',args);names.add(quoted[0] if quoted else ctors.get(cls,cls))
 assert names

 # Unregistered legacy loader classes are not runtime fixes; retain their source hash, not dead compile dependencies.
 omitted=[];active_classes=set(re.findall(r'\bnew\s+(\w+)\s*\(',re.sub(r'/\*.*?\*/|//[^\n]*','',raw,flags=re.S)))
 masked=re.sub(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',lambda m:' '*len(m[0]),raw,flags=re.S);cuts=[]
 for m in re.finditer(r'\bclass\s+(\w+)\s*:\s*public\s+SpellScriptLoader\s*\{',masked):
  if m[1] in active_classes:continue
  start=masked.index('{',m.start());i=start+1;depth=1
  while depth:
   if masked[i]=='{':depth+=1
   elif masked[i]=='}':depth-=1
   i+=1
  while i<len(masked) and masked[i].isspace():i+=1
  assert masked[i]==';';cuts.append((m.start(),i+1));omitted.append(m[1])
 for start,end in reversed(cuts):raw=raw[:start]+raw[end:]
 function=re.search(r'void (AddSC_\w+)\(\)\s*\{',raw)
 lastinclude=list(re.finditer(r'^#include[^\n]*',raw,re.M))[-1].end();converted=raw[:lastinclude]+f'\n\nnamespace {namespace}\n{{\n'+raw[lastinclude:]+'\n}\n\nvoid '+entry+'()\n{\n    '+namespace+'::'+function[1]+'();\n}\n'
 converted=converted.replace(' OVERRIDE',' override').replace('Skyfire::','Trinity::')
 for old,new in {'getLevel':'GetLevel','getRaceMask':'GetRaceMask','getRace':'GetRace','getClass':'GetClass','getClassMask':'GetClassMask','getGender':'GetGender'}.items():converted=converted.replace('->'+old+'(', '->'+new+'(')
 # Aura amount hooks use floating point in LoA; periodic interval/absorb refs retain their native types.
 hooks=set(re.findall(r'AuraEffectCalcAmountFn\(\s*\w+::(\w+)',converted))
 for name in hooks:converted=re.sub(r'(void\s+'+name+r'\s*\(AuraEffect const\*[^,]*,\s*)int32\s*&',r'\1float&',converted)
 disabled=[];kept=[]
 if local:
  marker=re.search(r'void (AddSC_\w+)\(\)\s*\{',local);assert marker;head=local[:marker.start()];tail=local[marker.start():];disabled=[];kept=[];out=[]
  for line in tail.splitlines():
   if 'new ' in line:
    quoted=re.findall(r'"([^"]+)"',line);cls=re.search(r'new\s+(\w+)\s*\(',line);name=quoted[0] if quoted else (cls[1] if cls else None)
    if name in names:disabled.append(name);continue
    if name:kept.append(name)
   out.append(line)
  assert out[-1].strip()=='}';out.insert(len(out)-1,'    '+entry+'();')
 converted=re.sub(r'Register(Spell|Aura)Script\(\s*(\w+)\s*\);',lambda m:'new '+('spell_script' if m[1]=='Spell' else 'aura_script')+'<'+m[2]+'>('+chr(34)+m[2]+chr(34)+');',converted)
 converted=adapt(mod,converted).replace('See LICENSE.md file for Copyright information','See SKYFIRE27-LICENSE.md for Copyright information')
 if local:original.write_text(head+'void '+entry+'();\n\n'+'\n'.join(out)+'\n',encoding='utf-8')
 target=ROOT/f'src/server/scripts/Spells/spell_{mod}_sf27.cpp';target.write_text(converted,encoding='utf-8')
 cmake=ROOT/'src/server/scripts/Spells/CMakeLists.txt';text=cmake.read_text(encoding='utf-8');anchor=f'  Spells/spell_{mod}.cpp';new=f'  Spells/spell_{mod}_sf27.cpp'
 if new not in text:text=text.replace(anchor,anchor+'\n'+new) if anchor in text else text.replace('\n)\n','\n'+new+'\n)\n',1)
 cmake.write_text(text,encoding='utf-8')
 result=dict(module=mod,local_baseline=a.base_ref,upstream='df19329b209fa1d1ae80444b43f6201d5dc124f0',source_sha256=hashlib.sha256(a.source.read_bytes()).hexdigest(),upstream_names=sorted(names),unregistered_legacy_classes_omitted=omitted,local_overridden=disabled,local_only_retained=kept,api_adaptations=['Skyfire namespace to Trinity','override keyword','float aura amount callbacks'])
 path=ROOT/f'doc/sf27-{mod}-native-port.json';path.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8');print(json.dumps(dict(module=mod,local_baseline=a.base_ref,upstream=len(names),overridden=len(disabled),local_only=len(kept))))
if __name__=='__main__':main()
