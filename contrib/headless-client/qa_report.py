"""Coverage accounting deliberately keeps data checks separate from gameplay."""
from collections import Counter
STATES={'passed','failed','blocked','untested'}
def summarize(results):
    if not results: return dict(counts={s:0 for s in sorted(STATES)},complete=False,ready=False)
    for result in results:
        if result.get('status') not in STATES: raise ValueError('Invalid result state')
    counts=Counter(r['status'] for r in results)
    return dict(counts={s:counts[s] for s in sorted(STATES)},complete=not(counts['blocked'] or counts['untested']),ready=all(r['status']=='passed' for r in results))
