"""Atomic evidence writes with bounded Windows file-sharing retries."""
import json
import os
import time
import uuid
from pathlib import Path


def write_json(path, value):
    path=Path(path);temporary=path.with_name(path.name+'.'+uuid.uuid4().hex+'.tmp')
    temporary.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8')
    for attempt in range(20):
        try:
            os.replace(temporary,path)
            return
        except PermissionError:
            if attempt==19:raise
            time.sleep(.05)
