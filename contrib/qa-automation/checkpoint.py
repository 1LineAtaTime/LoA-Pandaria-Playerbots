"""Private, content-addressed runtime checkpoints and scoped SQL backups. Never restores automatically."""
import hashlib
import json
import os
import re
import shutil
import subprocess
from pathlib import Path

IDENT = re.compile(r'^[A-Za-z_][A-Za-z0-9_]*$')
DATABASES = {'World', 'Character', 'Login'}


def sha(path):
    h = hashlib.sha256()
    with Path(path).open('rb') as f:
        for block in iter(lambda: f.read(1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def private_path(root, path):
    path = Path(path).resolve()
    if not path.is_relative_to((Path(root) / 'out').resolve()):
        raise ValueError('Private checkpoints must stay inside repository out/')
    return path


def validate_plan(plan):
    if not isinstance(plan, list) or not plan:
        raise ValueError('Backup plan must be a nonempty array')
    for item in plan:
        if not isinstance(item, dict) or item.get('database') not in DATABASES:
            raise ValueError('Use a configured World, Character or Login database')
        if item.get('full') is True:
            if set(item) != {'database', 'full'}:
                raise ValueError('Full backups cannot contain row selectors')
            continue
        if set(item) != {'database', 'table', 'key', 'ids'}:
            raise ValueError('Row backups require database/table/key/ids')
        if any(not isinstance(item[k], str) or not IDENT.fullmatch(item[k]) for k in ('table', 'key')):
            raise ValueError('Invalid SQL identifier')
        ids = item['ids']
        if not isinstance(ids, list) or not ids or len(ids) > 10000 or any(type(i) is not int or not 0 <= i <= 0xffffffffffffffff for i in ids):
            raise ValueError('Backup IDs must be a bounded list of unsigned integers')
        if len(set(ids)) != len(ids):
            raise ValueError('Duplicate backup IDs')
    return plan


def runtime_checkpoint(root, output):
    root, output = Path(root), private_path(root, output)
    output.mkdir(parents=True, exist_ok=False)
    cache = private_path(root, root / 'out/qa-automation/artifacts')
    cache.mkdir(parents=True, exist_ok=True)
    selected = [root/'exports'/n for n in ('worldserver.exe', 'worldserver.pdb', 'authserver.exe', 'authserver.pdb',
                'worldserver.conf', 'authserver.conf', 'playerbots.conf', 'ACTIVE-RUNTIME-MANIFEST.json')]
    selected += sorted((root/'exports').glob('*.dll'))
    artifacts = []
    for p in selected:
        if not p.is_file():
            continue
        digest = sha(p); destination = private_path(root, cache/digest/p.name)
        destination.parent.mkdir(parents=True, exist_ok=True)
        if not destination.exists():
            shutil.copyfile(p, destination)
        if sha(destination) != digest:
            raise RuntimeError('Checkpoint artifact changed or cache is corrupt: ' + p.name)
        artifacts.append(dict(name=p.name, sha256=digest, bytes=p.stat().st_size,
                              private_copy=str(destination.relative_to(root))))
    revision = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=root, text=True).strip()
    patch = subprocess.check_output(['git', 'diff', '--binary', 'HEAD'], cwd=root)
    (output/'working-tree.patch').write_bytes(patch)
    # Newly authored untracked tools are not in git diff. Preserve text sources only;
    # the user's untracked reference executables never enter this checkpoint.
    untracked = subprocess.check_output(['git', 'ls-files', '-z', '--others', '--exclude-standard', '--',
        'contrib', 'src', 'modules', 'dep', 'sql', 'doc'], cwd=root).decode('utf-8').strip('\0').split('\0')
    untracked = [name for name in untracked if name]
    copied = []
    for name in untracked:
        source = root/name
        if source.suffix.lower() not in ('.py', '.ps1', '.json', '.md', '.cpp', '.h', '.sql', '.txt'):
            continue
        destination = output/'untracked-source'/name
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(source, destination)
        copied.append(name)
    report = dict(revision=revision, artifacts=artifacts, working_patch_sha256=sha(output/'working-tree.patch'),
                  untracked_source_files=copied, database_backups=[],
                  consistency='Runtime files individually hash-verified; no process-memory or cross-schema atomic snapshot.')
    (output/'checkpoint.json').write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
    return report


def database_backups(db, plan, output):
    plan = validate_plan(plan); output = Path(output)
    dump = Path(db.client).with_name('mariadb-dump.exe' if os.name == 'nt' else 'mariadb-dump')
    if not dump.exists():
        raise RuntimeError('Matching mariadb-dump client is required')
    results = []
    for index, item in enumerate(plan):
        kind = item['database']; host, port, user, password, schema = db.connections[kind]
        args = [str(dump), '--no-defaults', '--protocol=TCP', '--host='+host, '--port='+port,
                '--user='+user, '--single-transaction', '--skip-lock-tables', '--hex-blob', '--default-character-set=utf8mb4']
        name = f'{index:02d}-{kind}'
        if item.get('full'):
            args += ['--routines', '--events', schema]
        else:
            table, key = item['table'], item['key']
            columns = db.q(kind, "SELECT COLUMN_NAME,DATA_TYPE FROM information_schema.COLUMNS WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME='"+table+"' AND COLUMN_NAME='"+key+"'")
            if not columns or columns[0][1].lower() not in ('tinyint','smallint','mediumint','int','bigint'):
                raise ValueError('Backup selector must reference an existing integer column')
            where = '`'+key+'` IN ('+','.join(map(str, sorted(item['ids'])))+')'
            args += ['--no-create-info', '--skip-triggers', '--skip-add-locks', '--skip-disable-keys', '--skip-extended-insert', '--complete-insert', '--where='+where, schema, table]
            name += '-'+table
        target = output/(name+'.sql'); partial = target.with_suffix('.sql.partial')
        with partial.open('wb') as stream:
            process = subprocess.run(args, stdout=stream, stderr=subprocess.PIPE,
                                     env=dict(os.environ, MYSQL_PWD=password), timeout=1800)
        if process.returncode:
            raise RuntimeError('Backup failed: '+process.stderr.decode(errors='replace').replace(password,'[redacted]')[-800:])
        partial.replace(target)
        results.append(dict(plan=item, file=target.name, bytes=target.stat().st_size, sha256=sha(target),
                            consistency='Separate single-transaction dump; different entries are not one atomic snapshot.'))
        if not item.get('full'):
            with target.open(encoding='utf-8',errors='strict') as source:
                results[-1]['rows_dumped']=sum(line.startswith('INSERT INTO ') for line in source)
            restore = output/(name+'-restore.sql')
            with restore.open('wb') as destination,target.open('rb') as source:
                destination.write(('START TRANSACTION;\nDELETE FROM `'+item['table']+'` WHERE '+where+';\n').encode())
                shutil.copyfileobj(source,destination)
                destination.write(b'\nCOMMIT;\n')
            results[-1]['restore_file'] = restore.name
            results[-1]['restore_sha256'] = sha(restore)
    return results
