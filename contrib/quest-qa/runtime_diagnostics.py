"""Collect current server diagnostics without changing logger thresholds or deleting logs."""
import hashlib
import re
from collections import Counter
from pathlib import Path

LINE=re.compile(r'^(\S+)\s+(WARN|ERROR|FATAL)\s+(.*)$')


def collect(runtime):
    groups={};events=Counter();partial=[];files_read=[]
    for name in ('Server.log','Errors.txt','DBErrors.log','Auth.log','AuthErrors.txt'):
        path=Path(runtime)/'logs'/name
        if not path.exists():continue
        files_read.append(name)
        with path.open('rb') as stream:
            size=path.stat().st_size
            if size>32*1024*1024:
                stream.seek(size-32*1024*1024);stream.readline();partial.append(name)
            lines=stream.read().decode('utf-8',errors='replace').splitlines()
        file_events=Counter()
        for line in lines:
            match=LINE.match(line)
            if not match:continue
            stamp,severity,message=match.groups()
            key=(severity,message)
            row=groups.setdefault(key,dict(id=hashlib.sha256((severity+' '+message).encode()).hexdigest()[:16],
                severity=severity,message=message,occurrences=0,first_seen=stamp,last_seen=stamp,files=[]))
            if name not in row['files']:row['files'].append(name)
            event=(stamp,severity,message)
            file_events[event]+=1
            if file_events[event]>events[event]:
                events[event]=file_events[event];row['occurrences']+=1
            row['first_seen']=min(row['first_seen'],stamp);row['last_seen']=max(row['last_seen'],stamp)
    return dict(unique_signatures=len(groups),partial_files=partial,files_read=files_read,
        scope='Current log files only; rotated history is preserved and not included.',
        findings=sorted(groups.values(),key=lambda r:(r['severity'],r['message'])))
