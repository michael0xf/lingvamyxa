"""Focused native regression; reuse a specified successful Exec run's objects.

Usage: python tests/l2/run_sched_snapshot.py PATH/TO/lmx_objects.json
Only the changed exec.c and this test are compiled; no L1/L2 rebuild.
The preceding source revision must fail the parent-retain regression.
"""
import datetime
import hashlib
import json
from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parents[2]
saved = Path(sys.argv[1]).resolve()
records = json.loads(saved.read_text(encoding='utf-8-sig'))
revision = '9e7c92994d514ac626d40c433b5ed49584bd4e87'
out = root / 'build/sched_snapshot' / datetime.datetime.now().strftime('%Y%m%d_%H%M%S_%f')
out.mkdir(parents=True)
inc = out / 'include'
sha = lambda data: hashlib.sha256(data).hexdigest().upper()
exec_record = next(r for r in records if r['source'].replace('\\', '/') == 'l2src/lmx_message_exec.c')
baseline = Path(exec_record['identity']['source']).parent.parent
compiler = Path(exec_record['identity']['compiler'])
assert sha(compiler.read_bytes()) == exec_record['identity']['compilerSHA256']
evidence = {'saved_manifest': str(saved), 'manifest_sha256': sha(saved.read_bytes()),
            'compiler': str(compiler), 'compiler_sha256': sha(compiler.read_bytes()),
            'baseline_revision': revision, 'headers': {}, 'objects': [], 'results': []}
seen = set()
objects = []
for record in records:
    for entry in record['identity']['headers']:
        name, digest = entry.rsplit(':', 1)
        if name in seen:
            continue
        seen.add(name)
        path = Path(name)
        data = path.read_bytes()
        assert sha(data) == digest, name
        rel = path.relative_to(baseline)
        if 'headers' in rel.parts:
            rel = Path(*rel.parts[rel.parts.index('headers') + 1:])
        elif rel.parts[:2] == ('lm1', 'build'):
            rel = Path(*rel.parts[2:])
        dest = inc / rel
        dest.parent.mkdir(parents=True, exist_ok=True)
        dest.write_bytes(data)
        evidence['headers'][name] = digest
    if record['key'] == exec_record['key']:
        continue
    source = baseline / record['object']
    dest = out / source.name
    dest.write_bytes(source.read_bytes())
    objects.append(str(dest))
    evidence['objects'].append({'path': str(source), 'sha256': sha(dest.read_bytes()),
                                'identity': record['identity']})
source = root / 'stg/l1_baseline/l2src/lmx_message_exec.c'
test = Path(__file__).with_name('sched_snapshot.c')
(out / 'current_exec.c').write_bytes(source.read_bytes())
(out / 'sched_snapshot.c').write_bytes(test.read_bytes())
(out / 'baseline_exec.c').write_bytes(subprocess.check_output(
    ['git', 'show', revision + ':stg/l1_baseline/l2src/lmx_message_exec.c'], cwd=root))
flags = ['-std=c99', '-Wall', '-Wextra', '-Wpedantic', '-Werror=incompatible-pointer-types',
         '-Werror=implicit-function-declaration', '-DLMX_MSG_EXEC_TEST', '-I', str(inc)]
def invoke(command, log):
    result = subprocess.run(command, cwd=out, capture_output=True, timeout=30)
    (out / log).write_bytes(result.stdout + result.stderr)
    if result.returncode:
        print((result.stdout + result.stderr).decode(errors='replace')[-4000:])
    return result
ok = True
for variant, expected in [('baseline', 1), ('current', 0)]:
    obj = out / (variant + '.o')
    src = out / (variant + '_exec.c')
    r = invoke([str(compiler), *flags, '-Dmalloc=lmx_sched_snapshot_test_malloc', '-c', str(src), '-o', str(obj)], variant + '_compile.log')
    assert r.returncode == 0
    exe = out / (variant + '.exe')
    r = invoke([str(compiler), *flags, str(out / 'sched_snapshot.c'), str(obj), *objects, '-o', str(exe)], variant + '_link.log')
    assert r.returncode == 0
    r = invoke([str(exe)], variant + '_test.log')
    result = {'variant': variant, 'compile_exit': 0, 'link_exit': 0, 'test_exit': r.returncode,
              'expected_exit': expected, 'source_sha256': sha(src.read_bytes()),
              'test_sha256': sha(test.read_bytes()), 'executable_sha256': sha(exe.read_bytes()),
              'stdout': r.stdout.decode(errors='replace')}
    evidence['results'].append(result)
    ok = ok and r.returncode == expected
    print(variant, 'exit', r.returncode, result['stdout'].strip())
evidence['passed'] = ok
(out / 'evidence.json').write_text(json.dumps(evidence, indent=2), encoding='utf-8')
print('evidence', out / 'evidence.json')
sys.exit(0 if ok else 1)
