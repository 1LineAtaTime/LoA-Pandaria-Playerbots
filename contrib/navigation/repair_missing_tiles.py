#!/usr/bin/env python3
"""Windows-only asset repair. Client data and output stay outside Git.
The generator uses Y,X arguments for X,Y filenames. Parent meshes are preserved.
No C++ build, SQL mutation or logging-level change is performed.
"""
import pathlib,json,subprocess,os,shutil,hashlib,concurrent.futures,collections,time
import argparse,csv,_winapi
parser=argparse.ArgumentParser(description="Generate missing navigation tiles from installed inputs; validate against existing parents before optional installation.")
parser.add_argument("--inventory",type=pathlib.Path,required=True,help="CSV with map,x,y columns")
parser.add_argument("--runtime",type=pathlib.Path,default=pathlib.Path("exports"))
parser.add_argument("--work",type=pathlib.Path,required=True)
parser.add_argument("--generator",type=pathlib.Path,default=pathlib.Path("out/build/vs2022-x64/bin/RelWithDebInfo/mmaps_generator.exe"))
parser.add_argument("--probe",type=pathlib.Path,default=pathlib.Path("out/build/vs2022-x64/bin/RelWithDebInfo/mmap_load_probe.exe"))
parser.add_argument("--workers",type=int,default=8)
parser.add_argument("--install",action="store_true",help="Install validated additions; never replace existing tiles or parents")
args=parser.parse_args()
if not 1<=args.workers<=16:parser.error("workers must be between 1 and 16")
root=pathlib.Path.cwd();ex=args.runtime.resolve();stage=args.work.resolve()
if stage.is_relative_to(ex) or ex.is_relative_to(stage):parser.error('work directory must be separate from runtime')
if not args.generator.is_file() or not args.probe.is_file():parser.error('generator and probe executables are required')
stage.mkdir(parents=True,exist_ok=True)
with args.inventory.open(encoding="utf-8-sig",newline="") as f:rows=[{k:int(r[k]) for k in ("map","x","y")} for r in csv.DictReader(f)]
if len({(r['map'],r['x'],r['y']) for r in rows})!=len(rows):parser.error("duplicate coordinates")
for r in rows:
 if r['map']<0 or not 0<=r['x']<64 or not 0<=r['y']<64:parser.error("invalid coordinates")
 if not (ex/'mmaps'/f"{r['map']:04d}.mmap").is_file():parser.error(f"missing parent for map {r['map']}")
(stage/'inventory.json').write_text(json.dumps(rows,indent=2),encoding='utf-8')
for mid in sorted({r['map'] for r in rows}):
 d=stage/str(mid);(d/'mmaps').mkdir(parents=True,exist_ok=True)
 for sub in ('maps','vmaps','dbc'):
  if not (d/sub).exists():
   _winapi.CreateJunction(str(ex/sub),str(d/sub))
 shutil.copy2(ex/'mmaps'/f'{mid:04d}.mmap',d/'mmaps'/f'{mid:04d}.mmap')
env=dict(os.environ,PATH=str(ex)+';'+os.environ['PATH'])
gen=args.generator.resolve();probe=args.probe.resolve()
def work(r):
 mid,x,y=r['map'],r['x'],r['y'];name=f'{mid:04d}_{x:02d}_{y:02d}.mmtile';d=stage/str(mid);target=ex/'mmaps'/name;src=d/'mmaps'/name
 result=dict(tile=name,map=mid,x=x,y=y)
 try:
  if target.exists():return dict(result,status='existing preserved')
  if not src.exists():
   p=subprocess.run([str(gen),str(mid),'--tile',f'{y},{x}','--threads','1','--silent'],cwd=d,env=env,capture_output=True,timeout=600)
   (d/(name+'.log')).write_bytes(p.stdout+p.stderr)
   if p.returncode:return dict(result,status='generator failed',exit=p.returncode)
  if not src.exists():return dict(result,status='no generated geometry')
  v=stage/'validation'/name;(v/'mmaps').mkdir(parents=True,exist_ok=True)
  shutil.copy2(ex/'mmaps'/f'{mid:04d}.mmap',v/'mmaps'/f'{mid:04d}.mmap');shutil.copy2(src,v/'mmaps'/name)
  for dx in (-1,0,1):
   for dy in (-1,0,1):
    if dx==0 and dy==0:continue
    n=f'{mid:04d}_{x+dx:02d}_{y+dy:02d}.mmtile'
    if (ex/'mmaps'/n).exists():shutil.copy2(ex/'mmaps'/n,v/'mmaps'/n)
  cfg=v/'probe.conf';cfg.write_text('[worldserver]\nDataDir = "'+v.as_posix()+'"\n',encoding='utf-8')
  p=subprocess.run([str(probe),str(cfg),str(mid),str(x),str(y)],cwd=root,env=env,capture_output=True,timeout=30)
  (v/'probe.log').write_bytes(p.stdout+p.stderr)
  if p.returncode:return dict(result,status='probe rejected',exit=p.returncode)
  data=src.read_bytes()
  if args.install:
   tmp=target.with_suffix('.mmtile.codex-new');tmp.write_bytes(data);os.rename(tmp,target)
  return dict(result,status='installed' if args.install else 'validated only',bytes=len(data),sha256=hashlib.sha256(data).hexdigest())
 except Exception as e:return dict(result,status='exception',error=str(e))
results=[];start=time.time()
print('Candidates',len(rows),flush=True)
with concurrent.futures.ThreadPoolExecutor(max_workers=args.workers) as pool:
 for future in concurrent.futures.as_completed([pool.submit(work,r) for r in rows]):
  r=future.result()
  results.append(r)
  (stage/'results.json').write_text(json.dumps(results,indent=2),encoding='utf-8')
  if len(results)%10==0 or r['status'] in ('probe rejected','exception','generator failed'):print(len(results),collections.Counter(x['status'] for x in results),'seconds',round(time.time()-start),flush=True)
print('DONE',collections.Counter(x['status'] for x in results),flush=True)

raise SystemExit(1 if any(r['status'] in ('probe rejected','exception','generator failed') for r in results) else 0)
