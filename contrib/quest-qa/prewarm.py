"""Provision reusable client-defined bodies. This is NOT quest gameplay testing."""
import json
from datetime import datetime,timezone
from pool import Pool,ROOT
from catalogue_matrix import playable_pairs
from evidence import write_json


def main():
    pool=Pool();output=ROOT/'out/quest-qa/pool-prewarm.json'
    report=dict(started_utc=datetime.now(timezone.utc).isoformat(),scope='Reusable identity provisioning only; no quest gameplay passes',account_character_cap=11,cases=[])
    for race,klass in playable_pairs(ROOT/'exports/dbc/CharBaseInfo.dbc'):
        try:
            char=pool.ensure(race,klass)
            report['cases'].append(dict(race=race,class_id=klass,status='PROVISIONED',character_guid=char['guid'],account_id=char['account_id'],prepared_faction_state=char['prepared_faction_state']))
        except Exception as e:report['cases'].append(dict(race=race,class_id=klass,status='BLOCKED',error=str(e)))
        write_json(output,report)
    report.update(finished_utc=datetime.now(timezone.utc).isoformat(),account_counts=pool.db.q('Character','SELECT account,COUNT(*) FROM characters GROUP BY account'),gameplay_passes=0)
    assert all(int(n)<=11 for _,n in report['account_counts'])
    write_json(output,report);print(json.dumps(dict(report=str(output),provisioned=sum(c['status']=='PROVISIONED' for c in report['cases']),blocked=sum(c['status']=='BLOCKED' for c in report['cases']))))


if __name__=='__main__':main()
