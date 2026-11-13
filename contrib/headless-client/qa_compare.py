"""Compare equivalent QA runs. A prior passing run is a regression baseline, not retail truth."""
import argparse,json,sys
from pathlib import Path

def compare(before,after):
 if before['suite']!=after['suite']:raise ValueError('Suite names differ; compare equivalent scenario sets')
 if before.get('suite_sha256') and after.get('suite_sha256') and before['suite_sha256']!=after['suite_sha256']:raise ValueError('Scenario expectations changed; review before comparing')
 changes=[]
 for category in ('gameplay','data_audits'):
  old={r['id']:r for r in before[category]};new={r['id']:r for r in after[category]}
  for ident,r in old.items():
   n=new.get(ident)
   if r['status']=='passed' and (n is None or n['status']!='passed'):
    changes.append(dict(category=category,id=ident,regression='lost passing result',before=r['status'],after=n['status'] if n else 'missing'))
   elif category=='data_audits' and n and n.get('violation_count',0)>r.get('violation_count',0):
    changes.append(dict(category=category,id=ident,regression='more structural violations',before=r.get('violation_count'),after=n['violation_count']))
 return dict(regressions=changes,regression_free=not changes,baseline_is_retail_truth=False)

def main():
 p=argparse.ArgumentParser();p.add_argument('baseline',type=Path);p.add_argument('actual',type=Path);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
 result=compare(json.loads(a.baseline.read_text()),json.loads(a.actual.read_text()));a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result));return int(not result['regression_free'])
if __name__=='__main__':sys.exit(main())
